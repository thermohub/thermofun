#!/bin/bash
# Installing dependencies needed to build thermofun on (k)ubuntu linux 16.04 or 18.04

#Needs gcc v.5 or higher and ArangoDB server locally installed
sudo apt-get install -y libboost-all-dev libcurl4-openssl-dev

# Uncomment what is necessary to reinstall by force 
#sudo rm -f /usr/local/lib/libvelocypack.a
#sudo rm -f /usr/local/lib/libjsonio.a

threads=3
#QT_PATH=/home/dmiron/Qt/5.11.0/gcc_64
BRANCH_JSON=master
git status

# Velocypack from ArangoDB (added for installing JSONIO database client)
# if no VPack installed in /usr/local/lib/libvelocypack.a (/usr/local/include/velocypack)
test -f /usr/local/lib/libvelocypack.a || {

	# Building velocypack library
	mkdir -p ~/code && \
		cd ~/code && \
		git clone https://github.com/arangodb/velocypack.git && \
		cd velocypack && \
		git checkout ce7c5fec3ec54c46ff4adbadfd4519540895e096 && \
		mkdir -p build && \
		cd build && \
		cmake .. -DCMAKE_CXX_FLAGS=-fPIC -DBuildVelocyPackExamples=OFF && \
		make -j $threads && \
		sudo make install

	# Removing generated build files
	cd ~ && \
		 rm -rf ~/code
}

# JSONIO database client (added for building ThermoMatch code)
# if no JSONIO installed in /usr/local/lib/libjsonio.a (/usr/local/include/jsonio)
test -f /usr/local/lib/libjsonio.a || {

	# Building jsonio library
	mkdir -p ~/code && \
		cd ~/code && \
		git clone https://bitbucket.org/gems4/jsonio.git -b $BRANCH_JSON && \
		cd jsonio && \
		mkdir -p build && \
		cd build && \
		cmake .. -DCMAKE_CXX_FLAGS=-fPIC -DCMAKE_BUILD_TYPE=Release && \
		make -j $threads && \
		sudo make install

	# Removing generated build files
	cd ~ && \
		 rm -rf ~/code
}
sudo ldconfig 

