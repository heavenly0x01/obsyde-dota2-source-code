@echo off
setlocal
set "ROOT=%~dp0"
set "BUILD_DIR=%ROOT%build"
set "SOURCE=%ROOT%injector\inj.cpp"
if not exist "%SOURCE%" (
    echo Source file not found: "%SOURCE%"
    exit /b 1
)
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
powershell -NoProfile -Command "$p = New-Object Security.Principal.WindowsPrincipal([Security.Principal.WindowsIdentity]::GetCurrent()); if ($p.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) { exit 0 } else { exit 1 }"
if errorlevel 1 (
    powershell -NoProfile -Command "Start-Process -FilePath '%~f0' -ArgumentList '/elevated' -WorkingDirectory '%~dp0' -Verb RunAs"
    exit /b
)
where cl.exe >nul 2>&1
if not errorlevel 1 goto compile
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto no_compiler
set "VSINSTALL="
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSINSTALL=%%i"
if not defined VSINSTALL goto no_compiler
if not exist "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat" goto no_compiler
call "%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 goto no_compiler
:compile
cl.exe /nologo /O2 /DNDEBUG /MT /EHsc /Fo"%BUILD_DIR%\inj.obj" "%SOURCE%" /Fe"%BUILD_DIR%\injector.exe" /link /SUBSYSTEM:CONSOLE /MANIFESTUAC:level='requireAdministrator' Shell32.lib > "%BUILD_DIR%\build_inj.log" 2>&1
set "BUILD_RESULT=%ERRORLEVEL%"
type "%BUILD_DIR%\build_inj.log"
if not "%BUILD_RESULT%"=="0" goto build_failed
if not exist "%BUILD_DIR%\injector.exe" goto missing_output
echo Build complete: "%BUILD_DIR%\injector.exe"
exit /b 0
:no_compiler
echo Visual Studio C++ Build Tools were not found. Install the Desktop development with C++ workload.
exit /b 1
:missing_output
echo Build failed: the compiler returned success but the EXE was not created.
exit /b 1
:build_failed
echo Build failed. Full output: "%BUILD_DIR%\build_inj.log"
exit /b 1

