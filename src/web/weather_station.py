import argparse
import database
import flask
import json
import logging
import os
import signal
import sys
import traceback

from mako.template import Template

g_flask_app = flask.Flask(__name__)
g_root_dir = ""
g_root_url = ""
g_db_uri = ""
g_tempmod_dir = "tempmod"

ERROR_LOG = 'error.log'
HTML_DIR = 'html'

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

def handle_indoor_air_request(values):
    global g_db_uri
    db = database.AppMongoDatabase()
    db.connect(g_db_uri)
    return False, ""

def handle_api_1_0_get_request(request, values):
    """Called to parse a version 1.0 API GET request."""
    if request == 'indoor_air':
        return handle_indoor_air_request(values)
    return False, ""

def handle_api_1_0_post_request(request, values):
    """Called to parse a version 1.0 API POST request."""
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
