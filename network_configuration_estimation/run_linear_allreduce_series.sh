#!/bin/bash

# --- Configuration ---
EXECUTABLE="./a.out"
OUTPUT_FILE="points_for_viz.csv"

# Array of process counts (P) to test (powers of two from 4 to 1024)
PROCESS_COUNTS=1024 #(4 8 16 32 64 128 256 512 1024)
# ---------------------

echo "Starting Linear Allreduce scaling experiment across P = ${PROCESS_COUNTS[*]}..."

# 1. Clean up previous file and write the header ONLY once.
echo "P,m,T,X1,X2,X3" > $OUTPUT_FILE
echo "--- Created new consolidated file: $OUTPUT_FILE ---"

# 2. Loop through all defined process counts
for P in "${PROCESS_COUNTS[@]}"; do
    echo "  Running with P=$P processes..."

    # Run the MPI program and append the data (excluding the header line printed by the C code)
    # The C code prints the header on rank 0, so we skip the first line of output using 'tail -n +2'
    # 2>/dev/null suppresses any MPI warnings/errors (stderr)
    smpirun -platform platform.xml -n $P $EXECUTABLE 2>/dev/null | tail -n +2 >> $OUTPUT_FILE

    if [ $? -ne 0 ]; then
        echo "Error: smpirun with P=$P failed. Please check your executable or MPI environment."
        exit 1
    fi
done

echo "==================================================="
echo "Experiment complete! Data ready in '$OUTPUT_FILE'."
echo "Next step: Run the Python analysis script: python3 analyze_regression.py"
echo "==================================================="
