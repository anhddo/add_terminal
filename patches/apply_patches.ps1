# PowerShell script to copy TWS API and apply modifications
param(
    [string]$SourcePath = "C:\TWS API",
    [string]$DestPath = "third_party\tws_api"
)

Write-Host "=== TWS API Patch Application Script ===" -ForegroundColor Cyan

# Check if source exists
if (-not (Test-Path $SourcePath)) {
    Write-Host "ERROR: Source TWS API not found at: $SourcePath" -ForegroundColor Red
    Write-Host "Please specify the correct path using -SourcePath parameter" -ForegroundColor Yellow
    exit 1
}

# Get script directory (patches folder)
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = Split-Path -Parent $ScriptDir

# Full destination path
$FullDestPath = Join-Path $RepoRoot $DestPath

Write-Host "Source: $SourcePath" -ForegroundColor Gray
Write-Host "Destination: $FullDestPath" -ForegroundColor Gray
Write-Host ""

# Remove existing destination if it exists
if (Test-Path $FullDestPath) {
    Write-Host "Removing existing TWS API at destination..." -ForegroundColor Yellow
    Remove-Item -Path $FullDestPath -Recurse -Force
}

# Copy TWS API
Write-Host "Copying TWS API from source..." -ForegroundColor Green
Copy-Item -Path $SourcePath -Destination $FullDestPath -Recurse

Write-Host "Applying modifications..." -ForegroundColor Green

$allSuccess = $true

# 1. Update CMake minimum version in root CMakeLists.txt
try {
    Write-Host "  1. Updating CMake minimum version..." -ForegroundColor Gray
    $cmakeFile = Join-Path $FullDestPath "CMakeLists.txt"
    $content = Get-Content $cmakeFile -Raw
    $content = $content -replace 'cmake_minimum_required\(VERSION 3\.0\)', 'cmake_minimum_required(VERSION 3.5)'
    Set-Content $cmakeFile -Value $content -NoNewline
    Write-Host "    ✓ Updated successfully" -ForegroundColor Green
} catch {
    Write-Host "    Failed to update CMake version: $_" -ForegroundColor Red
    $allSuccess = $false
}

# 2. Copy fixed client CMakeLists.txt
try {
    Write-Host "  2. Copying fixed client CMakeLists.txt..." -ForegroundColor Gray
    $clientCMakeSource = Join-Path $ScriptDir "tws_api_client_cmake.txt"
    $clientCMakeDest = Join-Path $FullDestPath "source\cppclient\client\CMakeLists.txt"

    if (-not (Test-Path $clientCMakeSource)) {
        throw "Source CMakeLists.txt not found: $clientCMakeSource"
    }

    Copy-Item -Path $clientCMakeSource -Destination $clientCMakeDest -Force
    Write-Host "    ✓ Copied successfully" -ForegroundColor Green
} catch {
    Write-Host "    Failed to copy client CMakeLists: $_" -ForegroundColor Red
    $allSuccess = $false
}

# 3. Create twsapiConfig.cmake.in
try {
    Write-Host "  3. Creating twsapiConfig.cmake.in..." -ForegroundColor Gray
    $configFile = Join-Path $FullDestPath "twsapiConfig.cmake.in"
    $configContent = @'
# twsapi CMake configuration file

@PACKAGE_INIT@

set_and_check(twsapi_INCLUDE_DIRS "@CONF_INCLUDE_DIRS@")

include("${CMAKE_CURRENT_LIST_DIR}/twsapiTargets.cmake")

check_required_components(twsapi)

'@
    Set-Content $configFile -Value $configContent
    Write-Host "    ✓ Created successfully" -ForegroundColor Green
} catch {
    Write-Host "    Failed to create twsapiConfig.cmake.in: $_" -ForegroundColor Red
    $allSuccess = $false
}

# 4. Create twsapiConfigVersion.cmake.in
try {
    Write-Host "  4. Creating twsapiConfigVersion.cmake.in..." -ForegroundColor Gray
    $versionFile = Join-Path $FullDestPath "twsapiConfigVersion.cmake.in"
    $versionContent = @'
# Stub file - not used in this build configuration
set(PACKAGE_VERSION "@PROJECT_VERSION@")

# Check whether the requested PACKAGE_FIND_VERSION is compatible
if("${PACKAGE_VERSION}" VERSION_LESS "${PACKAGE_FIND_VERSION}")
  set(PACKAGE_VERSION_COMPATIBLE FALSE)
else()
  set(PACKAGE_VERSION_COMPATIBLE TRUE)
  if("${PACKAGE_VERSION}" VERSION_EQUAL "${PACKAGE_FIND_VERSION}")
    set(PACKAGE_VERSION_EXACT TRUE)
  endif()
endif()

'@
    Set-Content $versionFile -Value $versionContent
    Write-Host "    ✓ Created successfully" -ForegroundColor Green
} catch {
    Write-Host "    Failed to create twsapiConfigVersion.cmake.in: $_" -ForegroundColor Red
    $allSuccess = $false
}

Write-Host ""
if ($allSuccess) {
    Write-Host "=== All modifications applied successfully ===" -ForegroundColor Green
} else {
    Write-Host "=== Some modifications failed ===" -ForegroundColor Red
}

Write-Host ""
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "  1. Configure CMake: cmake --preset=default" -ForegroundColor Gray
Write-Host "  2. Build: cmake --build build" -ForegroundColor Gray
