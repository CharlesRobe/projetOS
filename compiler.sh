#!/bin/bash
gcc main.c main_functions.c -o Formula1_OS
echo "[ OK ] Compile necessary files."
touch "circuits_inities.txt"
echo "[ OK ] Create file 'circuits_inities.txt'"
echo "All done. You may now run the command '$ ./Formula1_OS <circuit_name>' to execute the program."