#!/bin/bash

# This script generates a sequence of unique random positive integers.
# Usage: ./generate_numbers.sh <number_of_elements>

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <number_of_elements>" >&2
    exit 1
fi

NUM_ELEMENTS=$1
MAX_VALUE=1000000 # Upper bound for random numbers

# Use jot on macOS or shuf on Linux for efficient random number generation
if command -v jot &> /dev/null; then
    # jot can generate random numbers, -r flag
    # jot <count> <min> <max>
    jot -r "$NUM_ELEMENTS" 1 "$MAX_VALUE" | tr '\n' ' '
else
    # shuf can select N random lines from an input range
    # shuf -i <min-max> -n <count>
    shuf -i "1-$MAX_VALUE" -n "$NUM_ELEMENTS" | tr '\n' ' '
fi
