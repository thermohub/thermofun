#!/bin/bash
# Installing dependencies needed to build ThermoFun on (k)ubuntu linux 16.04 or 18.04

EXTN=so

#sudo rm -f /usr/local/include/nlohmann/json.hpp
#sudo rm -rf /usr/local/include/eigen3/Eigen
#sudo rm -rf /usr/local/include/pybind11
#sudo rm -rf /usr/local/include/spdlog
#sudo rm -f /usr/local/lib/libChemicalFun.$EXTN

workfolder=${PWD}
BRANCH_TFUN=master
BUILD_TYPE=Debug

# nlohmann/json
test -f /usr/local/include/nlohmann/json.hpp || {

	# Building json library
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

# spdlog 
# if no spdlog installed in /usr/local/lib/ (/usr/local/include/spdlog)
test -d /usr/local/include/spdlog || {

        # Building thermofun library
        mkdir -p ~/code && \
                cd ~/code && \
                git clone https://github.com/gabime/spdlog -b v1.10.0  && \
                cd spdlog && \
                mkdir -p build && \
                cd build && \
                cmake .. \
                make && \
                sudo make install

        # Removing generated build files
        cd ~ && \
                 rm -rf ~/code
}

#Pybind11
test -d /usr/local/include/pybind11 || {

        # Building pybind11 library
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

# Eigen
# Eigen3 math library (added for building and installing xGEMS)
# if not installed in /usr/local/include/eigen3)
test -d /usr/local/include/eigen3/Eigen || {

        # Building eigen library
        mkdir -p ~/code && \
                cd ~/code && \
                git clone https://gitlab.com/libeigen/eigen.git -b 3.4.0 && \
                cd eigen && \
                mkdir -p build && \
                cd build && \
                cmake .. \
                make && \
                sudo make install

        # Removing generated build files
        cd ~ && \
                 rm -rf ~/code
}



# ChemicalFun library
# if no ChemicalFun installed in /usr/local/lib/ (/usr/local/include/ChemicalFun)
test -f /usr/local/lib/libChemicalFun.$EXTN || {

        # Building chemicalfun library
        mkdir -p ~/code && \
                cd ~/code && \
                git clone https://bitbucket.org/gems4/chemicalfun.git -b $BRANCH_TFUN  && \
                cd chemicalfun && \
                mkdir -p build && \
                cd build && \
                        cmake .. -DCMAKE_CXX_FLAGS=-fPIC -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DTFUN_BUILD_PYTHON=OFF && \
        make -j $threads && \
                sudo make install

        # Removing generated build files
        cd ~ && \
                 rm -rf ~/code
}

if [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
   sudo ldconfig
fi
