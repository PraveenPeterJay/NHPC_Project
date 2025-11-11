#!/bin/bash

# Array of strings
strings=("lin" "rnos" "rab" "rd" "rs")

# Loop over all ordered pairs (i ≠ j)
base1="./utils/"
for s1 in "${strings[@]}"; do
  for s2 in "${strings[@]}"; do
    if [[ "$s1" != "$s2" ]]; then
      filename="${s1}_${s2}"
      touch "$base1$filename.h"
      rm "$base1$filename"
      echo "Created file: $filename"
    fi
  done
done

