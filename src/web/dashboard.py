# -*- coding: utf-8 -*-
# 
# # MIT License
# 
# Copyright (c) 2025 Mike Simms
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import argparse
import bcrypt
import database
import flask
import functools
import json
import logging
import os
import secrets
import signal
import sys
import time
import uuid
import traceback
import InputChecker

from mako.template import Template
from urllib.parse import unquote_plus

# Global variables
g_flask_app = flask.Flask(__name__)
g_root_dir = ""
g_root_url = ""
g_db_uri = ""
g_tempmod_dir = "tempmod"

# Files and directories
ERROR_LOG = 'error.log'
CSS_DIR = 'css'
JS_DIR = 'js'
IMAGES_DIR = 'images'
HTML_DIR = 'html'

START_TS = 'start_ts'
MIN_PASSWORD_LEN  = 8
SESSION_COOKIE = 'session_cookie'

# Constants used with the API
PARAM_USERNAME = "username" # Login name for a user
PARAM_REALNAME = "realname" # User's real name
PARAM_PASSWORD = "password" # User's password
PARAM_PASSWORD1 = "password1" # User's password when creating an account
PARAM_PASSWORD2 = "password2" # User's confirmation password when creating an account
PARAM_SESSION_COOKIE = "session_cookie"
PARAM_SESSION_EXPIRY = "session_expiry"
PARAM_HASH_KEY = "hash" # Password hash
PARAM_API_KEY = "api_key"
PARAM_COLLECTION = "collection"
PARAM_TIMESTAMP = "ts"

def login_required(function_to_protect):
    @functools.wraps(function_to_protect)
    def wrapper(*args, **kwargs):
        global g_flask_app

        # If the app has not been created then redirect to the "waiting for backend" page.
        if g_flask_app == None:
            flask.session['redirect'] = '/' + function_to_protect.__name__
            return flask.redirect(flask.url_for('waiting'))

        # Grab the session cookie.
        session_cookie = flask.request.cookies.get(SESSION_COOKIE)
        if session_cookie is not None:
            session_cookie = uuid.UUID(session_cookie)

            # Get the user from the session cookie.
            # This function will take care of checking for session expiry.
            db = connect_to_db()
            user, expiry = db.retrieve_session_data(str(session_cookie))

            # We found a user with a valid login session, continue.
            if user:
                return function_to_protect(*args, **kwargs)
        
        # No valid login session, redirect to the login page.
        flask.session['redirect'] = '/' + function_to_protect.__name__
        return flask.redirect(flask.url_for('login'))
    return wrapper
    
class ApiException(Exception):
    """Exception thrown by a REST API."""

    def __init__(self, *args):
        Exception.__init__(self, args)

    def __init__(self, code, message):
        self.code = code
        self.message = message
        Exception.__init__(self, code, message)

class ApiMalformedRequestException(ApiException):
    """Exception thrown by a REST API when an API request is missing required parameters."""

    def __init__(self, message):
        ApiException.__init__(self, 400, message)

class ApiAuthenticationException(ApiException):
    """Exception thrown by a REST API when user authentication fails."""

    def __init__(self, message):
        ApiException.__init__(self, 401, message)

class ApiNotLoggedInException(ApiException):
    """Exception thrown by a REST API when the user is not logged in."""

    def __init__(self):
        ApiException.__init__(self, 403, "Not logged in")
        
def signal_handler(signal, frame):
    print("Exiting...")
    sys.exit(0)

def log_info(log_str):
    """Writes an info message to the log file."""
    logger = logging.getLogger()
    logger.info(log_str)

def log_error(log_str):
    """Writes an error message to the log file."""
    logger = logging.getLogger()
    logger.error(log_str)

def connect_to_db():
    """Utility function for getting a database connection."""
    global g_db_uri
    db = database.AppMongoDatabase()
    db.connect(g_db_uri)
    return db

