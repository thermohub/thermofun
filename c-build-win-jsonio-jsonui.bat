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
set QMAKE="C:\Qt\5.12.0\msvc2017_64\bin\qmake.exe"
set JOM="C:/Qt/Tools/QtCreator/bin/jom.exe"

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

%QMAKE% %ROOT_DIR%\..\jsonio\jsonio-lib.pro -spec win32-msvc && C:/Qt/Tools/QtCreator/bin/jom.exe qmake_all

%JOM% -j4

cd release

%CP% *.lib %DEP_DIR%\lib-static-release-x64

cd ..\..
cd debug

%QMAKE% %ROOT_DIR%\..\jsonio\jsonio-lib.pro -spec win32-msvc "CONFIG+=debug" && C:/Qt/Tools/QtCreator/bin/jom.exe qmake_all

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

%QMAKE% %ROOT_DIR%\..\jsonui\jsonui-lib.pro -spec win32-msvc && C:/Qt/Tools/QtCreator/bin/jom.exe qmake_all

%JOM% -j2

cd release

%CP% *.lib %DEP_DIR%\lib-static-release-x64

cd ..\..
cd debug

%QMAKE% %ROOT_DIR%\..\jsonui\jsonui-lib.pro -spec win32-msvc "CONFIG+=debug" && C:/Qt/Tools/QtCreator/bin/jom.exe qmake_all

%JOM% -j2

cd debug

%CP% *.lib %DEP_DIR%\lib-static-debug-x64

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