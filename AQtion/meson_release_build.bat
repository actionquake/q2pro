REM Meson RELEASE: build and copy files
cd..
meson compile -C AQtion\Release
xcopy /y AQtion\Release\q2pro.exe AQtion\Release\..\AQtion\
xcopy /y AQtion\Release\q2proded.exe AQtion\Release\..\AQtion\
xcopy /y AQtion\Release\gamex86_64.dll AQtion\Release\..\AQtion\action\
pause