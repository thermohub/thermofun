if "%APPVEYOR_BUILD_WORKER_IMAGE%"=="Visual Studio 2017" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
)
if "%APPVEYOR_BUILD_WORKER_IMAGE%"=="Visual Studio 2015" (
    call "C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd" /x64
    call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86_amd64
)

echo "Configuring..."
for /f "usebackq skip=1" %%i in (`where python`) do (
  set PREFIX=%%i
  goto :done
  )
:done
echo "%PREFIX%"
cmake -G"Visual Studio 15 2017" -DTHERMOFUN_PYTHON_INSTALL_PREFIX:PATH="%PREFIX%" -A x64 -S . -B build
echo "Building..."
cmake --build build --config %CONFIGURATION% --target install
