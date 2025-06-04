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
import database
import flask
import json
import logging
import os
import signal
import sys
import traceback
import InputChecker

from mako.template import Template

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
HTML_DIR = 'html'

START_TS = 'start_ts'

# Constants used with the API
PARAM_USERNAME = "username" # Login name for a user
PARAM_REALNAME = "realname" # User's real name
PARAM_PASSWORD = "password" # User's password
PARAM_PASSWORD1 = "password1" # User's password when creating an account
PARAM_PASSWORD2 = "password2" # User's confirmation password when creating an account
PARAM_SESSION_TOKEN = "session_token"
PARAM_SESSION_EXPIRY = "session_expiry"
PARAM_HASH_KEY = "hash" # Password hash

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
    global g_db_uri
    start_ts = 0
    if START_TS in values:
        start_ts = int(values[START_TS])
    db = database.AppMongoDatabase()
    db.connect(g_db_uri)
    readings = list(db.retrieve_air_quality(start_ts))
    result = json.dumps(readings)
    return True, result

def handle_api_patio_request(values):
    global g_db_uri
    start_ts = 0
    if START_TS in values:
        start_ts = int(values[START_TS])
    db = database.AppMongoDatabase()
    db.connect(g_db_uri)
    readings = list(db.retrieve_patio_status(start_ts))
    result = json.dumps(readings)
    return True, result

def handle_api_website_status(values):
    global g_db_uri
    start_ts = 0
    if START_TS in values:
        start_ts = int(values[START_TS])
    db = database.AppMongoDatabase()
    db.connect(g_db_uri)
    readings = list(db.retrieve_website_status(start_ts))
    result = json.dumps(readings)
    return True, result

def handle_api_login(self, values):
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
        if not self.user_mgr.authenticate_user(email, password):
            raise ApiAuthenticationException("Authentication failed.")
    except Exception as e:
        raise ApiAuthenticationException(str(e))

    # Create session information for this new login.
    cookie, expiry = self.user_mgr.create_new_session(email)
    if not cookie:
        raise ApiAuthenticationException("Session token not generated.")
    if not expiry:
        raise ApiAuthenticationException("Session expiry not generated.")

    # Encode the session info.
    session_data = {}
    session_data[PARAM_SESSION_TOKEN] = cookie
    session_data[PARAM_SESSION_EXPIRY] = expiry
    json_result = json.dumps(session_data, ensure_ascii=False)

    return True, json_result

def handle_api_create_login(self, values):
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
        if not self.user_mgr.create_user(email, realname, password1, password2):
            raise Exception("User creation failed.")
    except:
        raise Exception("User creation failed.")

    # The new user should start in a logged-in state, so generate session info.
    cookie, expiry = self.user_mgr.create_new_session(email)
    if not cookie:
        raise ApiAuthenticationException("Session token not generated.")
    if not expiry:
        raise ApiAuthenticationException("Session expiry not generated.")

    # Encode the session info.
    session_data = {}
    session_data[PARAM_SESSION_TOKEN] = cookie
    session_data[PARAM_SESSION_EXPIRY] = expiry
    json_result = json.dumps(session_data, ensure_ascii=False)

    return True, json_result

def handle_api_login_status(self, values):
    # Required parameters.
    if PARAM_SESSION_TOKEN not in values:
        raise ApiAuthenticationException("Session token not specified.")
    
    # Validate the required parameters.
    session_token = values[PARAM_SESSION_TOKEN]
    if not InputChecker.is_uuid(session_token):
        raise ApiAuthenticationException("Session token is invalid.")

    valid_session = validate_session(session_token)
    return valid_session, ""

def handle_api_logout(self, values):
    # Required parameters.
    if PARAM_SESSION_TOKEN not in values:
        raise ApiAuthenticationException("Session token not specified.")

    # Validate the required parameters.
    session_token = values[PARAM_SESSION_TOKEN]
    if not InputChecker.is_uuid(session_token):
        raise ApiAuthenticationException("Session token is invalid.")

    delete_session(session_token)
    return True, ""

def handle_api_1_0_get_request(request, values):
    """Called to parse a version 1.0 API GET request."""
    if request == 'indoor_air':
        return handle_api_indoor_air_request(values)
    if request == 'patio':
        return handle_api_patio_request(values)
    if request == 'website_status':
        return handle_api_website_status(values)
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
        return handle_api_status(values)
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

    # Create the app object. It contains all the functionality.
    print(f"The app is running on http://{args.host}:{args.port}")
    g_flask_app.run(host=args.host, port=args.port)

if __name__=="__main__":
	main()
