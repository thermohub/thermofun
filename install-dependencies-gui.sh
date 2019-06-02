#!/bin/bash
# Installing dependencies needed to build ThermoFunGUI on (k)ubuntu linux 16.04 or 18.04

#Needs gcc v.5 or higher and ArangoDB server locally installed

# installing dependencies of thermofun library
chmod u+x install-dependencies.sh
./install-dependencies.sh

sudo apt-get install -y lua5.3-dev libboost-all-dev libcurl4-openssl-dev libboost-test-dev automake flex bison libssl-dev libgl-dev

# Uncomment what is necessary to reinstall by force 
#sudo rm -f /usr/local/lib/pugixml.so
#sudo rm -f /usr/local/lib/libyaml-cpp.so
#sudo rm -f /usr/local/lib/libthrift.a
#sudo rm -f /usr/local/lib/libjsonimpex.a
sudo rm -f /usr/local/lib/libjsonui.a

threads=3
QT_PATH=$1
echo $QT_PATH
BRANCH_JSON=master
workfolder=${PWD}

# pugixml
test -f /usr/local/lib/libpugixml.so || {

	# Building yaml-cpp library
	mkdir -p ~/code && \
		cd ~/code && \
		git clone https://github.com/zeux/pugixml.git && \
		cd pugixml && \
		mkdir -p build && \
		cd build && \
		cmake .. -DCMAKE_CXX_FLAGS=-fPIC -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DCMAKE_PREFIX_PATH=$QT_PATH && \
		make -j $threads && \
		sudo make install

	# Removing generated build files
	cd ~ && \
		 rm -rf ~/code
}

# YAMLCPP
test -f /usr/local/lib/libyaml-cpp.so || {

	# Building yaml-cpp library
	mkdir -p ~/code && \
		cd ~/code && \
		git clone https://github.com/jbeder/yaml-cpp.git && \
		cd yaml-cpp && \
		mkdir -p build && \
		cd build && \
		cmake .. -DCMAKE_CXX_FLAGS=-fPIC -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DYAML_CPP_BUILD_TOOLS=OFF -DYAML_CPP_BUILD_CONTRIB=OFF -DCMAKE_PREFIX_PATH=$QT_PATH && \
		make -j $threads && \
		sudo make install

	# Removing generated build files
	cd ~ && \
		 rm -rf ~/code
}

# Thrift 
# if no Thrift installed in /usr/local/lib/libthrift.a (/usr/local/include/thrift)
test -f /usr/local/lib/libthrift.a || {

	# Building thrift library
	mkdir -p ~/code && \
		cd ~/code && \
		git clone http://github.com/apache/thrift && \
		cd thrift && \
		./bootstrap.sh && \
		./configure --without-lua --without-qt5 && \
		make -j $threads && \
		sudo make install

	# Removing generated build files
	cd ~ && \
		 rm -rf ~/code
}

## JSONIMPEX (added for building ThermoMatch code)
## if no JSONIMPEX installed in /usr/local/lib/libjsonimpex.a (/usr/local/include/jsonimpex)
test -f /usr/local/lib/libjsonimpex.a || {

	# Building jsonimpex library
	mkdir -p ~/code && \
		cd ~/code && \
		git clone https://bitbucket.org/gems4/jsonimpex.git -b $BRANCH_JSON && \
		cd jsonimpex && \
		mkdir -p build && \
		cd build && \
		cmake .. -DCMAKE_CXX_FLAGS=-fPIC -DCMAKE_BUILD_TYPE=Release && \
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
		cmake .. -DCMAKE_CXX_FLAGS=-fPIC -DCMAKE_BUILD_TYPE=Release -DJSONUI_NO_IMPEX=ON -DCMAKE_PREFIX_PATH=$QT_PATH && \
		make -j $threads && \
		sudo make install

	# Removing generated build files
	cd ~ && \
		 rm -rf ~/code
}

sudo ldconfig
