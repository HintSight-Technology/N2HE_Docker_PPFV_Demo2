#!/bin/bash

# Path to the encrypted result file
FILE="ios_ppfr/put.txt"

# Function to check if the file is being used by another process
is_file_in_use() {
    lsof "$FILE" &> /dev/null
    return $?
}

# Function to handle the file verification and execution
verify_and_run() {
    if [[ -e "$FILE" ]]; then
        if ! is_file_in_use; then
            # Run the C++ executable 'dec'
	    ./eval            
echo " "
            echo "SHELL: Decryption finished. "
            echo "SHELL: ====================================="
            return 0
        else
            echo "SHELL: File is in use. Retrying in 1 second..."
        fi
    else
        echo "SHELL: File does not exist. Retrying in 1 second..."
    fi

    # Pause for 1 second before retrying
    sleep 1
    return 1
}

# Ensure inotifywait is installed
if ! command -v inotifywait &> /dev/null; then
    echo "SHELL: inotifywait could not be found, please install inotify-tools package."
    exit 1
fi
echo "SHELL: Detection set for ios_ppfr/put.txt"
echo "SHELL: ====================================="
# Monitor the file for changes
while true; do
    if inotifywait -qq -e modify "$FILE"; then
        echo "SHELL: File ios_ppfr/put.txt changed, verifying..."
        echo " "
        while ! verify_and_run; do
            # Retry verification if it failed
            :
        done
    else
        echo "SHELL: No changes detected. Pausing for 10 seconds..."
        sleep 10
    fi
done
