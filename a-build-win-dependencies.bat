@echo off
setlocal EnableDelayedExpansion 

set PROGFILES=%ProgramFiles%
if not "%ProgramFiles(x86)%" == "" set PROGFILES=%ProgramFiles(x86)%

REM Check if Visual Studio 2017 comunity is installed
set MSVCDIR="%PROGFILES%\Microsoft Visual Studio\2017\Community"
set VCVARSALLPATH="%PROGFILES%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat"        
if exist %MSVCDIR% (
  if exist %VCVARSALLPATH% (
   	set COMPILER_VER="2017"
        echo Using Visual Studio 2017 Community
	goto setup_env
  )
)


echo No compiler : Microsoft Visual Studio 2017 Community is not installed.
goto end

:setup_env

:begin

REM Setup path to helper bin
set ROOT_DIR="%CD%"
set FART="%CD%\bin\fart\fart.exe"
set RM="%CD%\bin\unxutils\rm.exe"
set CP="%CD%\bin\unxutils\cp.exe"
set MKDIR="%CD%\bin\unxutils\mkdir.exe"
set SEVEN_ZIP="%CD%\bin\7-zip\7za.exe"
set WGET="%CD%\bin\unxutils\wget.exe"
set XIDEL="%CD%\bin\xidel\xidel.exe"
set DEP_DIR="%ROOT_DIR%\..\dependencies"
set QT_LIB="C:\Qt\5.12.0\msvc2017_64\bin"

REM Housekeeping
del /F /S /Q tmp_*
%RM% -rf tmp_*
REM %RM% -rf dependencies
REM %RM% -rf curl.zip
%RM% -rf build_*.txt

echo
echo ******                ******
echo ****** Compiling Curl ******
echo ******                ******
echo


mkdir tmp_libcurl

cd tmp_libcurl

echo Get Curl from git...

git clone https://github.com/curl/curl.git
cd curl*
git checkout tags/curl-7_62_0

cd src

rename "tool_hugehelp.c.cvs" "tool_hugehelp.c"

cd ..

cd winbuild

if %COMPILER_VER% == "2017" (
	set VCVERSION = 14.1
	goto buildnow
)

:buildnow
REM Build!

if [%1]==[-static] (
	set RTLIBCFG=static
	echo Using /MT instead of /MD
) 

echo "%MSVCDIR%\VC\Auxiliary\Build\vcvarsall.bat"
REM call %MSVCDIR%\VC\Auxiliary\Build\vcvarsall.bat x86
REM echo Compiling dll-debug-x86 version...
REM nmake /f Makefile.vc mode=dll VC=%VCVERSION% DEBUG=yes

REM echo Compiling dll-release-x86 version...
REM nmake /f Makefile.vc mode=dll VC=%VCVERSION% DEBUG=no GEN_PDB=yes

REM echo Compiling static-debug-x86 version...
REM nmake /f Makefile.vc mode=static VC=%VCVERSION% DEBUG=yes

REM echo Compiling static-release-x86 version...
REM nmake /f Makefile.vc mode=static VC=%VCVERSION% DEBUG=no

call %MSVCDIR%\VC\Auxiliary\Build\vcvarsall.bat x64
echo Compiling dll-debug-x64 version...
nmake /f Makefile.vc mode=dll VC=%VCVERSION% DEBUG=yes MACHINE=x64

echo Compiling dll-release-x64 version...
nmake /f Makefile.vc mode=dll VC=%VCVERSION% DEBUG=no GEN_PDB=yes MACHINE=x64

REM echo Compiling static-debug-x64 version...
REM nmake /f Makefile.vc mode=static VC=%VCVERSION% DEBUG=yes MACHINE=x64
REM 
REM echo Compiling static-release-x64 version...
REM nmake /f Makefile.vc mode=static VC=%VCVERSION% DEBUG=no MACHINE=x64

