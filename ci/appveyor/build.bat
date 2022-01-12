if "%APPVEYOR_BUILD_WORKER_IMAGE%"=="Visual Studio 2019" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
)
if "%APPVEYOR_BUILD_WORKER_IMAGE%"=="Visual Studio 2015" (
    call "C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd" /x64
    call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86_amd64
)

echo "Configuring..."
cmake -G"Visual Studio 16 2019" -DTHERMOFUN_PYTHON_INSTALL_PREFIX:PATH="C:/Miniconda36-x64/envs/thermofun/" -A x64 -S . -B build
echo "Building..."
cmake --build build --config %CONFIGURATION% --target install