def authenticate_user(email, password):
    """Validates a user against the credentials in the database."""
    if len(email) == 0:
        raise Exception("An email address not provided.")
    if len(password) < MIN_PASSWORD_LEN:
        raise Exception("The password is too short.")

    # Connect to the database.
    db = connect_to_db()

    # Get the exsting password hash for the user.
    _, db_hash1, _ = db.retrieve_user(email)
    if db_hash1 is None:
        raise Exception("The user (" + email + ") could not be found.")

    # Validate the provided password against the hash from the database.
    if isinstance(password, str):
        password = password.encode()
    if isinstance(db_hash1, str):
        db_hash1 = db_hash1.encode()
    return bcrypt.checkpw(password, db_hash1)

def create_user(email, realname, password1, password2):
    """Adds a user to the database."""
    if len(email) == 0:
        raise Exception("An email address not provided.")
    if len(realname) == 0:
        raise Exception("Name not provided.")
    if len(password1) < MIN_PASSWORD_LEN:
        raise Exception("The password is too short.")
    if password1 != password2:
        raise Exception("The passwords do not match.")

    # Connect to the database.
    db = connect_to_db()

    # Make sure this user doesn't already exist.
    _, db_hash1, _ = db.retrieve_user(email)
    if db_hash1 is not None:
        raise Exception("The user already exists.")

    # Generate the salted hash of the password.
    salt = bcrypt.gensalt()
    computed_hash = bcrypt.hashpw(password1.encode('utf-8'), salt)
    if not db.create_user(email, realname, computed_hash):
        raise Exception("An internal error was encountered when creating the user.")

    return True

def create_new_session(email):
    """Starts a new session. Returns the session cookie and it's expiry date."""
    # Session cookie and expiry.
    session_cookie = str(uuid.uuid4())
    expiry = int(time.time() + 90.0 * 86400.0)

    # Connect to the database.
    db = connect_to_db()

    # Save it to the database.
    if db.create_session_cookie(email, session_cookie, expiry):
        return session_cookie, expiry
    return None, None

def delete_session(session_cookie):
    db = connect_to_db()
    return db.delete_session_cookie(session_cookie)

def validate_session(session_cookie):
    """Returns TRUE if the session cookie is valid."""
    db = connect_to_db()
    user, expiry = db.retrieve_session_data(str(session_cookie))
    if expiry is not None:

        # Is the cookie still valid.
        now = time.time()
        if now < expiry:
            return True, user

        # Cookie is expired, so delete it.
        db.delete_session_cookie(session_cookie)
    return False, user

def common_api_key_check(values):
    """Factoring of common API key checking code."""
    # Required parameters.
    if PARAM_API_KEY not in values:
        raise ApiAuthenticationException("API key not specified.")

    # Validate the key.
    key = values[PARAM_API_KEY]

    # Check the database.
    db = connect_to_db()
    user = db.retrieve_api_key(key)
    return True, user

def common_session_check(values):
    """Factoring of common session checking code."""
    # Required parameters.
    if PARAM_SESSION_COOKIE not in values:
        raise ApiAuthenticationException("Session cookie not specified.")

    # Validate the required parameters.
    session_cookie = values[PARAM_SESSION_COOKIE]
    if not InputChecker.is_uuid(session_cookie):
        raise ApiAuthenticationException("Session cookie is invalid.")

    # Is this is a valid session?
    valid_session, user = validate_session(session_cookie)
    if not valid_session:
        raise ApiAuthenticationException("Session is invalid.")

    return valid_session, user

def common_auth_check(values):
    """Checks for either a valid login session cookie, or a valid api key."""
    if PARAM_SESSION_COOKIE not in values and PARAM_API_KEY not in values:
        raise ApiAuthenticationException("Session cookie or API key not specified.")
    if PARAM_API_KEY in values:
        return common_api_key_check(values)
    if PARAM_SESSION_COOKIE in values:
        return common_session_check(values)
    raise Exception("Internal error.")

@g_flask_app.route('/css/<file_name>')
def css(file_name):
    """Returns the CSS page."""
    try:
        return flask.send_from_directory(CSS_DIR, file_name)
    except:
        log_error(traceback.format_exc())
        log_error(sys.exc_info()[0])
        log_error('Unhandled exception in ' + css.__name__)
    return ""

