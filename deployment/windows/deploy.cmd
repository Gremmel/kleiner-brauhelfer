@ECHO OFF

SET EXE_FILE=kleiner-brauhelfer.exe
SET DEPLOY_DIR=deploy

ECHO - Select application folder (e.g. .\build-brauhelfer-Desktop_Qt_5_10_0_MSVC2017_64bit-Release\bin)
FOR /F "Tokens=1 Delims=" %%I IN ('cscript //nologo "%~dp0BrowseFolder.vbs"') DO SET EXE_DIR=%%I
IF "%EXE_DIR%" == "" GOTO :EOF 
IF NOT EXIST "%EXE_DIR%\%EXE_FILE%" GOTO :EOF 

ECHO - Create deployment directory "%DEPLOY_DIR%"
MKDIR "%~dp0%DEPLOY_DIR%"

ECHO - Copy application file "%EXE_DIR%\%EXE_FILE%"
XCOPY "%EXE_DIR%\%EXE_FILE%" "%~dp0%DEPLOY_DIR%" /Y

ECHO - Copy language files
mkdir "%~dp0%DEPLOY_DIR%\languages"
XCOPY "%~dp0..\..\source\languages\*.qm" "%~dp0%DEPLOY_DIR%\languages" /Y
XCOPY "%~dp0..\..\source\languages\*.png" "%~dp0%DEPLOY_DIR%\languages" /Y

ECHO - Copy logo file
XCOPY "%~dp0..\..\source\res\logo.ico" "%~dp0%DEPLOY_DIR%" /Y

ECHO - Select Qt bin folder (e.g. C:\Qt\5.3\mingw482_32\bin)
FOR /F "Tokens=1 Delims=" %%I IN ('cscript //nologo "%~dp0BrowseFolder.vbs"') DO SET QT_DIR=%%I
IF "%QT_DIR%" == "" GOTO :EOF 
IF NOT EXIST "%QT_DIR%\qmake.exe" GOTO :EOF 

ECHO - Set Qt path to "%QT_DIR%"
SET PATH=%QT_DIR%;%PATH%

ECHO - Run windeployqt
windeployqt "%~dp0%DEPLOY_DIR%\%EXE_FILE%"
MOVE /Y "%~dp0%DEPLOY_DIR%\*.qm" "%~dp0%DEPLOY_DIR%\languages"

ECHO - Copy additional DLLs
XCOPY "%QT_DIR%\libgcc_s_dw2-1.dll" "%~dp0%DEPLOY_DIR%" /Y
XCOPY "%QT_DIR%\libstdc++-6.dll" "%~dp0%DEPLOY_DIR%" /Y
XCOPY "%QT_DIR%\libwinpthread-1.dll" "%~dp0%DEPLOY_DIR%" /Y

ECHO Deployed to:
ECHO %~dp0%DEPLOY_DIR%

ECHO - Start Inno Setup Compiler
ECHO Edit MyAppVersion and MyAppOutFileName before compiling
"%~dp0setup.iss"

:EOF
