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
robocopy . ..\%1 /e /xd .git /xd .build /xd build /xf CreateProject.bat /xd bin /np /njh /njs

popd

if %errorlevel% GEQ 8 (
    echo Error: Robocopy failed with exit code %errorlevel%.
    exit /b %errorlevel%
)

cd %~dp0..\%1


rem Configure
mkdir build
pushd build
cmake ..\cmake -DAPPNAME=%1
popd

rem Create configure.bat file
echo ^@echo off > Configure.bat
echo if not exist build mkdir build >> Configure.bat
echo pushd build >> Configure.bat
echo cmake ..\cmake -DAPPNAME=%1 >> Configure.bat
echo popd >> Configure.bat

echo Copy of template project created: %1 (build\%1.sln)
echo Run/Edit Configure.bat for reconfiguring and adding extra options
