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

"""Abstracts the configuration file."""
"""The example configuration file documents the purpose of each item."""

import configparser
import os

class Config(object):
    """Class that abstracts the configuration file."""
    """The example configuration file documents the purpose of each item."""

    def __init__(self):
        self.config = None
        super(Config, self).__init__()

    def load(self, config_file_name):
        """Loads the configuration file."""
        self.config = configparser.RawConfigParser(allow_no_value=True)
        self.config.read(config_file_name)

    def get_str(self, section, setting):
        """Utility function for reading a string from the configuration file."""
        try:
            value = self.config.get(section, setting)
            return value
        except configparser.NoOptionError:
            pass
        except configparser.NoSectionError:
            pass
        except:
            pass
        return ""

    def get_bool(self, section, setting):
        """Utility function for reading a boolean from the configuration file."""
        value = self.get_str(section, setting)
        return value.lower() == "true"

    def get_int(self, section, setting):
        """Utility function for reading an integer from the configuration file."""
        value = self.get_str(section, setting)
        if len(value) > 0:
            return int(value)
        return 0

    def get_database_url(self):
        """Returns the mongo database URL used to store results."""
        db_url = self.get_str('Database', 'URL')

        # Check for None or empty string. Use default in this case.
        if db_url is None:
            db_url = 'http://localhost:27017'
        return db_url

    def get_tty_path(self):
        """Returns the TTY path for reading from the microcontroller."""
        tty_path = self.get_str('TTY', 'Path')

        # Check for None or empty string. Use default in this case.
        if tty_path is None:
            tty_path = '/dev/ttyUSB0'
        return tty_path

    def get_tty_baud_rate(self):
        """Returns the TTY path for reading from the microcontroller."""
        baud = self.get_int('TTY', 'Baud')
        if baud is 0:
            baud = 9600
        return baud
