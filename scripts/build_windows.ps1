Param(
    [string]$Generator = "Visual Studio 17 2022",
    [string]$Arch = "x64",
    [string]$BuildType = "Release",
    [switch]$Build
)

$ErrorActionPreference = "Stop"

# Project root (parent of this scripts folder)
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build"

if (!(Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

function Get-Pybind11CMakeDir {
    try {
        $d = & python -c "import pybind11; print(pybind11.get_cmake_dir())"
        if ($LASTEXITCODE -eq 0 -and $d) { return $d }
    } catch {}
    try {
        $d = & python -c "import pybind11, pathlib; print(pathlib.Path(pybind11.__file__).parent/'share'/'cmake'/'pybind11')"
        if ($LASTEXITCODE -eq 0 -and $d) { return $d }
    } catch {}
    throw "Could not locate pybind11 CMake dir. Ensure 'pip install pybind11' in the active Python environment."
}

$pybind11Dir = Get-Pybind11CMakeDir

$cmakeArgs = @(
    "-S", $ProjectRoot,
    "-B", $BuildDir,
    "-G", $Generator,
    "-A", $Arch,
    "-DBUILD_PYTHON_BINDINGS=ON",
    "-Dpybind11_DIR=$pybind11Dir"
)

Write-Host "Configuring: cmake $($cmakeArgs -join ' ')"
& cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }

if ($Build) {
    Write-Host "Building: cmake --build `"$BuildDir`" --config $BuildType"
    & cmake --build "$BuildDir" --config $BuildType
    if ($LASTEXITCODE -ne 0) { throw "Build failed" }
}

Write-Host "Done. Build directory: $BuildDir"
