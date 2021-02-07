#!/bin/bash


cd "$(dirname "$0")"
echo "shell script directory: "$PWD""
mkdir bin
echo "Installing echo server and client..."
make
echo "Echo server and client installed"
./bin/server
./bin/client