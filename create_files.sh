#!/bin/bash

# Array of strings
strings=("lin" "rnos" "rab" "rd" "rs")

# Loop over all ordered pairs (i ≠ j)
base1="./utils/"
for s1 in "${strings[@]}"; do
  for s2 in "${strings[@]}"; do

    filename="${s1}_${s2}.c"
    filepath="${base1}${filename}"

    # Create file only if it does not exist
    if [ ! -e "$filepath" ]; then
      touch "$filepath"
      echo "Created file: $filename"
    else
      echo "File already exists: $filename — skipping"
    fi

  done
done

