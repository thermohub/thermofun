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

mkdir tmp_libboost
cd tmp_libboost

REM Get download url .Look under <blockquote><a type='application/zip' href='xxx'>
echo Get download url...
%XIDEL% https://www.boost.org/users/download/ -e "//a[ends-with(@href, '.zip')]/@href" > tmp_url
set /p url=<tmp_url

REM exit on errors, else continue
if %errorlevel% neq 0 exit /b %errorlevel%

REM Download latest boost and rename to boost.zip
echo Downloading latest boost...
%WGET% "%url%" -O boost.zip

REM Extract downloaded zip file to boost
%SEVEN_ZIP% x boost.zip -y

cd boost_*

REM Copy include folder to dependencies folder
%CP% -rf boost %DEP_DIR%\include

echo Compiling boost...
echo "%MSVCDIR%\VC\Auxiliary\Build\vcvarsall.bat"
call %MSVCDIR%\VC\Auxiliary\Build\vcvarsall.bat x64

call bootstrap.bat

b2.exe --build-type=complete architecture=x86 address-model=64 --libdir="..\dependencies\boost" --includedir="..\dependencies\boost" install -j3 --with-regex --with-filesystem --with-system

cd ..\dependencies\boost
%MKDIR% -p %DEP_DIR%\lib-static-release-x64
REM %CP% *.lib %DEP_DIR%\lib-static-release-x64
%CP% lib*mt-x64*.lib %DEP_DIR%\lib-static-release-x64

%MKDIR% -p %DEP_DIR%\lib-static-debug-x64
REM %CP% *.lib %DEP_DIR%\lib-static-debug-x64
%CP% lib*mt-gd*.lib %DEP_DIR%\lib-static-debug-x64

Setlocal enabledelayedexpansion
For %%# in ("%DEP_DIR%\lib-static-release-x64\libboost*.lib") Do (
    Set "File=%%~nx#"
    Ren "%%#" "!File:%Pattern%=%Replace%!"
)

For %%# in ("%DEP_DIR%\lib-static-debug-x64\libboost*.lib") Do (
    Set "File=%%~nx#"
    Ren "%%#" "!File:%Pattern%=%Replace%!"
)

REM Cleanup temporary file/folders
REM cd %ROOT_DIR%
REM del /F /S /Q tmp_*
REM %RM% -rf tmp_*