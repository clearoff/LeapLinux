#!/bin/bash

function mut(){
	local tmp
	local tmp1 
	tmp="$1"
	if [ ${tmp} -eq 1 ];then
		echo -n "1 "
		r=1
	else
		echo -n "${tmp} +"
		tmp1=$(( ${tmp} - 1 ))
		mut $tmp1
		r=$(( ${tmp}+${r} ))
	fi
}

read val 
mut "$val"
echo "res : $r"
