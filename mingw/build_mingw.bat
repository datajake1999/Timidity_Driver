@ECHO OFF
g++ -DUNICODE -D_UNICODE -I..\driver\include -O3 ..\common\*.c ..\driver\src\*.cpp ..\driver\timidity\*.c -s -static -shared -Wl,--add-stdcall-alias -ladvapi32 -lwinmm -o timidity.dll
gcc -DUNICODE -D_UNICODE -O3 ..\installer\drvinst.c -s -static -ladvapi32 -o drvsetup.exe
gcc -DUNICODE -D_UNICODE -O3 ..\test\test.c -s -static -lwinmm -o drvtest.exe
windres ..\config\Timidity.rc resource.o
gcc -c -DUNICODE -D_UNICODE -O3 ..\config\*.c ..\common\*.c
gcc dialog.o registry.o winmain.o resource.o -s -static -Wl,-subsystem,windows -ladvapi32 -lcomctl32 -lcomdlg32 -o timcfg.exe
gcc cpl.o dialog.o registry.o resource.o -s -static -shared -Wl,--add-stdcall-alias -ladvapi32 -lcomctl32 -lcomdlg32 -o timcfg.cpl
del *.o
gcc -DUNICODE -D_UNICODE -O3 ..\common\*.c ..\driver\timidity\*.c ..\mid2wav\*.c -s -static -ladvapi32 -o mid2wav.exe
copy ..\installer\install.bat
copy ..\installer\uninstall.bat
