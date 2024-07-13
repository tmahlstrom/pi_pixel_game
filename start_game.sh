#!/bin/bash

LOG_FILE=/home/tylerahlstrom/debug_start_game.log
exec > >(tee -a $LOG_FILE) 2>&1

echo "Script started at $(date)"

# Check if the LED control program is running
if pgrep -f "ws_to_led --led-rows=64 --led-cols=256 --led-slowdown-gpio=4 -f 5x7.bdf" > /dev/null; then
    echo "LED control program is already running."
    exit 1
else
    echo "LED control program is not running. Proceeding..."
fi

# Check if Xvfb is running
if pgrep -f "Xvfb :99" > /dev/null; then
    echo "Xvfb is already running."
    exit 1
else
    echo "Xvfb is not running. Proceeding..."
fi

# Check if the game is running
if pgrep -f "lt3.x86_64" > /dev/null; then
    echo "Game is already running."
    exit 1
else
    echo "Game is not running. Proceeding..."
fi

# Start the LED control program
echo "Starting LED control program..."
cd /home/tylerahlstrom/ws_to_led
sudo ./ws_to_led --led-rows=64 --led-cols=256 --led-slowdown-gpio=4 -f 5x7.bdf >> $LOG_FILE 2>&1 &
LED_PID=$!
echo "LED control program started with PID $LED_PID"
cd /home/tylerahlstrom

sleep 5  # Adjust the sleep duration as needed

# Start Xvfb on display :99
echo "Starting Xvfb..."
Xvfb :99 -screen 0 320x240x16 >> $LOG_FILE 2>&1 &
XVFB_PID=$!
echo "Xvfb started with PID $XVFB_PID"

sleep 5  # Adjust the sleep duration as needed

# Set the DISPLAY environment variable
export DISPLAY=:99
echo "DISPLAY set to $DISPLAY"

sleep 5  # Adjust the sleep duration as needed

# Start the game
echo "Starting the game..."
/home/tylerahlstrom/games/lt3.x86_64 >> $LOG_FILE 2>&1 &
GAME_PID=$!
echo "Game started with PID $GAME_PID"

sleep 30
echo "Script completed at $(date)"

