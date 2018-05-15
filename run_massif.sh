#!/bin/sh
valgrind --tool=massif build/net-calc 127.0.0.1 9090
