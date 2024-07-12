#!/bin/bash

VERBOSE=false

# Function to print verbose output if enabled
verbose_print() {
    if [ "$VERBOSE" = true ]; then
        echo "$@"
    fi
}

# Function to stop a process by name
stop_process() {
    local process_name=$1
    local pids

    # Find the PIDs of the process
    pids=$(pgrep -d ' ' -f "$process_name")

    if [[ -n "$pids" ]]; then
        verbose_print "Stopping processes matching $process_name with PIDs: $pids"
        for pid in $pids; do
            sudo kill "$pid"
            sleep 1  # Give some time for the process to terminate
            pid=$(pgrep -f "$process_name")  # Check if the process is still running
            if [[ -n "$pid" ]]; then
                verbose_print "Process $process_name (PID $pid) did not stop, using kill -9"
                sudo kill -9 "$pid"
            else
                verbose_print "Process $process_name (PID $pid) stopped successfully"
            fi
        done
    else
        verbose_print "Process $process_name not found"
    fi
}

# Parse command-line options
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        -v|--verbose)
        VERBOSE=true
        shift
        ;;
        *)
        echo "Unknown option: $key"
        exit 1
        ;;
    esac
done

# Stop the LED control program
stop_process "ws_to_led"

# Stop Xvfb
stop_process "Xvfb :99"

# Stop the game
stop_process "lt3.x86_64"

echo "All processes stopped"

