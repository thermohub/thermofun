#!/bin/bash
# Installing dependencies needed to build ThermoFun on (k)ubuntu linux 16.04 or 18.04

#sudo rm -f /usr/local/include/nlohmann/json.hpp

workfolder=${PWD}

# nlohmann/json
test -f /usr/local/include/nlohmann/json.hpp || {

	# Building yaml-cpp library
	mkdir -p ~/code && \
		cd ~/code && \
		git clone https://github.com/nlohmann/json.git && \
		cd json && \
		mkdir -p build && \
		cd build && \
		cmake .. -DJSON_BuildTests=OFF && \
		make && \
		sudo make install

	# Removing generated build files
	cd ~ && \
		 rm -rf ~/code
}

sudo ldconfig
