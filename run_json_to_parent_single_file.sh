#!/bin/bash

chmod 777 test_folder2/jax.c
chmod 777 test_folder2/u.txt

# Compile the C program
gcc json_to_parent.c -o program -ljson-c
if [ $? -ne 0 ]; then
    echo "Compilation failed"
    exit 1
fi

# Run the compiled program with the specified directory
./program "test_folder2"
if [ $? -ne 0 ]; then
    echo "Execution failed"
    exit 1
fi

echo "First run finished"
echo ""
# Move specified files back to the test folder

chmod 000 test_folder2/jax.c
chmod 000 test_folder2/u.txt

# Sleep for a brief moment to ensure file operations complete
sleep 1

# Repeat the compilation and execution process
gcc json_to_parent.c -o program -ljson-c
if [ $? -ne 0 ]; then
    echo "Compilation failed"
    exit 1
fi

./program "test_folder2"
if [ $? -ne 0 ]; then
    echo "Execution failed"
    exit 1
fi

mv "izolated_space_dir/jax.c" "test_folder2"
mv "izolated_space_dir/u.txt" "test_folder2"

echo "Experiment completed successfully."