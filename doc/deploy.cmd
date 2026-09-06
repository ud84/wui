@echo off
rem Build locally. Publishing uses deploy.sh with an AWS CLI profile.
python "%~dp0build.py" %*
exit /b %errorlevel%
