@echo off

set VARS=/Od /Zi /W4 /wd4130 /wd4996

set files=..\code\sv_markdown_test.c

set options=%VARS% %files%

mkdir ..\build

pushd ..\build

cl %options%

popd
