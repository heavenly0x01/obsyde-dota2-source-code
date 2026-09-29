@echo off
setlocal
pushd "%~dp0"
if not exist build mkdir build

cl /nologo /std:c++17 /EHsc /O2 /LD /Isrc /Ivendor\imgui ^
  src\main.cpp src\game.cpp src\rtti.cpp src\hook.cpp src\draw.cpp src\menu.cpp ^
  vendor\imgui\imgui.cpp vendor\imgui\imgui_draw.cpp vendor\imgui\imgui_tables.cpp vendor\imgui\imgui_widgets.cpp ^
  vendor\imgui\imgui_impl_win32.cpp vendor\imgui\imgui_impl_dx11.cpp ^
  /link /OUT:build\obsdota2.dll

set "RESULT=%ERRORLEVEL%"
popd
exit /b %RESULT%
