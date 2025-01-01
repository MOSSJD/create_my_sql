#!/bin/bash
g++ -c -O2 main.cpp -o out/main.o
g++ -c -O2 my_sql.cpp -o out/my_sql.o
g++ out/main.o out/my_sql.o -O2 -o out/main