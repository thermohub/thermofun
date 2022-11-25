if [ ! -f $HOME/miniconda/bin/conda ]; then
    echo "Downloading and installing miniconda"
    wget -O miniconda.sh https://repo.continuum.io/miniconda/Miniconda3-latest-MacOSX-x86_64.sh
    rm -rf $HOME/miniconda
    bash miniconda.sh -b -p $HOME/miniconda
fi
if [ ! -f $HOME/miniconda/bin/conda ]; then
    echo ERROR: conda was not installed.
    exit 1
fi
bash $HOME/miniconda/etc/profile.d/conda.sh
export PATH=$HOME/miniconda/bin/:$PATH
conda config --set always_yes yes --set changeps1 no
conda config --add channels conda-forge
conda install conda-devenv
conda update -q conda
conda info -a
conda devenv
source activate thermofun
mkdir build
cd build
# Configure step
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_LIBDIR=lib \
    ..
make install
conda list
python -c "import thermofun"
cd ..
pytest -ra -vv --color=yes .
if [ $? -eq 1 ]
then
echo "The install failed" >&2
exit 1
fi
