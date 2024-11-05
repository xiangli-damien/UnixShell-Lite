#!/bin/bash

rm -f ./bin/msh

gcc -I./include/ -o ./bin/msh src/*.c

if [ $? -eq 0 ]; then
    echo "Compilation successful."
    chmod +x ./bin/msh

else
    echo "Compilation failed. HaHa"
fi