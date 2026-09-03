@echo off
rem Build the Einstein emulator with the Visual Studio toolchain.
rem
rem   build.cmd core   core library + M0 tests only (no network, no deps)
rem   build.cmd app    everything, fetching SDL3 and Dear ImGui from source
rem   build.cmd        same as "app"
rem
rem Note: paths are assigned with set "VAR=..." and expanded as !VAR! -- the
rem "Program Files (x86)" parentheses break the plain %VAR% forms inside
rem parenthesised blocks and for /f commands.
setlocal EnableDelayedExpansion
cd /d "%~dp0"

set "MODE=%~1"
if "!MODE!"=="" set "MODE=app"
set "BUILDTYPE=%~2"
if "!BUILDTYPE!"=="" set "BUILDTYPE=RelWithDebInfo"

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "!VSWHERE!" (
    echo ERROR: vswhere not found at "!VSWHERE!"
    exit /b 1
)

set "VSPATH="
for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSPATH=%%i"
if "!VSPATH!"=="" (
    echo ERROR: no Visual Studio install with the C++ toolset ^(VC.Tools.x86.x64^) found.
    exit /b 1
)
echo Toolchain: !VSPATH!

call "!VSPATH!\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 (
    echo ERROR: vcvars64.bat failed.
    exit /b 1
)

set "CMAKEDIR=!VSPATH!\Common7\IDE\CommonExtensions\Microsoft\CMake"
if not exist "!CMAKEDIR!\CMake\bin\cmake.exe" (
    echo ERROR: bundled cmake.exe not found under "!CMAKEDIR!"
    exit /b 1
)
set "PATH=!CMAKEDIR!\CMake\bin;!CMAKEDIR!\Ninja;!PATH!"

if /i "!MODE!"=="core" (
    set "BUILDDIR=build-core"
    set "CONFIGURE_ARGS=-DEIN_BUILD_APP=OFF -DCMAKE_BUILD_TYPE=!BUILDTYPE!"
) else (
    set "BUILDDIR=build"
    set "CONFIGURE_ARGS=-DEIN_BUILD_APP=ON -DCMAKE_BUILD_TYPE=!BUILDTYPE!"
)

echo === configure [!MODE! / !BUILDTYPE!] ===
cmake -S . -B "!BUILDDIR!" -G Ninja !CONFIGURE_ARGS! || exit /b 1

echo === build ===
cmake --build "!BUILDDIR!" || exit /b 1

echo === test ===
rem zexdoc/zexall are labelled "slow" (~3 min each) and are excluded here.
ctest --test-dir "!BUILDDIR!" --output-on-failure -LE slow || exit /b 1

echo.
echo Build OK: !BUILDDIR!
echo Z80 exercisers: ctest --test-dir !BUILDDIR! -L slow --output-on-failure
if /i not "!MODE!"=="core" echo Run: !BUILDDIR!\src\app\albert.exe
