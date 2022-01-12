rem Copyright (C) 2022 dmiron
rem 
rem This file is part of thermofun.
rem 
rem thermofun is free software: you can redistribute it and/or modify
rem it under the terms of the GNU General Public License as published by
rem the Free Software Foundation, either version 3 of the License, or
rem (at your option) any later version.
rem 
rem thermofun is distributed in the hope that it will be useful,
rem but WITHOUT ANY WARRANTY; without even the implied warranty of
rem MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
rem GNU General Public License for more details.
rem 
rem You should have received a copy of the GNU General Public License
rem along with thermofun.  If not, see <http://www.gnu.org/licenses/>.

call activate thermofun

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"

echo "Configuring..."
cmake -G"Visual Studio 16 2019" -DTHERMOFUN_PYTHON_INSTALL_PREFIX:PATH="%CONDA_PREFIX%" -A x64 -S . -B build
echo "Building..."
cmake --build build --target install