#!/bin/bash
appname=ThermoFunGuiDemo
dirname=`dirname $0`
if [ "${dirname:0:1}" != "/" ]; then
   dirname=$PWD/$dirname
fi
export LD_LIBRARY_PATH=/usr/local/lib
cd $dirname
./$appname $*
