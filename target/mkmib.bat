set PROJDIR=.

%PROJDIR%\..\tools\gawk.exe -f %PROJDIR%\..\tools\snmp\def2c.awk %PROJDIR%\config\fte_mib.def > %PROJDIR%\source\net\snmp\fte_mib.c
%PROJDIR%\..\tools\gawk.exe -f %PROJDIR%\..\tools\snmp\def2mib.awk %PROJDIR%\config\fte_mib.def > %PROJDIR%\source\net\snmp\fte.mib

pause
:end