@g_flask_app.route('/js/<file_name>')
def js(file_name):
    """Returns the JS page."""
    try:
        return flask.send_from_directory(JS_DIR, file_name)
    except:
        log_error(traceback.format_exc())
        log_error(sys.exc_info()[0])
        log_error('Unhandled exception in ' + js.__name__)
    return ""

@g_flask_app.route('/images/<file_name>')
def images(file_name):
    """Returns the contents from the images directory."""
    try:
        return flask.send_from_directory(IMAGES_DIR, file_name)
    except:
        log_error(traceback.format_exc())
        log_error(sys.exc_info()[0])
        log_error('Unhandled exception in ' + js.__name__)
    return ""

@g_flask_app.route('/login')
def login():
    """Renders the login page."""
    try:
        html_file = os.path.join(g_root_dir, HTML_DIR, 'login.html')
        my_template = Template(filename=html_file, module_directory=g_tempmod_dir)
        return my_template.render(root_url=g_root_url)
    except:
        log_error("Unhandled Exception")
    return ""

@g_flask_app.route('/admin')
@login_required
def admin():
    """Renders the admin page."""
    try:
        html_file = os.path.join(g_root_dir, HTML_DIR, 'admin.html')
        my_template = Template(filename=html_file, module_directory=g_tempmod_dir)
        return my_template.render(root_url=g_root_url)
    except:
        log_error("Unhandled Exception")
    return ""

@g_flask_app.route('/')
def index():
    """Renders the index page."""
    try:
        html_file = os.path.join(g_root_dir, HTML_DIR, 'index.html')
        my_template = Template(filename=html_file, module_directory=g_tempmod_dir)
        return my_template.render(root_url=g_root_url)
    except:
        log_error("Unhandled Exception")
    return ""

def handle_api_indoor_air_request(values):
    """Called when an API request for the indoor air status data is received."""
    start_ts = 0
    if START_TS in values:
        start_ts = int(values[START_TS])
    db = connect_to_db()
    readings = list(db.retrieve_air_quality(start_ts))
    result = json.dumps(readings)
    return True, result

def handle_api_patio_request(values):
    """Called when an API request for the patio status is received."""
    start_ts = 0
    if START_TS in values:
        start_ts = int(values[START_TS])
    db = connect_to_db()
    readings = list(db.retrieve_patio_status(start_ts))
    result = json.dumps(readings)
    return True, result

def handle_api_refrigerator_request(values):
    """Called when an API request for the keg status is received."""
    start_ts = 0
    if START_TS in values:
        start_ts = int(values[START_TS])
    db = connect_to_db()
    readings = list(db.retrieve_refrigerator_status(start_ts))
    result = json.dumps(readings)
    return True, result

def handle_api_website_status(values):
    """Called when an API request for the website status data is received."""
    start_ts = 0
    if START_TS in values:
        start_ts = int(values[START_TS])
    db = connect_to_db()
    readings = list(db.retrieve_website_status(start_ts))
    result = json.dumps(readings)
    return True, result

def handle_api_login(values):
    """Called when an API request to login a user is received."""
    # Required parameters.
    if PARAM_USERNAME not in values:
        raise ApiAuthenticationException("Username not specified.")
    if PARAM_PASSWORD not in values:
        raise ApiAuthenticationException("Password not specified.")

    # Decode and validate the required parameters.
    email = unquote_plus(values[PARAM_USERNAME])
    if not InputChecker.is_email_address(email):
        raise ApiAuthenticationException("Invalid email address.")
    password = unquote_plus(values[PARAM_PASSWORD])

    # Validate the credentials.
    try:
        if not authenticate_user(email, password):
            raise ApiAuthenticationException("Authentication failed.")
    except Exception as e:
        raise ApiAuthenticationException(str(e))

    # Create session information for this new login.
    cookie, expiry = create_new_session(email)
    if not cookie:
        raise ApiAuthenticationException("Session cookie not generated.")
    if not expiry:
        raise ApiAuthenticationException("Session expiry not generated.")

    # Encode the session info.
    session_data = {}
    session_data[PARAM_SESSION_COOKIE] = cookie
    session_data[PARAM_SESSION_EXPIRY] = expiry
    json_result = json.dumps(session_data, ensure_ascii=False)

    return True, json_result

