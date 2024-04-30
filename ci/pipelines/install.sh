if [ ! -f $HOME/miniconda/bin/conda ]; then
    echo "Downloading and installing miniconda"
    alias wgetncc="wget --no-check-certificate"
    wgetncc -O miniconda.sh https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh
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
conda config --set ssl_verify false || exit 1
conda info -a
conda devenv
source activate thermofun
mkdir build
cd build
python_path=$(which python)
# Configure step
cmake -GNinja \
    -DPYTHON_EXECUTABLE:FILEPATH=$python_path \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_LIBDIR=lib \
    ..
ninja install
if [ $? -eq 1 ]
then
echo "The install failed" >&2
exit 1
fi
conda list
cd ..
pytest -ra -vv --color=yes .
