# -*- coding: utf-8 -*-
# 
# # MIT License
# 
# Copyright (c) 2017 Mike Simms
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
"""Database base classes"""

import logging
import os
import pymongo
import sqlite3
import sys
import traceback
from bson.objectid import ObjectId

DATABASE_ID_KEY = "_id"
USERNAME_KEY = "username" # Login name for a user
PASSWORD_KEY = "password" # User's password
REALNAME_KEY = "realname" # User's real name
HASH_KEY = "hash" # Password hash

# Keys associated with session management.
SESSION_COOKIE_KEY = "cookie"
SESSION_USER_KEY = "user"
SESSION_EXPIRY_KEY = "expiry"

# Keys associated with scale calibration.
SCALE_NAME_KEY = "name"
SCALE_TARE_VALUE_KEY = "tare_value"
SCALE_CALIBRATION_VALUE_KEY = "calibration_value"
SCALE_CALIBRATION_WEIGHT_KEY = "calibration_weight"
SCALE_FULL_VALUE_KEY = "full_value"
SCALE_RAW_VALUE_KEY = "raw_value"

# Collection names.
COLLECTION_USERS = "users"
COLLECTION_SESSIONS = "sessions"
COLLECTION_SCALE_CALIBRATIONS = "scale_calibrations"
COLLECTION_API_KEYS = "api_keys"
COLLECTION_INDOOR_AIR_QUALITY = "indoor_air_quality"
COLLECTION_PATIO_MONITOR = "patio_monitor"
COLLECTION_AC = "ac"
COLLECTION_KEG = "keg"
COLLECTION_WEBSITE_STATUS = "website_status"
COLLECTION_LIMITS = "limits"

# Keys associated with API key management.
API_KEY = "key"
API_EXPIRY_KEY = "expiry"
API_USER_KEY = "user"

class DatabaseException(Exception):
    """Exception thrown by the database."""

    def __init__(self, *args):
        Exception.__init__(self, args)

    def __init__(self, message):
        self.message = message
        Exception.__init__(self, message)

class Database(object):
    """Base class for a database. Encapsulates common functionality."""
    db_file = ""

    def __init__(self):
        super(Database, self).__init__()

    def log_error(self, log_str):
        """Writes an error message to the log file."""
        logger = logging.getLogger()
        logger.error(log_str)

    def log_info(self, log_str):
        """Writes an info message to the log file."""
        logger = logging.getLogger()
        logger.info(log_str)

    def is_quoted(self, log_str):
        """Determines if the provided string starts and ends with a double quote."""
        if len(log_str) < 2:
            return False
        return log_str[0] == '\"' and log_str[len(log_str)-1] == '\"'

    def quote_identifier(self, log_str, errors="strict"):
        """Adds quotes to the given string if they do not already exist."""
        if self.is_quoted(log_str):
            return log_str
        encodable = log_str.encode("utf-8", errors).decode("utf-8")
        null_index = encodable.find("\x00")
        if null_index >= 0:
            return ""
        return "\"" + encodable.replace("\"", "\"\"") + "\""

class SqliteDatabase(Database):
    """Abstract Sqlite database implementation."""

    def __init__(self, root_dir, file_name):
        self.db_file_name = os.path.join(root_dir, file_name)
        Database.__init__(self)

    def connect(self):
        """Inherited from the base class and unused."""
        pass

    def execute(self, sql):
        """Executes the specified SQL query."""
        try:
            con = sqlite3.connect(self.db_file_name)
            with con:
                cur = con.cursor()
                cur.execute(sql)
                return cur.fetchall()
        except:
            self.log_error("Database error:\n\tfile = " + self.db_file_name + "\n\tsql = " + self.quote_identifier(sql))
        finally:
            if con:
                con.close()
        return None

def insert_into_collection(collection, doc):
    """Handles differences in document insertion between pymongo 3 and 4."""
    if int(pymongo.__version__[0]) < 4:
        result = collection.insert(doc)
    else:
        result = collection.insert_one(doc)
    return result is not None and result.inserted_id is not None 

def update_collection(collection, doc):
    """Handles differences in document updates between pymongo 3 and 4."""
    if int(pymongo.__version__[0]) < 4:
        collection.save(doc)
        return True
    else:
        query = { DATABASE_ID_KEY: doc[DATABASE_ID_KEY] }
        new_values = { "$set" : doc }
        result = collection.update_one(query, new_values)
        return result.matched_count > 0 

