@echo off
title TypeAssist Server
cd /d "%~dp0"

echo =========================================================
echo      TYPEASSIST: INTELLIGENT WRITING ASSISTANT           
echo =========================================================
echo.
echo [1/2] Verifying linguistic corpora and datasets...
call node data\scripts\prepare_data.js
echo.
echo [2/2] Launching web server on http://localhost:3001 ...
echo.
echo Open your browser at: http://localhost:3001
echo Press Ctrl+C to stop the server anytime.
echo =========================================================
echo.

call npx next dev --webpack -p 3001

pause

