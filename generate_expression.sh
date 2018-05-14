#!/bin/sh
printf 0
for j in `seq 400000`
do
for i in `seq 1 1000`
do
  printf "+$i"
done
done
printf "\n"