REM REM Copy compiled .*lib, *.pdb, *.dll files folder to dependencies\lib\dll-debug folder
REM cd %ROOT_DIR%\tmp_libcurl\curl*\builds\libcurl-vc-x86-debug-dll-ipv6-sspi-winssl
REM %MKDIR% -p %ROOT_DIR%\dependencies\lib-dll-debug-x86
REM %CP% lib\*.pdb %ROOT_DIR%\dependencies\lib-dll-debug-x86
REM %CP% lib\*.lib %ROOT_DIR%\dependencies\lib-dll-debug-x86
REM %CP% bin\*.dll %ROOT_DIR%\dependencies\lib-dll-debug-x86
REM 
REM REM Copy compiled .*lib, *.pdb, *.dll files to dependencies\lib\dll-release folder
REM cd %ROOT_DIR%\tmp_libcurl\curl*\builds\libcurl-vc-x86-release-dll-ipv6-sspi-winssl
REM %MKDIR% -p %ROOT_DIR%\dependencies\lib-dll-release-x86
REM %CP% lib\*.pdb %ROOT_DIR%\dependencies\lib-dll-release-x86
REM %CP% lib\*.lib %ROOT_DIR%\dependencies\lib-dll-release-x86
REM %CP% bin\*.dll %ROOT_DIR%\dependencies\lib-dll-release-x86
REM 
REM REM Copy compiled .*lib file in lib-release folder to dependencies\lib\static-debug folder
REM cd %ROOT_DIR%\tmp_libcurl\curl*\builds\libcurl-vc-x86-debug-static-ipv6-sspi-winssl
REM %MKDIR% -p %ROOT_DIR%\dependencies\lib-static-debug-x86
REM %CP% lib\*.lib %ROOT_DIR%\dependencies\lib-static-debug-x86
REM 
REM REM Copy compiled .*lib files in lib-release folder to dependencies\lib\static-release folder
REM cd %ROOT_DIR%\tmp_libcurl\curl*\builds\libcurl-vc-x86-release-static-ipv6-sspi-winssl
REM %MKDIR% -p %ROOT_DIR%\dependencies\lib-static-release-x86
REM %CP% lib\*.lib %ROOT_DIR%\dependencies\lib-static-release-x86

echo Copy compiled .*lib, *.pdb, *.dll files folder to dependencies\lib\dll-debug folder
cd %ROOT_DIR%\tmp_libcurl\curl*\builds\libcurl-vc-x64-debug-dll-ipv6-sspi-winssl
%MKDIR% -p %DEP_DIR%\lib-dll-debug-x64
%CP% lib\*.pdb %DEP_DIR%\lib-dll-debug-x64
%CP% lib\*.lib %DEP_DIR%\lib-dll-debug-x64
%CP% bin\*.dll %DEP_DIR%\lib-dll-debug-x64

echo Copy compiled .*lib, *.pdb, *.dll files to dependencies\lib\dll-release folder
cd %ROOT_DIR%\tmp_libcurl\curl*\builds\libcurl-vc-x64-release-dll-ipv6-sspi-winssl
%MKDIR% -p %DEP_DIR%\lib-dll-release-x64
%CP% lib\*.pdb %DEP_DIR%\lib-dll-release-x64
%CP% lib\*.lib %DEP_DIR%\lib-dll-release-x64
%CP% bin\*.dll %DEP_DIR%\lib-dll-release-x64

echo Copy compiled .*lib file in lib-release folder to dependencies\lib\static-debug folder
cd %ROOT_DIR%\tmp_libcurl\curl*\builds\libcurl-vc-x64-debug-static-ipv6-sspi-winssl
%MKDIR% -p %DEP_DIR%\lib-static-debug-x64
%CP% lib\*.lib %DEP_DIR%\lib-static-debug-x64

echo Copy compiled .*lib files in lib-release folder to dependencies\lib\static-release folder
cd %ROOT_DIR%\tmp_libcurl\curl*\builds\libcurl-vc-x64-release-static-ipv6-sspi-winssl
%MKDIR% -p %DEP_DIR%\lib-static-release-x64
%CP% lib\*.lib %DEP_DIR%\lib-static-release-x64


echo Copy include folder to dependencies folder
%CP% -rf include %DEP_DIR%

echo Cleanup temporary file/folders
cd %ROOT_DIR%
del /F /S /Q tmp_libcurl
%RM% -rf tmp_libcurl
REM %RM% -rf tmp_*


echo
echo ******                		 ******
echo ****** Compiling Velocypack ******
echo ******                		 ******
echo

mkdir tmp_velo
cd tmp_velo

echo Get velocypack from git...

git clone https://github.com/arangodb/velocypack.git

echo Setting velocypack build examples and tests OFF
%FART% -w "%ROOT_DIR%\tmp_velo\velocypack\CMakeLists.txt" ON OFF

echo Setting linker settings from /MT to /MD
%FART% "%ROOT_DIR%\tmp_velo\velocypack\cmake\Modules\AR_CompilerSettings.cmake" MTd MDd
%FART% "%ROOT_DIR%\tmp_velo\velocypack\cmake\Modules\AR_CompilerSettings.cmake" MT MD

cd velo*
mkdir build 
cd build

echo "%MSVCDIR%\VC\Auxiliary\Build\vcvarsall.bat"
call %MSVCDIR%\VC\Auxiliary\Build\vcvarsall.bat x64
echo Compiling lib-release-x64 version...
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DBuildTools=OFF -DBuildExamples=OFF -DBuildTests=OFF ..
nmake /f Makefile

cd %ROOT_DIR%\tmp_velo\velo*\build\
%MKDIR% -p %DEP_DIR%\lib-static-release-x64
REM %CP% *.lib %ROOT_DIR%\dependencies\lib-static-release-x64
%CP% *.lib %DEP_DIR%\lib-static-release-x64

