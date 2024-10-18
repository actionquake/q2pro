REM Meson DEBUG: clean, build, and copy files
cd..
meson compile -C AQtion\Debug --clean
meson compile -C AQtion\Debug
xcopy /y AQtion\Debug\q2pro.exe AQtion\Debug\..\AQtion\
xcopy /y AQtion\Debug\q2proded.exe AQtion\Debug\..\AQtion\
xcopy /y AQtion\Debug\gamex86_64.dll AQtion\Debug\..\AQtion\action\
pause