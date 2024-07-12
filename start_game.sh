#!/bin/bash

# Check if the LED control program is running
if pgrep -f "ws_to_led --led-rows=64 --led-cols=256 --led-slowdown-gpio=4 -f 5x7.bdf" > /dev/null; then
    echo "LED control program is already running."
    exit 1
fi

# Check if Xvfb is running
if pgrep -f "Xvfb :99" > /dev/null; then
    echo "Xvfb is already running."
    exit 1
fi

# Check if the game is running
if pgrep -f "lt3.x86_64" > /dev/null; then
    echo "Game is already running."
    exit 1
fi

# Start the LED control program
cd ~/ws_to_led
sudo ./ws_to_led --led-rows=64 --led-cols=256 --led-slowdown-gpio=4 -f 5x7.bdf &
cd ~
sleep 1  # Adjust the sleep duration as needed

# Start Xvfb on display :99
Xvfb :99 -screen 0 320x240x16 &
sleep 1  # Adjust the sleep duration as needed

# Set the DISPLAY environment variable
export DISPLAY=:99
sleep 1  # Adjust the sleep duration as needed

# Start the game
games/./lt3.x86_64 &

