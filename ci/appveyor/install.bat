conda config --set always_yes yes --set changeps1 no
conda config --add channels conda-forge
conda clean --packages
conda install conda-devenv
conda update -q conda
conda info -a
conda devenv
call activate thermofun
