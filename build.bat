@ECHO OFF
call "C:\Program Files\Microsoft Visual Studio 8\VC\bin\vcvars32.bat"
call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /SRV32 /RETAIL
md X86
cl /nologo -c /D "UNICODE" /D "_UNICODE" /I "driver\include" /O2 common\*.c driver\src\*.cpp driver\timidity\*.c
link *.obj advapi32.lib bufferoverflowU.lib kernel32.lib user32.lib winmm.lib /nologo /dll /def:"driver\src\winmm_drv.def" /OUT:X86\timidity.dll
del *.obj
cl /nologo -c /D "UNICODE" /D "_UNICODE" /O2 installer\drvinst.c
link *.obj advapi32.lib bufferoverflowU.lib kernel32.lib user32.lib /nologo /OUT:X86\drvsetup.exe
del *.obj
cl /nologo -c /D "UNICODE" /D "_UNICODE" /O2 test\test.c
link *.obj bufferoverflowU.lib kernel32.lib user32.lib winmm.lib /nologo /OUT:X86\test.exe
del *.obj
rc config\Timidity.rc
cl /nologo -c /D "UNICODE" /D "_UNICODE" /O2 config\*.c common\*.c
link dialog.obj registry.obj winmain.obj config\Timidity.res advapi32.lib bufferoverflowU.lib comctl32.lib comdlg32.lib kernel32.lib user32.lib /nologo /OUT:X86\timcfg.exe
link cpl.obj dialog.obj registry.obj config\Timidity.res advapi32.lib bufferoverflowU.lib comctl32.lib comdlg32.lib kernel32.lib user32.lib /nologo /dll /def:"config\cpl.def" /OUT:X86\timcfg.cpl
del *.obj
cl /nologo -c /D "UNICODE" /D "_UNICODE" /O2 common\*.c driver\timidity\*.c mid2wav\*.c
link *.obj advapi32.lib bufferoverflowU.lib kernel32.lib user32.lib /nologo /OUT:X86\mid2wav.exe
del *.obj
copy installer\install.bat X86
copy installer\uninstall.bat X86
cd X86
del *.exp *.lib
cd..
call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /X64 /RETAIL
md X64
cl /nologo -c /D "UNICODE" /D "_UNICODE" /I "driver\include" /O2 common\*.c driver\src\*.cpp driver\timidity\*.c
link *.obj advapi32.lib bufferoverflowU.lib kernel32.lib user32.lib winmm.lib /nologo /dll /def:"driver\src\winmm_drv.def" /OUT:X64\timidity.dll
del *.obj
cl /nologo -c /D "UNICODE" /D "_UNICODE" /O2 installer\drvinst.c
link *.obj advapi32.lib bufferoverflowU.lib kernel32.lib user32.lib /nologo /OUT:X64\drvsetup.exe
del *.obj
cl /nologo -c /D "UNICODE" /D "_UNICODE" /O2 test\test.c
link *.obj bufferoverflowU.lib kernel32.lib user32.lib winmm.lib /nologo /OUT:X64\test.exe
del *.obj
cl /nologo -c /D "UNICODE" /D "_UNICODE" /O2 config\*.c common\*.c
link dialog.obj registry.obj winmain.obj config\Timidity.res advapi32.lib bufferoverflowU.lib comctl32.lib comdlg32.lib kernel32.lib user32.lib /nologo /OUT:X64\timcfg.exe
link cpl.obj dialog.obj registry.obj config\Timidity.res advapi32.lib bufferoverflowU.lib comctl32.lib comdlg32.lib kernel32.lib user32.lib /nologo /dll /def:"config\cpl.def" /OUT:X64\timcfg.cpl
del *.obj
cl /nologo -c /D "UNICODE" /D "_UNICODE" /O2 common\*.c driver\timidity\*.c mid2wav\*.c
link *.obj advapi32.lib bufferoverflowU.lib kernel32.lib user32.lib /nologo /OUT:X64\mid2wav.exe
del *.obj
copy installer\install.bat X64
copy installer\uninstall.bat X64
cd X64
del *.exp *.lib
cd..
call "C:\Program Files\Microsoft Platform SDK\SetEnv.Cmd" /SRV64 /RETAIL
md IA64
cl /nologo -c /D "UNICODE" /D "_UNICODE" /I "driver\include" /O2 common\*.c driver\src\*.cpp driver\timidity\*.c
link *.obj advapi32.lib bufferoverflowU.lib kernel32.lib user32.lib winmm.lib /nologo /dll /def:"driver\src\winmm_drv.def" /OUT:IA64\timidity.dll
del *.obj
cl /nologo -c /D "UNICODE" /D "_UNICODE" /O2 installer\drvinst.c
link *.obj advapi32.lib bufferoverflowU.lib kernel32.lib user32.lib /nologo /OUT:IA64\drvsetup.exe
del *.obj
cl /nologo -c /D "UNICODE" /D "_UNICODE" /O2 test\test.c
link *.obj bufferoverflowU.lib kernel32.lib user32.lib winmm.lib /nologo /OUT:IA64\test.exe
del *.obj
cl /nologo -c /D "UNICODE" /D "_UNICODE" /O2 config\*.c common\*.c
link dialog.obj registry.obj winmain.obj config\Timidity.res advapi32.lib bufferoverflowU.lib comctl32.lib comdlg32.lib kernel32.lib user32.lib /nologo /OUT:IA64\timcfg.exe
link cpl.obj dialog.obj registry.obj config\Timidity.res advapi32.lib bufferoverflowU.lib comctl32.lib comdlg32.lib kernel32.lib user32.lib /nologo /dll /def:"config\cpl.def" /OUT:IA64\timcfg.cpl
del *.obj
cl /nologo -c /D "UNICODE" /D "_UNICODE" /O2 common\*.c driver\timidity\*.c mid2wav\*.c
link *.obj advapi32.lib bufferoverflowU.lib kernel32.lib user32.lib /nologo /OUT:IA64\mid2wav.exe
del *.obj
copy installer\install.bat IA64
copy installer\uninstall.bat IA64
cd IA64
del *.exp *.lib
cd..
del config\Timidity.res
