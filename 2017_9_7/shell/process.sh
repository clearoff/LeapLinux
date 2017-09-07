#!/bin/bash

i=0
str=""
arr={"|" "/" "-" }
while [ $i -le 100 ]
do
	let index=i%4
	printf "[%.100s][%d%%][\e[43;46;1m%c\e[0m]\r" "$str" "$i" "${arr[$index]}"
	sleep 0.1
	let i++
	str+='#'
done
