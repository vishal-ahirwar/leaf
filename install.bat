@echo off
setlocal

set "LEAF_DIR=%USERPROFILE%\leaf"
set "DOWNLOAD_URL_LEAF=https://example.com/leaf.exe"
set "DOWNLOAD_URL_UPDATER=https://example.com/updater.exe"

echo Creating directory %LEAF_DIR%...
if not exist "%LEAF_DIR%" (
    mkdir "%LEAF_DIR%"
)

echo Downloading leaf.exe...
powershell -Command "Invoke-WebRequest -Uri %DOWNLOAD_URL_LEAF% -OutFile %LEAF_DIR%\leaf.exe"

echo Downloading updater.exe...
powershell -Command "Invoke-WebRequest -Uri %DOWNLOAD_URL_UPDATER% -OutFile %LEAF_DIR%\updater.exe"

echo Adding %LEAF_DIR% to PATH...
setx PATH "%PATH%;%LEAF_DIR%"

echo Installation complete. Please restart your terminal for the changes to take effect.
endlocal
