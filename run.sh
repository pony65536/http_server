#!/bin/sh
set -e
cmake -B build
cmake --build build
build/`arch`/bin/http_server