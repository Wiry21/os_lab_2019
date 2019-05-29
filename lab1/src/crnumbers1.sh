#!/bin/bash
RANDOM=$$
for i in `seq 150`
do
echo $RANDOM >> numbers.txt
done