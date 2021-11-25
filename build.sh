#!/bin/bash

set -xe

cc -Werror -std=c11 -pedantic -ggdb -o poker poker.c
./poker