cd ..
del /F /S /Q build
cd build 

echo Compiling lib-debug-x64 version...
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=debug -DBuildTools=OFF -DBuildExamples=OFF -DBuildTests=OFF ..
nmake /f Makefile

cd %ROOT_DIR%\tmp_velo\velo*\build\
%MKDIR% -p %DEP_DIR%\lib-static-debug-x64
REM %CP% *.lib %ROOT_DIR%\dependencies\lib-static-debug-x64
%CP% *.lib %DEP_DIR%\lib-static-debug-x64

echo Copy include folder to dependencies folder
cd ..
%CP% -rf include %DEP_DIR%

echo Cleanup temporary file/folders
cd %ROOT_DIR%
del /F /S /Q tmp_velo

%RM% -rf tmp_velo

%MKDIR% -p %ROOT_DIR%\..\build-fun-gui\debug
%MKDIR% -p %ROOT_DIR%\..\build-fun-gui\release

%CP% -rf %ROOT_DIR%\Resources %ROOT_DIR%\..\build-fun-gui\debug
%CP% -rf %ROOT_DIR%\Resources %ROOT_DIR%\..\build-fun-gui\release

%CP% %DEP_DIR%\lib-dll-release-x64\libcurl*.dll %ROOT_DIR%\..\build-fun-gui\release
%CP% %DEP_DIR%\lib-dll-debug-x64\libcurl*.dll %ROOT_DIR%\..\build-fun-gui\debug

echo copy Qt dll files to build-fun-gui\

%CP% %QT_LIB%\Qt5Charts.dll %ROOT_DIR%\..\build-fun-gui\release
%CP% %QT_LIB%\Qt5Core.dll %ROOT_DIR%\..\build-fun-gui\release
%CP% %QT_LIB%\Qt5Gui.dll %ROOT_DIR%\..\build-fun-gui\release
%CP% %QT_LIB%\Qt5Network.dll %ROOT_DIR%\..\build-fun-gui\release
%CP% %QT_LIB%\Qt5Positioning.dll %ROOT_DIR%\..\build-fun-gui\release
%CP% %QT_LIB%\Qt5PrintSupport.dll %ROOT_DIR%\..\build-fun-gui\release
%CP% %QT_LIB%\Qt5Qml.dll %ROOT_DIR%\..\build-fun-gui\release
%CP% %QT_LIB%\Qt5Quick.dll %ROOT_DIR%\..\build-fun-gui\release
%CP% %QT_LIB%\Qt5QuickWidgets.dll %ROOT_DIR%\..\build-fun-gui\release
%CP% %QT_LIB%\Qt5WebChannel.dll %ROOT_DIR%\..\build-fun-gui\release
%CP% %QT_LIB%\Qt5WebEngineCore.dll %ROOT_DIR%\..\build-fun-gui\release
%CP% %QT_LIB%\Qt5WebEngineWidgets.dll %ROOT_DIR%\..\build-fun-gui\release
%CP% %QT_LIB%\Qt5Widgets.dll %ROOT_DIR%\..\build-fun-gui\release

%CP% %QT_LIB%\Qt5Charts.dll %ROOT_DIR%\..\build-fun-gui\debug
%CP% %QT_LIB%\Qt5Core.dll %ROOT_DIR%\..\build-fun-gui\debug
%CP% %QT_LIB%\Qt5Gui.dll %ROOT_DIR%\..\build-fun-gui\debug
%CP% %QT_LIB%\Qt5Network.dll %ROOT_DIR%\..\build-fun-gui\debug
%CP% %QT_LIB%\Qt5Positioning.dll %ROOT_DIR%\..\build-fun-gui\debug
%CP% %QT_LIB%\Qt5PrintSupport.dll %ROOT_DIR%\..\build-fun-gui\debug
%CP% %QT_LIB%\Qt5Qml.dll %ROOT_DIR%\..\build-fun-gui\debug
%CP% %QT_LIB%\Qt5Quick.dll %ROOT_DIR%\..\build-fun-gui\debug
%CP% %QT_LIB%\Qt5QuickWidgets.dll %ROOT_DIR%\..\build-fun-gui\debug
%CP% %QT_LIB%\Qt5WebChannel.dll %ROOT_DIR%\..\build-fun-gui\debug
%CP% %QT_LIB%\Qt5WebEngineCore.dll %ROOT_DIR%\..\build-fun-gui\debug
%CP% %QT_LIB%\Qt5WebEngineWidgets.dll %ROOT_DIR%\..\build-fun-gui\debug
%CP% %QT_LIB%\Qt5Widgets.dll %ROOT_DIR%\..\build-fun-gui\debug

echo Finished a-build-win-dependencies.bat scrip! 

:end
exit /b
