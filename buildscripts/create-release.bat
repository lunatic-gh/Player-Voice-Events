@echo off
setlocal

REM Set the zip file name and path
set ZIP_FILE=release\PlayerVoiceEvents.zip

REM Create the release directory if it doesn't exist
if not exist release (
    mkdir release
)

REM Remove the existing zip file if it exists
if exist %ZIP_FILE% del /f %ZIP_FILE%

REM Create the zip file and include the specified files and directories
powershell -Command "Compress-Archive -Path .\Scripts\, .\Seq\, .\SKSE\, .\LICENSE, .\PVE-Conditions.jsonc, .\PlayerVoiceEvents.esp, .\Sound\, .\reference\ -DestinationPath %ZIP_FILE%"

echo Created Release at: "%ZIP_FILE%"

endlocal
