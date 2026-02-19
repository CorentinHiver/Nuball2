#!/bin/bash

# Definition of the command to execute
executable="./main"
multi_threaded_command="$executable -U -m 10 -p 2"
mono_threaded_command="$executable -U -m 1 -p 2"

# Add -d option if specified
if [ "$1" = "d" ]; then
  multi_threaded_command="$multi_threaded_command -d"
  mono_threaded_command="$mono_threaded_command -d"
fi

number_of_attempts=0

# Loop until the program executes successfully
while true; do
  nombre_de_tentatives=$((nombre_de_tentatives + 1)) # Increment the number of attempts
  echo "Launching forced conversion for the $number_of_attempts time..."

  # If the executable doesn't exist or not executable, rebuild it
  if ! [ -x "$executable" ]; then
    make opt
    # Check if make was successful
    if [ $? -eq 0 ]; then
      echo "Make completed successfully."
    else
      echo "Make encountered an error."
      exit 1
    fi
  fi

  # Launch the multi-threaded command
  $multi_threaded_command

  # Check the exit code of the command
  if [ $? -eq 0 ]; then
    echo "Conversion finished successfully!"
    break # Exit the loop if the command succeeded
  else
    echo "Conversion failed... Switching to mono-threaded mode for 5 minutes."

    # Launch the mono-threaded command in the background
    $mono_threaded_command &
    background_pid=$!

    # Wait for 5 minutes
    sleep 300

    # Check if the background process is still running, send SIGINT if it is
    if ps -p $background_pid > /dev/null; then
      echo "Sending SIGINT to the background process..."
      kill -2 $background_pid
    fi
  fi
done

echo "Conversion should be finished by now! Total attempts: $number_of_attempts"