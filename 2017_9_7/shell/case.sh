#!/bin/bash

read val
case $val in 
	a )
	echo "enter val is a"
	;;
	b )
	echo "enter val is b"
	;;
	c )
	echo "enter val is c"
	;;
	* )
	echo "enter val is *"
	;;
esac
