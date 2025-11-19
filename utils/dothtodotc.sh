#!/bin/bash

# Exit on error
set -e

# Loop through all .h files in the directory
for file in *.h; do
    # Skip if no .h files exist
    [ -e "$file" ] || continue

    # Remove .h and add .c
    newname="${file%.h}.c"

    echo "Renaming '$file' → '$newname'"
    mv "$file" "$newname"
done

echo "Done!"

