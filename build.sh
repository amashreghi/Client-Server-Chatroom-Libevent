#!/bin/bash

libs=("-levent" "-lpthread")

gcc client.c -o bin/client ${libs[@]}
gcc server.c -o bin/server ${libs[@]}
