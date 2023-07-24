@echo off
set shell_path=%~dp0
set root_path=%shell_path%..

if "%1" == "" (
    echo "Usage: build-windows.bat <arch> <build_type>"
    echo "  <arch>       : x64, Win32"
    echo "  <build_type> : Debug, Release"
    exit /b 1
)

call:build %~1 %~2

exit /b 0

:build
    cd %root_path%
    mkdir .\build\windows\%~1
    cd .\build\windows\%~1

    if "%~2" == "Debug" (
        set build_debugger=ON
    ) else (
        set build_debugger=OFF
    )

    cmake ^
        -G "Visual Studio 17 2022" ^
        -A %~1 ^
        -DCMAKE_BUILD_TYPE=%~2 ^
        -DPANGU_DEBUGGER=%build_debugger% ^
        %root_path%
    cmake --build . --config %~2
    cd %shell_path%
goto:eof