def handle_api_create_login(values):
    """Called when an API request to create a user is received."""
    # Required parameters.
    if PARAM_USERNAME not in values:
        raise ApiAuthenticationException("Username not specified.")
    if PARAM_REALNAME not in values:
        raise ApiAuthenticationException("Real name not specified.")
    if PARAM_PASSWORD1 not in values:
        raise ApiAuthenticationException("Password not specified.")
    if PARAM_PASSWORD2 not in values:
        raise ApiAuthenticationException("Password confirmation not specified.")

    # Decode and validate the required parameters.
    email = unquote_plus(values[PARAM_USERNAME])
    if not InputChecker.is_email_address(email):
        raise ApiMalformedRequestException("Invalid email address.")
    realname = unquote_plus(values[PARAM_REALNAME])
    if not InputChecker.is_valid_decoded_str(realname):
        raise ApiMalformedRequestException("Invalid name.")
    password1 = unquote_plus(values[PARAM_PASSWORD1])
    password2 = unquote_plus(values[PARAM_PASSWORD2])

    # Add the user to the database, should fail if the user already exists.
    try:
        if not create_user(email, realname, password1, password2):
            raise Exception("User creation failed.")
    except:
        raise Exception("User creation failed.")

    # The new user should start in a logged-in state, so generate session info.
    cookie, expiry = create_new_session(email)
    if not cookie:
        raise ApiAuthenticationException("Session cookie not generated.")
    if not expiry:
        raise ApiAuthenticationException("Session expiry not generated.")

    # Encode the session info.
    session_data = {}
    session_data[PARAM_SESSION_COOKIE] = cookie
    session_data[PARAM_SESSION_EXPIRY] = expiry
    json_result = json.dumps(session_data, ensure_ascii=False)

    return True, json_result

def handle_api_login_status(values):
    """Called when an API request to login a user is received."""
    # Validate the session cookie.
    valid_session, _ = common_session_check(values)
    return valid_session, ""

def handle_api_logout(values):
    """Called when an API request to logout a user is received."""
    # Required parameters.
    if PARAM_SESSION_COOKIE not in values:
        raise ApiAuthenticationException("Session cookie not specified.")

    # Validate the required parameters.
    session_cookie = values[PARAM_SESSION_COOKIE]
    if not InputChecker.is_uuid(session_cookie):
        raise ApiAuthenticationException("Session cookie is invalid.")

    delete_session(session_cookie)
    return True, ""

def handle_api_update_status(values):
    """Called when an API request to update the status of a sensor is received."""
    # Required parameters.
    if PARAM_COLLECTION not in values:
        raise ApiAuthenticationException("Collection not specified.")

    # Validate the session cookie.
    _, _ = common_auth_check(values)
    if PARAM_SESSION_COOKIE in values:
        del values[PARAM_SESSION_COOKIE] # Remove this as there's no reason to store it.
    if PARAM_API_KEY in values:
        del values[PARAM_API_KEY] # Remove this as there's no reason to store it.

    # What are we updating?
    collection = values[PARAM_COLLECTION]
    del values[PARAM_COLLECTION] # Remove this as there's no reason to store it.

    # Was a timestamp provided? If not, add one.
    if not PARAM_TIMESTAMP in values:
        values[PARAM_TIMESTAMP] = time.time()

    # Connect to the database.
    db = connect_to_db()

    # Add to the database.
    db.create_status(collection, values)

    return True, ""

def handle_api_create_api_key(values):
    """Called when an API request to create an API key is received."""
    # Validate the session cookie.
    _, user = common_session_check(values)

    # Generate an API key.
    api_key = secrets.token_bytes(256)
    api_key = api_key.hex()

    # Expiry. Three years.
    expiry = int(time.time() + 90.0 * 86400.0 * 3)

    # Connect to the database.
    db = connect_to_db()

    # Store it.
    db.create_api_key(api_key, expiry, user)
    return True, ""

