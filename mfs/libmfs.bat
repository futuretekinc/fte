@echo off
IF NOT EXIST ..\include mkdir ..\include
IF NOT EXIST ..\include\mfs mkdir ..\include\mfs

copy source\include\mfs.h ..\include\mfs\mfs.h /Y
copy source\include\mfs_cnfg.h ..\include\mfs\mfs_cnfg.h /Y
copy source\include\mfs_rev.h ..\include\mfs\mfs_rev.h /Y
copy source\include\part_mgr.h ..\include\mfs\part_mgr.h /Y

