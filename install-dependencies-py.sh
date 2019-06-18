#!/bin/bash
# Installing dependencies needed to build ThermoFun python interface on (k)ubuntu linux 16.04 or 18.04

#sudo rm -f /usr/local/include/pybind11/pybind11.h

workfolder=${PWD}

# pybind11
test -f /usr/local/include/pybind11/pybind11.h || {

	# Building yaml-cpp library
	mkdir -p ~/code && \
		cd ~/code && \
		git clone https://github.com/pybind/pybind11.git && \
		cd pybind11 && \
		mkdir -p build && \
		cd build && \
		cmake .. -DPYBIND11_TEST=OFF && \
		make && \
		sudo make install

	# Removing generated build files
	cd ~ && \
		 rm -rf ~/code
}

sudo ldconfig
