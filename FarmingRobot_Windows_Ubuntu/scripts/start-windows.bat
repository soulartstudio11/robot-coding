@echo off
setlocal
cd /d "%~dp0..\web-terminal"
echo Farming Robot terminal: http://localhost:8000
where py >nul 2>nul
if %errorlevel%==0 (
  start "" http://localhost:8000
  py -m http.server 8000
) else (
  where python >nul 2>nul
  if %errorlevel%==0 (
    start "" http://localhost:8000
    python -m http.server 8000
  ) else (
    echo Python not found. Install Python 3 from https://www.python.org/downloads/
    pause
  )
)
