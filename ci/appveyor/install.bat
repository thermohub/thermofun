conda config --set always_yes yes --set changeps1 no
conda update conda --yes
conda config --add channels conda-forge
conda info -a
conda install conda-devenv
conda devenv
call activate thermofun
