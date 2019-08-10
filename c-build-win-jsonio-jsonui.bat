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

REM Check if Visual Studio 2019 comunity is installed
set MSVCDIR="%PROGFILES%\Microsoft Visual Studio\2019\Community"
set VCVARSALLPATH="%PROGFILES%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"        
if exist %MSVCDIR% (
  if exist %VCVARSALLPATH% (
   	set COMPILER_VER="2019"
        echo Using Visual Studio 2019 Community
	goto setup_env
  )
)


echo No compiler : Microsoft Visual Studio 2017 or 2019 Community is not installed.
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
set QT_LIB=%1
set JOM="C:\Qt\Tools\QtCreator\bin\jom.exe"

cd ..

echo
echo ******                  ******
echo ****** Compiling JSONIO ******
echo ******                  ******
echo

echo git clone jsonio...
git clone https://bitbucket.org/gems4/jsonio.git
cd jsonio
git checkout build-windows
%CP% -rf include %DEP_DIR%
cd ..

%MKDIR% -p build-jsonio/release
%MKDIR% -p build-jsonio/debug

cd build-jsonio/release

echo "%MSVCDIR%\VC\Auxiliary\Build\vcvarsall.bat"
call %MSVCDIR%\VC\Auxiliary\Build\vcvarsall.bat x64

if exist %QT_LIB% ( goto :build_jsonio)

echo %QT_LIB% not found
goto end

:build_jsonio

echo building jsonio
%QT_LIB%\qmake.exe %ROOT_DIR%\..\jsonio\jsonio-lib.pro -spec win32-msvc && %JOM% qmake_all 

%JOM%

cd release

%CP% *.lib %DEP_DIR%\lib-static-release-x64

cd ..\..
cd debug

%QT_LIB%\qmake.exe %ROOT_DIR%\..\jsonio\jsonio-lib.pro -spec win32-msvc "CONFIG+=debug" && %JOM% qmake_all

%JOM% -j4

cd debug

%CP% *.lib %DEP_DIR%\lib-static-debug-x64

cd %ROOT_DIR%\..

REM JSONUI

echo
echo ******                  ******
echo ****** Compiling JSONUI ******
echo ******                  ******
echo

echo git clone jsonui..
git clone https://bitbucket.org/gems4/jsonui.git
cd jsonui
git checkout build-windows
%CP% -rf include %DEP_DIR%
cd ..

%MKDIR% -p build-jsonui/release
%MKDIR% -p build-jsonui/debug

cd build-jsonui/release

echo "%MSVCDIR%\VC\Auxiliary\Build\vcvarsall.bat"
call %MSVCDIR%\VC\Auxiliary\Build\vcvarsall.bat x64

%QT_LIB%\qmake.exe %ROOT_DIR%\..\jsonui\jsonui-lib.pro -spec win32-msvc && C:/Qt/Tools/QtCreator/bin/jom.exe qmake_all

%JOM% -j2

cd release

%CP% *.lib %DEP_DIR%\lib-static-release-x64

cd ..\..
cd debug

%QT_LIB%\qmake.exe %ROOT_DIR%\..\jsonui\jsonui-lib.pro -spec win32-msvc "CONFIG+=debug" && C:/Qt/Tools/QtCreator/bin/jom.exe qmake_all

%JOM% -j2

cd debug

%CP% *.lib %DEP_DIR%\lib-static-debug-x64


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

REM %MKDIR% -p %DEP_DIR%\lib-static-release-x64
REM REM %CP% *.lib %DEP_DIR%\lib-static-release-x64
REM %CP% lib*mt-x64*.lib %DEP_DIR%\lib-static-release-x64
REM 
REM %MKDIR% -p %DEP_DIR%\lib-static-debug-x64
REM REM %CP% *.lib %DEP_DIR%\lib-static-debug-x64
REM %CP% lib*mt-gd*.lib %DEP_DIR%\lib-static-debug-x64
REM 
REM REM Cleanup temporary file/folders
REM cd %ROOT_DIR%
REM del /F /S /Q tmp_*
REM %RM% -rf tmp_*