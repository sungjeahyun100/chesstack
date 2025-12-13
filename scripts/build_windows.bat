@echo off
setlocal enabledelayedexpansion

rem Project root is parent of this scripts folder
set SCRIPT_DIR=%~dp0
for %%i in ("%SCRIPT_DIR%..") do set PROJ=%%~fi
set BUILD=%PROJ%\build
if not exist "%BUILD%" mkdir "%BUILD%"

set GEN=Visual Studio 17 2022
set ARCH=x64
set CONFIG=Release

rem Find pybind11 CMake dir via Python
set P11=
for /f "delims=" %%i in ('python -c "import pybind11; print(pybind11.get_cmake_dir())"') do set P11=%%i
if not defined P11 (
  for /f "delims=" %%i in ('python -c "import pybind11, pathlib; print(pathlib.Path(pybind11.__file__).parent / 'share' / 'cmake' / 'pybind11')"') do set P11=%%i
)
if not defined P11 (
  echo Could not locate pybind11 CMake dir. Ensure ^"pip install pybind11^" in this Python environment.
  exit /b 1
)

echo Configuring with pybind11_DIR="%P11%"
cmake -S "%PROJ%" -B "%BUILD%" -G "%GEN%" -A %ARCH% -DBUILD_PYTHON_BINDINGS=ON -Dpybind11_DIR="%P11%"
if errorlevel 1 exit /b 1

echo Building...
cmake --build "%BUILD%" --config %CONFIG%
exit /b %errorlevel%
