#!/bin/bash
g++ -c -O2 utils.cpp -o out/utils.o
g++ -c -O2 database.cpp -o out/database.o
g++ -c -O2 shell.cpp -o out/shell.o
g++ -c -O2 my_sql.cpp -o out/my_sql.o
g++ -c -O2 main.cpp -o out/main.o
g++ out/utils.o out/database.o out/shell.o out/my_sql.o out/main.o -O2 -o out/main