def handle_api_list_api_keys(values):
    """Called when an API request to list API keys is received."""
    # Validate the session cookie.
    _, user = common_session_check(values)

    # Connect to the database.
    db = connect_to_db()

    # List the api keys.
    keys = db.retrieve_api_keys(user)

    return True, keys

def handle_api_1_0_get_request(request, values):
    """Called to parse a version 1.0 API GET request."""
    if request == 'indoor_air':
        return handle_api_indoor_air_request(values)
    if request == 'patio':
        return handle_api_patio_request(values)
    if request == 'refrigerator':
        return handle_api_refrigerator_request(values)
    if request == 'website_status':
        return handle_api_website_status(values)
    if request == 'list_api_keys':
        return handle_api_list_api_keys(values)
    return False, ""

def handle_api_1_0_post_request(request, values):
    """Called to parse a version 1.0 API POST request."""
    if request == 'login':
        return handle_api_login(values)
    if request == 'create_login':
        return handle_api_create_login(values)
    if request == 'logout':
        return handle_api_logout(values)
    if request == 'update_status':
        return handle_api_update_status(values)
    if request == 'create_api_key':
        return handle_api_create_api_key(values)
    return False, ""

def handle_api_1_0_delete_request(request, values):
    """Called to parse a version 1.0 API DELETE request."""
    return False, ""

def handle_api_request(verb, request, values):
    """Handles API requests."""
    request = request.lower()
    if verb == 'GET':
        return handle_api_1_0_get_request(request, values)
    if verb == 'POST':
        return handle_api_1_0_post_request(request, values)
    if verb == 'DELETE':
        return handle_api_1_0_delete_request(request, values)
    return False, ""

@g_flask_app.route('/api/<version>/<method>', methods = ['GET','POST','DELETE'])
def api(version, method):
    """Endpoint for API calls."""
    response = ""
    code = 500
    try:
        # The the API params.
        if flask.request.method == 'GET':
            verb = "GET"
            params = flask.request.args
        elif flask.request.method == 'DELETE':
            verb = "DELETE"
            params = flask.request.args
        elif flask.request.data:
            verb = "POST"
            params = json.loads(flask.request.data)
        else:
            verb = "GET"
            params = ""

        # Process the API request.
        if version == '1.0':
            handled, response = handle_api_request(verb, method, params)
            if handled:
                code = 200
            else:
                code = 400
        else:
            code = 400
    except:
        log_error(traceback.format_exc())
        log_error(sys.exc_info()[0])
        response = 'Unhandled exception in ' + api.__name__
        log_error(response)
    return response, code

def main():
    global g_flask_app
    global g_db_uri

    # Configure the error logger.
    logging.basicConfig(filename=ERROR_LOG, filemode='w', level=logging.DEBUG, format='%(asctime)s %(message)s', datefmt='%m/%d/%Y %I:%M:%S %p')

    # Make sure we have a compatible version of python.
    if sys.version_info[0] < 3:
        print("This application requires python 3.")
        sys.exit(1)

    # Parse the command line options.
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", type=str, action="store", default="localhost", help="The host interface on which to bind.", required=False)
    parser.add_argument("--port", type=int, action="store", default=5050, help="The host port on which to bind.", required=False)
    parser.add_argument("--database", type=str, action="store", default="mongodb://localhost:27017", help="The URI for connecting to the database.", required=False)

    try:
        args = parser.parse_args()
    except IOError as e:
        parser.error(e)
        sys.exit(1)

    # Register the signal handler.
    signal.signal(signal.SIGINT, signal_handler)

    # Cleanup the arguments.
    if len(args.host) == 0:
        args.host = "localhost"

    # Make a note of the database URI.
    g_db_uri = args.database

    # Random secret key.
    g_flask_app.secret_key = os.urandom(12).hex()
    print(g_flask_app.secret_key)

    # Create the app object. It contains all the functionality.
    print(f"The app is running on http://{args.host}:{args.port}")
    g_flask_app.run(host=args.host, port=args.port)

if __name__=="__main__":
    main()
