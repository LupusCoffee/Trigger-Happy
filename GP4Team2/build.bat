@echo off
setlocal enabledelayedexpansion

REM === Configuration ===
REM Set your Unreal Engine version here
set ENGINE_VERSION=5.6

REM === Start Timer ===
set START_TIME=%TIME%

REM === Auto-detect project and engine info ===
for %%F in (*.uproject) do set PROJECT_NAME=%%~nF
set PROJECT_PATH=%~dp0%PROJECT_NAME%.uproject
set PACKAGE_DIR=%~dp0Saved\StagedBuilds
set ENGINE_PATH=C:\Program Files\Epic Games\UE_%ENGINE_VERSION%
if not exist "%ENGINE_PATH%" (
    echo Unreal Engine %ENGINE_VERSION% not found at %ENGINE_PATH%. Please adjust the ENGINE_PATH variable in this script.
    exit /b 1
)
set PLATFORM=Win64

REM === Handle configuration argument ===
set CONFIG=Shipping
if /i "%1"=="dev" (
    set CONFIG=Development
)

REM === Display build info ===
echo.
echo ================================================================================
echo   STARTING BUILD
echo ================================================================================
echo   Project:        %PROJECT_NAME%
echo   Platform:       %PLATFORM%
echo   Configuration:  %CONFIG%
echo   Engine:         UE %ENGINE_VERSION%
echo ================================================================================
echo.

REM === Run the build (don't exit early if it's already up to date)
call "%ENGINE_PATH%\Engine\Build\BatchFiles\Build.bat" %PROJECT_NAME% %PLATFORM% %CONFIG% "%PROJECT_PATH%" -waitmutex -nobuilduht

REM === Package the project ===
echo.
echo ================================================================================
echo   PACKAGING PROJECT
echo ================================================================================
echo.
call "%ENGINE_PATH%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun ^
    -project="%PROJECT_PATH%" ^
    -platform=%PLATFORM% ^
    -clientconfig=%CONFIG% ^
    -serverconfig=%CONFIG% ^
    -cook ^
    -iterate ^
    -maps=All ^
    -build ^
    -stage ^
    -archive ^
    -archivedirectory="PackagedBuild"

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ================================================================================
    echo   BUILD FAILED!
    echo ================================================================================
    exit /b %ERRORLEVEL%
)

REM === Calculate and display execution time ===
set END_TIME=%TIME%
set /A ELAPSED_H=%END_TIME:~0,2%-%START_TIME:~0,2%
set /A ELAPSED_M=%END_TIME:~3,2%-%START_TIME:~3,2%
set /A ELAPSED_S=%END_TIME:~6,2%-%START_TIME:~6,2%
set /A ELAPSED_CS=%END_TIME:~9,2%-%START_TIME:~9,2%

if %ELAPSED_CS% LSS 0 (
    set /A ELAPSED_S=%ELAPSED_S%-1
    set /A ELAPSED_CS+=100
)
if %ELAPSED_S% LSS 0 (
    set /A ELAPSED_M=%ELAPSED_M%-1
    set /A ELAPSED_S+=60
)
if %ELAPSED_M% LSS 0 (
    set /A ELAPSED_H=%ELAPSED_H%-1
    set /A ELAPSED_M+=60
)

REM Format time with leading zeros
if %ELAPSED_H% LSS 10 set ELAPSED_H=0%ELAPSED_H%
if %ELAPSED_M% LSS 10 set ELAPSED_M=0%ELAPSED_M%
if %ELAPSED_S% LSS 10 set ELAPSED_S=0%ELAPSED_S%
if %ELAPSED_CS% LSS 10 set ELAPSED_CS=0%ELAPSED_CS%

REM === Display success summary ===
echo.
echo ================================================================================
echo   BUILD SUCCESSFUL!
echo ================================================================================
echo.
echo   PROJECT:        %PROJECT_NAME%
echo   PLATFORM:       %PLATFORM%
echo   CONFIGURATION:  %CONFIG%
echo   BUILD TIME:     %ELAPSED_H%:%ELAPSED_M%:%ELAPSED_S%.%ELAPSED_CS%
echo.
echo ================================================================================
echo   BUILD LOCATION
echo ================================================================================
echo.
echo   Output Directory: %PACKAGE_DIR%\Windows
echo   Executable:       %PACKAGE_DIR%\Windows\%PROJECT_NAME%.exe
echo.
echo   To run your build:
echo   cd "%PACKAGE_DIR%\Windows"
echo   %PROJECT_NAME%.exe
echo.
echo ================================================================================
echo.

exit /b 0
