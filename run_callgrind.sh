#!/bin/sh
valgrind --tool=callgrind  --dump-instr=yes --dump-line=yes --collect-jumps=yes build/net-calc 127.0.0.1 9090
