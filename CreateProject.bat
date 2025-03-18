@echo off

pushd %~dp0

if "%~1"=="" (
    echo Error: Project name is required.
    exit /b 1
)

if exist ..\%1 (
    echo Error: Target directory ..\%1 already exists
    popd
    exit /b 1
)
mkdir ..\%1 2>nul >nul
robocopy . ..\%1 /e /xd .git /xd .build /xd build /xd bin /np /njh /njs

popd

if %errorlevel% GEQ 8 (
    echo Error: Robocopy failed with exit code %errorlevel%.
    exit /b %errorlevel%
)

cd %~dp0..\%1


rem Configure
mkdir build\msvc
pushd build\msvc
cmake ..\..\cmake -DAPPNAME=%1
popd

echo Copy of template project created: %1 (build\msvc\%1.sln)
