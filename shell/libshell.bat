@echo off
IF NOT EXIST ..\include\shell mkdir ..\include\shell

copy source\include\sh_enet.h ..\include\shell\sh_enet.h /Y
copy source\include\sh_mfs.h ..\include\shell\sh_mfs.h /Y
copy source\include\sh_rtcs.h ..\include\shell\sh_rtcs.h /Y
copy source\include\shell.h ..\include\shell\shell.h /Y