class AppMongoDatabase(Database):
    """Mongo DB implementation of the application database."""

    def __init__(self):
        Database.__init__(self)

    def connect(self, database_url):
        """Connects/creates the database."""
        try:
            # Connect to the database server.
            self.conn = pymongo.MongoClient(database_url)
            if self.conn is None:
                raise DatabaseException("Could not connect to MongoDB.")

            # Get a handle to the database.
            self.database = self.conn['statusdb']
            if self.database is None:
                raise DatabaseException("Could not connect to MongoDB.")

            # Handles to the various collections.
            self.users_collection = self.database[COLLECTION_USERS]
            self.sessions_collection = self.database[COLLECTION_SESSIONS]
            self.scale_calibrations_collection = self.database[COLLECTION_SCALE_CALIBRATIONS]
            self.api_keys_collection = self.database[COLLECTION_API_KEYS]
            self.limits_collection = self.database[COLLECTION_LIMITS]
            self.indoor_air_quality = self.database[COLLECTION_INDOOR_AIR_QUALITY]
            self.patio_monitor = self.database[COLLECTION_PATIO_MONITOR]
            self.ac_monitor = self.database[COLLECTION_AC]
            self.keg = self.database[COLLECTION_KEG]
            self.website_status = self.database[COLLECTION_WEBSITE_STATUS]
        except pymongo.errors.ConnectionFailure as e:
            raise DatabaseException("Could not connect to MongoDB: %s" % e)

    #
    # User management methods
    #

    def create_user(self, username, realname, passhash):
        """Create method for a user."""
        if username is None:
            self.log_error(self.create_user.__name__ + ": Unexpected empty object: username")
            return False
        if realname is None:
            self.log_error(self.create_user.__name__ + ": Unexpected empty object: realname")
            return False
        if passhash is None:
            self.log_error(self.create_user.__name__ + ": Unexpected empty object: passhash")
            return False
        if len(username) == 0:
            self.log_error(self.create_user.__name__ + ": username too short")
            return False
        if len(realname) == 0:
            self.log_error(self.create_user.__name__ + ": realname too short")
            return False
        if len(passhash) == 0:
            self.log_error(self.create_user.__name__ + ": hash too short")
            return False

        try:
            post = { USERNAME_KEY: username, REALNAME_KEY: realname, HASH_KEY: passhash }
            return insert_into_collection(self.users_collection, post)
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return False

    def retrieve_user(self, username):
        """Retrieve method for a user."""
        if username is None:
            self.log_error(self.retrieve_user.__name__ + ": Unexpected empty object: username")
            return None, None, None
        if len(username) == 0:
            self.log_error(self.retrieve_user.__name__ + ": username is empty")
            return None, None, None

        try:
            # Find the user.
            result_keys = { DATABASE_ID_KEY: 1, HASH_KEY: 1, REALNAME_KEY: 1 }
            user = self.users_collection.find_one({ USERNAME_KEY: username }, result_keys)

            # If the user was found.
            if user is not None:
                return str(user[DATABASE_ID_KEY]), user[HASH_KEY], str(user[REALNAME_KEY])
            return None, None, None
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return None, None, None

    def retrieve_user_from_id(self, user_id):
        """Retrieve method for a user."""
        if user_id is None:
            self.log_error(self.retrieve_user_from_id.__name__ + ": Unexpected empty object: user_id")
            return None, None

        try:
            # Find the user.
            user_id_obj = ObjectId(str(user_id))
            result_keys = { USERNAME_KEY: 1, REALNAME_KEY: 1 }
            user = self.users_collection.find_one({ DATABASE_ID_KEY: user_id_obj }, result_keys)

            # If the user was found.
            if user is not None:
                return user[USERNAME_KEY], user[REALNAME_KEY]
            return None, None
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return None, None

    def update_user(self, user_id, username, realname, passhash):
        """Update method for a user."""
        if user_id is None:
            self.log_error(self.update_user.__name__ + ": Unexpected empty object: user_id")
            return False
        if username is None:
            self.log_error(self.update_user.__name__ + ": Unexpected empty object: username")
            return False
        if realname is None:
            self.log_error(self.update_user.__name__ + ": Unexpected empty object: realname")
            return False
        if len(username) == 0:
            self.log_error(self.update_user.__name__ + ": username too short")
            return False
        if len(realname) == 0:
            self.log_error(self.update_user.__name__ + ": realname too short")
            return False

        try:
            # Find the user.
            user_id_obj = ObjectId(str(user_id))
            user = self.users_collection.find_one({ DATABASE_ID_KEY: user_id_obj })

            # If the user was found.
            if user is not None:
                user[USERNAME_KEY] = username
                user[REALNAME_KEY] = realname
                if passhash is not None:
                    user[HASH_KEY] = passhash
                return update_collection(self.users_collection, user)
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return False

    def delete_user(self, user_id):
        """Delete method for a user."""
        if user_id is None:
            self.log_error(self.delete_user.__name__ + ": Unexpected empty object: user_id")
            return False

        try:
            user_id_obj = ObjectId(str(user_id))
            deleted_result = self.users_collection.delete_one({ DATABASE_ID_KEY: user_id_obj })
            if deleted_result is not None:
                return True
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return False

    #
    # Session cookie management methods
    #

    def create_session_cookie(self, user, cookie, expiry):
        """Create method for a session cookie."""
        if user is None:
            raise Exception("Unexpected empty object: user")
        if cookie is None:
            raise Exception("Unexpected empty object: cookie")
        if expiry is None:
            raise Exception("Unexpected empty object: expiry")

        try:
            post = { SESSION_COOKIE_KEY: cookie, SESSION_USER_KEY: user, SESSION_EXPIRY_KEY: expiry }
            return insert_into_collection(self.sessions_collection, post)
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return False

    def retrieve_session_data(self, cookie):
        """Retrieve method for session data."""
        if cookie is None:
            raise Exception("Unexpected empty object: cookie")

        try:
            session_data = self.sessions_collection.find_one({ SESSION_COOKIE_KEY: cookie })
            if session_data is not None:
                return session_data[SESSION_USER_KEY], session_data[SESSION_EXPIRY_KEY]
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return (None, None)

    def delete_session_cookie(self, cookie):
        """Delete method for a session cookie."""
        if cookie is None:
            raise Exception("Unexpected empty object: cookie")

        try:
            deleted_result = self.sessions_collection.delete_one({ SESSION_COOKIE_KEY: cookie })
            if deleted_result is not None:
                return True
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return False

    #
    # API key management methods
    #

    def create_api_key(self, key, expiry, user):
        """Create method for an API key."""
        if key is None:
            raise Exception("Unexpected empty object: key")
        if expiry is None:
            raise Exception("Unexpected empty object: expiry")
        if user is None:
            raise Exception("Unexpected empty object: user")

        try:
            post = { API_KEY: str(key), API_EXPIRY_KEY: expiry, API_USER_KEY: user }
            return insert_into_collection(self.api_keys_collection, post)
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return False

    def retrieve_api_key(self, key):
        """Retrieve method for data associated with an API key."""
        if key is None:
            raise Exception("Unexpected empty object: key")

        try:
            api_key = self.api_keys_collection.find_one({ API_KEY: key })
            if api_key is not None:
                return api_key[API_USER_KEY]
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return None

    def retrieve_api_keys(self, user):
        """Retrieve method for API keys associated with a specific user."""
        if user is None:
            raise Exception("Unexpected empty object: user")

        try:
            result_keys = { DATABASE_ID_KEY: 0, API_KEY: 1, API_EXPIRY_KEY: 1 }
            api_keys = self.api_keys_collection.find({ API_USER_KEY: user }, result_keys)
            return list(api_keys)
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return []

    def delete_api_key(self, key):
        """Delete method for an API key."""
        if key is None:
            raise Exception("Unexpected empty object: key")

        try:
            deleted_result = self.api_keys_collection.delete_one({ API_KEY: key })
            if deleted_result is not None:
                return True
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return False

    #
    # Sensor limit methods
    #

    def create_sensor_limit(self, key, lower_limit, upper_limit):
        """Create method for sensor limits."""
        if key is None:
            raise Exception("Unexpected empty object: key")
        if lower_limit is None:
            raise Exception("Unexpected empty object: lower_limit")
        if upper_limit is None:
            raise Exception("Unexpected empty object: upper_limit")

        try:
            post = { "key": str(key), "lower_limit": lower_limit, "upper_limit": upper_limit }
            return insert_into_collection(self.limits_collection, post)
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return False

    def retrieve_sensor_limits(self, key):
        """Retrieve method for sensor limits."""
        if key is None:
            raise Exception("Unexpected empty object: key")

        try:
            limits_result = self.limits_collection.find_one({ "key": key }, {})
            if limits_result is not None:
                return limits_result["lower_limit"], limits_result["upper_limit"]
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return None, None

    def delete_sensor_limits(self, key):
        """Delete method for a user."""
        if key is None:
            raise Exception("Unexpected empty object: key")

        try:
            deleted_result = self.limits_collection.delete_many({ "key": key })
            if deleted_result is not None:
                return True
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return False

    #
    # Status methods
    #

    def create_status(self, collection_name, values):
        """Create method for status."""
        if collection_name is None:
            raise Exception("Unexpected empty object: collection_name")
        if values is None:
            raise Exception("Unexpected empty object: values")

        try:
            if collection_name == COLLECTION_PATIO_MONITOR:
                return insert_into_collection(self.patio_monitor, values)
            if collection_name == COLLECTION_AC:
                return insert_into_collection(self.ac_monitor, values)
            if collection_name == COLLECTION_KEG:
                return insert_into_collection(self.keg, values)
            if collection_name == COLLECTION_WEBSITE_STATUS:
                return insert_into_collection(self.website_status, values)
            raise Exception("Unknown collection")
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return False

    #
    # Indoor air quality methods
    #

    def retrieve_air_quality(self, min_ts):
        """Retrieve method for air quality measurements."""
        try:
            filter = {}
            if min_ts > 0:
                filter = { "ts": { "$gt": min_ts } }
            return self.indoor_air_quality.find(filter, { "_id": 0 })
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return []

    #
    # Patio monitor methods
    #

    def retrieve_patio_status(self, min_ts):
        """Retrieve method for patio monitor measurements."""
        try:
            filter = {}
            if min_ts > 0:
                filter = { "ts": { "$gt": min_ts } }
            return self.patio_monitor.find(filter, {"_id": 0})
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return []

    #
    # AC monitor methods
    #

    def retrieve_ac_status(self, min_ts):
        """Retrieve method for AC measurements."""
        try:
            filter = {}
            if min_ts > 0:
                filter = { "ts": { "$gt": min_ts } }
            return self.ac_monitor.find(filter, {"_id": 0})
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return []
        
    #
    # Keg monitor methods
    #

    def retrieve_keg_status(self, min_ts):
        """Retrieve method for keg measurements (temp, amount left in the keg, etc)."""
        try:
            filter = {}
            if min_ts > 0:
                filter = { "ts": { "$gt": min_ts } }
            return self.keg.find(filter, {"_id": 0})
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return []

    #
    # Scale calibration methods
    #

    def create_scale_calibration(self, name, tare_value, cal_value, cal_weight, full_value):
        """Create method for scale calibrations."""
        try:
            post = { SCALE_NAME_KEY: name, SCALE_TARE_VALUE_KEY: tare_value, SCALE_CALIBRATION_VALUE_KEY: cal_value, SCALE_FULL_VALUE_KEY: full_value }
            return insert_into_collection(self.scale_calibrations_collection, post)
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return False

    def update_scale_calibration(self, name, tare_value, cal_value, cal_weight, full_value):
        """Update method for scale calibrations."""
        try:
            cal = self.scale_calibrations_collection.find_one({ SCALE_NAME_KEY: name }, {"_id": 0 })
            if cal is not None:
                if tare_value is not None:
                    cal[SCALE_TARE_VALUE_KEY] = tare_value
                if cal_value is not None and cal_weight is not None:
                    cal[SCALE_CALIBRATION_VALUE_KEY] = cal_value
                    cal[SCALE_CALIBRATION_WEIGHT_KEY] = cal_weight
                if full_value is not None:
                    cal[SCALE_FULL_VALUE_KEY] = full_value
                return update_collection(self.scale_calibrations_collection, cal)
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return False

    def retrieve_scale_calibration(self, name):
        """Retrieve method for scale calibrations."""
        try:
            cal = self.scale_calibrations_collection.find_one({ SCALE_NAME_KEY: name }, {"_id": 0 })
            if cal is None:
                cal = {}
            if SCALE_TARE_VALUE_KEY not in cal:
                cal[SCALE_TARE_VALUE_KEY] = None
            if SCALE_CALIBRATION_VALUE_KEY not in cal:
                cal[SCALE_CALIBRATION_VALUE_KEY] = None
            if SCALE_CALIBRATION_WEIGHT_KEY not in cal:
                cal[SCALE_CALIBRATION_WEIGHT_KEY] = None
            if SCALE_FULL_VALUE_KEY not in cal:
                cal[SCALE_FULL_VALUE_KEY] = None
            return cal
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return {}

    #
    # Website status methods
    #

    def retrieve_website_status(self, min_ts):
        """Retrieve method for website statuses."""
        try:
            filter = {}
            if min_ts > 0:
                filter = {"ts": {"$gt": min_ts}}
            return self.website_status.find(filter, {"_id": 0})
        except:
            self.log_error(traceback.format_exc())
            self.log_error(sys.exc_info()[0])
        return []
