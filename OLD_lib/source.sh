#!/bin/bash

#########################
# Sourcing the library: #
#########################

# Set SCRIPT_DIR to the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Original CPLUS_INCLUDE_PATH
OLD_PATH="$CPLUS_INCLUDE_PATH"
NEW_PATH=""

# List of directories to add (relative to SCRIPT_DIR)
dirs=(
    "$SCRIPT_DIR"
    "$SCRIPT_DIR/MTObjects"
    "$SCRIPT_DIR/Classes"
    "$SCRIPT_DIR/Analyse"
)

# Add each directory if not already in the path
for dir in "${dirs[@]}"; do
    if [[ ":$OLD_PATH:$NEW_PATH:" != *":$dir:"* ]]; then
        NEW_PATH="$NEW_PATH:$dir"
    fi
done

# Remove leading colon if NEW_PATH is not empty
if [[ -n "$NEW_PATH" ]]; then
    NEW_PATH="${NEW_PATH#:}"
fi

# Remove trailing colon if NEW_PATH is not empty
if [[ -n "$NEW_PATH" ]]; then
    NEW_PATH="${NEW_PATH%:}"
fi

# Combine with original path
if [[ -n "$NEW_PATH" ]]; then
    export CPLUS_INCLUDE_PATH="$NEW_PATH:$OLD_PATH"
else
    export CPLUS_INCLUDE_PATH="$OLD_PATH"
fi
