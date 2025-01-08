import argparse
import flask
import logging
import signal
import sys

g_flask_app = flask.Flask(__name__)

ERROR_LOG = 'error.log'

def signal_handler(signal, frame):
    print("Exiting...")
    sys.exit(0)

@g_flask_app.route('/')
def index():
    """Renders the index page."""
    return ""

def main():
    global g_flask_app

    # Configure the error logger.
    logging.basicConfig(filename=ERROR_LOG, filemode='w', level=logging.DEBUG, format='%(asctime)s %(message)s', datefmt='%m/%d/%Y %I:%M:%S %p')

    # Make sure we have a compatible version of python.
    if sys.version_info[0] < 3:
        print("This application requires python 3.")
        sys.exit(1)

    # Parse the command line options.
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", type=str, action="store", default="localhost", help="The host interface on which to bind.", required=False)
    parser.add_argument("--port", type=int, action="store", default=5000, help="The host port on which to bind.", required=False)

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

    # Create the app object. It contains all the functionality.
    print(f"The app is running on {args.host}:{args.port}")
    g_flask_app.run(host=args.host, port=args.port)

if __name__=="__main__":
	main()
