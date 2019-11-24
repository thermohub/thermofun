REM call build\test\%CONFIGURATION%\tests.exe
call python -c "import thermofun"
REM call pytest -ra -vv %TEST_DIR% --color=yes