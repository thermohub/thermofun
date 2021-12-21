conda install conda=4.9.2
conda config --set always_yes yes --set changeps1 no
conda config --add channels conda-forge
conda info -a
conda clean --packages
conda install conda-devenv
conda devenv
call activate thermofun
