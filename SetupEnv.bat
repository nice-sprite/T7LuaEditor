:: configure the build environment
@echo off
Call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
:: add ourselves to the PATH 
set path=w:\repos\Priscilla;%path%
