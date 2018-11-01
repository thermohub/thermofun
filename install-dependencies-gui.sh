#!/bin/bash
# Installing dependencies needed to build ThermoFunGUI on (k)ubuntu linux 16.04 or 18.04

#Needs gcc v.5 or higher and ArangoDB server locally installed

# installing dependencies of thermofun library
chmod u+x install-dependencies.sh
./install-dependencies.sh

# Uncomment what is necessary to reinstall by force 
#sudo rm -f /usr/local/lib/libthrift.a
#sudo rm -f /usr/local/lib/libjsonui.a

threads=3
QT_PATH=/home/dmiron/Qt/5.11.0/gcc_64

# Thrift 
# if no Thrift installed in /usr/local/lib/libthrift.a (/usr/local/include/thrift)
test -f /usr/local/lib/libthrift.a || {

	# Building thrift library
	mkdir -p ~/code && \
		cd ~/code && \
		git clone http://github.com/apache/thrift && \
		cd thrift && \
		./bootstrap.sh && \
		./configure --without-lua && \
		make -j $threads && \
		sudo make install

	# Removing generated build files
	cd ~ && \
		 rm -rf ~/code
}

# JSONUI database client (added for building ThermoMatch code)
# if no JSONUI installed in /usr/local/lib/libjsonui.a (/usr/local/include/jsonui)
test -f /usr/local/lib/libjsonui.a || {

	# Building jsonui library
	mkdir -p ~/code && \
		cd ~/code && \
		git clone https://bitbucket.org/gems4/jsonui.git && \
		cd jsonui && \
		mkdir -p build && \
		cd build && \
		cmake .. -DCMAKE_CXX_FLAGS=-fPIC -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=$QT_PATH && \
		make -j $threads && \
		sudo make install

	# Removing generated build files
	cd ~ && \
		 rm -rf ~/code
}

sudo ldconfig