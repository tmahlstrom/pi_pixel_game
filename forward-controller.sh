#!/bin/bash

# Function to handle keyboard events and forward to Xvfb
forward_keyboard_input() {
    # Keep track of currently pressed keys
    declare -A pressed_keys

    while read -r line; do
        if [[ $line =~ "EV_KEY" ]]; then
            key_code=$(echo "$line" | grep -oP '(?<=code )\d+')
            event_type=$(echo "$line" | grep -oP '(?<=value )\d+')

            if [[ $event_type == "1" ]]; then
                # Key press event
                if [[ ! ${pressed_keys[$key_code]} ]]; then
                    # Key is not already pressed, handle it
                    pressed_keys[$key_code]=1  # Mark key as pressed
                    handle_key_press "$key_code"
                fi
            elif [[ $event_type == "0" ]]; then
                # Key release event
                if [[ ${pressed_keys[$key_code]} ]]; then
                    # Key was pressed, handle release
                    unset pressed_keys[$key_code]
                    handle_key_release "$key_code"
                fi
            fi
        fi
    done
}

# Function to handle key press events
handle_key_press() {
    key_code=$1
    case "$key_code" in
        204) xdotool keydown --window :99 space ;;
        106) xdotool keydown --window :99 Right ;;
        105) xdotool keydown --window :99 Left ;;
        *) echo "Unknown key pressed with code $key_code" ;;
    esac
}

# Function to handle key release events
handle_key_release() {
    key_code=$1
    case "$key_code" in
        204) xdotool keyup --window :99 space ;;
        106) xdotool keyup --window :99 Right ;;
        105) xdotool keyup --window :99 Left ;;
        *) echo "Unknown key released with code $key_code" ;;
    esac
}

echo "Reading keyboard input..."
export DISPLAY=:99  # Set the display environment variable

# Start evtest in the background and pass its output to the function
sudo evtest /dev/input/event4 | forward_keyboard_input

