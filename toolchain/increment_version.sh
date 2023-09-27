#!/bin/bash

# Define the file path
file_path="kernel/src/kernel.cpp"

# Check if the file exists
if [ ! -f "$file_path" ]; then
  echo "Error: File $file_path does not exist."
  exit 1
fi

# Read the current value of buildCount
current_value=$(grep -oP 'int buildCount = \K\d+' "$file_path")

# Check if the current value is a valid integer
if ! [[ "$current_value" =~ ^[0-9]+$ ]]; then
  echo "Error: Unable to read the current buildCount value."
  exit 1
fi

# Increment the current value by 1
new_value=$((current_value + 1))

# Replace the old buildCount value with the new one in the file
sed -i "s/int buildCount = $current_value;/int buildCount = $new_value;/" "$file_path"

echo "Incremented buildCount in $file_path from $current_value to $new_value."
