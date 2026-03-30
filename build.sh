#!/usr/bin/env bash

set -e

SOURCES=(
  main.cpp
  menu.cpp
  save_handler.cpp
  utils.cpp
  classes/book.cpp
  classes/student.cpp
  classes/library.cpp
)

g++ -std=c++17 -Wall -Wextra -pedantic "${SOURCES[@]}" -o app

echo "Build successful: ./app"
