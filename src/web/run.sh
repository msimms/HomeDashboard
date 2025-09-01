#!/bin/bash

source venv/bin/activate
python3 dashboard.py --database="mongodb://localhost:27018/"
