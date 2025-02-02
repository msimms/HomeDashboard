# -*- coding: utf-8 -*-
#
# MIT License
# 
# Copyright (c) 2024 Mike Simms
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
import config
import logging
import pymongo
import serial
import sys
import time

def insert_into_collection(collection, doc):
    """Handles differences in document insertion between pymongo 3 and 4."""
    if int(pymongo.__version__[0]) < 4:
        result = collection.insert(doc)
    else:
        result = collection.insert_one(doc)
    return result is not None and result.inserted_id is not None 

def post_to_mongo(url, values):
    try:
        conn = pymongo.MongoClient(url)
        db = conn['statusdb']
        collection = db['indoor_air_quality']
        return insert_into_collection(collection, values)
    except pymongo.errors.ConnectionFailure as e:
        return False

def parse_air_quality_output(line):
    """
    Parses a string of space-separated numbers into a list of floats or integers.

    :param line: The string containing space-separated numbers.
    :result list: A list of numbers as floats or integers.
    """

    # Split the string into parts by whitespace
    parts = line.split()
    
    # Convert each part to a number (int or float)
    numbers = []
    for part in parts:
        if 'Error:' in part:
            logger = logging.getLogger()
            logger.error(part)
            break # This line won't contain anything useful, so just return
        elif '.' in part:
            numbers.append(float(part))  # Convert to float if it contains a decimal
        else:
            numbers.append(int(part))  # Convert to int otherwise
    return numbers

def read_from_tty(callback, device: str, baud_rate: int = 9600, timeout: int = 1):
    """
    Reads data from a TTY device.
    
    :param device: The path to the TTY device (e.g., '/dev/ttyUSB0').
    :param baud_rate: The baud rate for communication (default is 9600).
    :param timeout: The read timeout in seconds (default is 1 second).
    """
    try:
        keys = ['co2_ppm', 'temp_c', 'humidity', 'voc', 'voc_index']
        with serial.Serial(device, baud_rate, timeout=timeout) as ser:
            print(f"Listening to {device} at {baud_rate} baud.")
            while True:
                line = ser.readline().decode('utf-8').strip()
                values = parse_air_quality_output(line)
                if len(values) > 0:
                    dictionary = dict(zip(keys, values))
                    dictionary['ts'] = time.time()
                    callback(dictionary)
    except serial.SerialException as e:
        print(f"Error: {e}")
    except KeyboardInterrupt:
        print("\nExiting...")
    except Exception as e:
        print(f"Unexpected error: {e}")

def handle_reading(values):
    print(values)
    post_to_mongo(g_config.get_database_url(), values)

def main():
    global g_config

    # Make sure we have a compatible version of python.
    if sys.version_info[0] < 3:
        print("This application requires python 3.")
        sys.exit(1)

    # Parse the command line options.
    parser = argparse.ArgumentParser()
    parser.add_argument("--config-file", type=str, action="store", default="air_quality.config", help="The config file to use.", required=False)
    parser.add_argument("--log-file", type=str, action="store", default="air_quality.log", help="The log file to use.", required=False)

    try:
        args = parser.parse_args()
    except IOError as e:
        parser.error(e)
        sys.exit(1)

    # Configure the error logger.
    logging.basicConfig(filename=args.log_file, filemode='w', level=logging.DEBUG, format='%(asctime)s %(message)s', datefmt='%m/%d/%Y %I:%M:%S %p')

    # Load the config file.
    g_config = config.Config()
    if len(args.config_file) > 0:
        g_config.load(args.config_file)

    read_from_tty(handle_reading, g_config.get_tty_path(), g_config.get_tty_baud_rate())

if __name__ == "__main__":
    main()
