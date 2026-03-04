REM Meson RELEASE: Build a 'Release' build directory
cd..
meson -Dwrap_mode=forcefallback AQtion\Release --buildtype=debugoptimized
pause