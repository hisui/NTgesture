@echo off

set CC=cl /EHsc /W3 /Zi -IC:\Devel\lib\npapi -DWIN32=1 -DXP_WIN=1 -D_WINDOWS=1
set IMPORTS=user32.lib gdi32.lib psapi.lib msimg32.lib
set LINKER=link /DLL /PDB:vc90.pdb /DEBUG

%CC% -c ntghk.cpp
%LINKER% /DEF:ntghk.def /OUT:ntghk.dll ntghk.obj %IMPORTS%

%CC% -c ntg32.cpp
%CC% -c npp.cpp
%CC% -c daemon.cpp
rc ntg.rc
%LINKER% /DLL /DEF:ntg32.def /OUT:ntg32.dll ntg32.obj npp.obj daemon.obj ntg.res %IMPORTS% ntghk.lib
