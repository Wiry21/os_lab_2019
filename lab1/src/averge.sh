#!/bin/sh
count=$#
average=0
summ=0

args=0
for var in "$@"
do

summ=$(($var+$summ))


args=$(($args+1))

done

echo "Параметров $args"

average=$(($summ / $count))
echo "Среднее арифмитическое  равно $average" 