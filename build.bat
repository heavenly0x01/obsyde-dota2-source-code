@echo off
cd /d "%~dp0"
set VCVARS=
for %%V in (18 17 16) do if not defined VCVARS if exist "%ProgramFiles%\Microsoft Visual Studio\%%V\Community\VC\Auxiliary\Build\vcvars64.bat" set VCVARS=%ProgramFiles%\Microsoft Visual Studio\%%V\Community\VC\Auxiliary\Build\vcvars64.bat
if not defined VCVARS if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" set VCVARS=%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat
if not defined VCVARS (
    echo [-] vcvars64.bat not found
    exit /b 1
)
call "%VCVARS%" >nul 2>&1
if not exist build mkdir build
cl /nologo /std:c++17 /utf-8 /EHsc /O2 /MT /W3 /D_CRT_SECURE_NO_WARNINGS /DWIN32_LEAN_AND_MEAN /DNOMINMAX /Ivendor\imgui /Fobuild\ src\main.cpp src\game.cpp src\hook.cpp src\draw.cpp src\menu.cpp src\glass.cpp src\rtti.cpp src\scan.cpp vendor\imgui\imgui.cpp vendor\imgui\imgui_draw.cpp vendor\imgui\imgui_tables.cpp vendor\imgui\imgui_widgets.cpp vendor\imgui\imgui_impl_win32.cpp vendor\imgui\imgui_impl_dx11.cpp /LD /link /OUT:build\obsdota2.dll d3d11.lib dxgi.lib user32.lib shell32.lib
if errorlevel 1 (
    echo [] build failed
    exit /b 1
)
echo [+] build\obsdota2.dll
