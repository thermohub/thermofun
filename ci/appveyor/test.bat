REM call build\test\%CONFIGURATION%\tests.exe
call activate thermofun
call pytest -ra -vv %TEST_DIR% --color=yes