conda config --system --set always_yes yes --set changeps1 no
conda config --system --append channels conda-forge
conda install -n base conda-devenv
conda update -q conda
conda info -a
conda devenv
call activate thermofun
