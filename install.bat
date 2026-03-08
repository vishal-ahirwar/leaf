@echo off
setlocal

set "LEAF_DIR=%USERPROFILE%\leaf"
set "DOWNLOAD_URL_LEAF=https://github.com/vishal-ahirwar/leaf/releases/download/leafv0.2.2/leaf-windows.zip"
set "ZIP_PATH=%LEAF_DIR%\leaf.zip"
set "PS=powershell -NoProfile -ExecutionPolicy Bypass -Command"

echo Creating directory %LEAF_DIR%...
if not exist "%LEAF_DIR%" (
    mkdir "%LEAF_DIR%"
)

echo Downloading leaf...
%PS% "Invoke-WebRequest -Uri '%DOWNLOAD_URL_LEAF%' -OutFile '%ZIP_PATH%'"
if errorlevel 1 (
    echo Failed to download %DOWNLOAD_URL_LEAF%
    exit /b 1
)

%PS% "Expand-Archive -Path '%ZIP_PATH%' -DestinationPath '%LEAF_DIR%' -Force"
if errorlevel 1 (
    echo Failed to extract archive at %ZIP_PATH%
    exit /b 1
)

del /f /q "%ZIP_PATH%" >nul 2>&1

echo Adding %LEAF_DIR% to PATH...
%PS% "$target='%LEAF_DIR%'; $userPath=[Environment]::GetEnvironmentVariable('Path','User'); if([string]::IsNullOrWhiteSpace($userPath)){ [Environment]::SetEnvironmentVariable('Path',$target,'User'); Write-Output 'User PATH initialized.' } elseif(($userPath -split ';') -contains $target){ Write-Output 'PATH already contains target.' } else { [Environment]::SetEnvironmentVariable('Path',($userPath.TrimEnd(';') + ';' + $target),'User'); Write-Output 'User PATH updated.' }"

echo Installation complete. Please restart your terminal for the changes to take effect.
endlocal
