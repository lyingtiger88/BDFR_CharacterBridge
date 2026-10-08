param(
    [Parameter(Mandatory=$true)][string]$DazSdkDir,
    [string]$DazStudioDir="C:\\Program Files\\DAZ 3D\\DAZStudio4",
    [string]$BuildDir="build",
    [switch]$SkipInstall
)

$ErrorActionPreference="Stop"
$root=Split-Path -Parent $MyInvocation.MyCommand.Path
$build=Join-Path $root $BuildDir

Write-Host "BDFR Body Authoring v0.5.0 native build"
Write-Host "Requested SDK root: $DazSdkDir"

if(!(Test-Path $DazSdkDir)){
    throw "DAZ SDK path does not exist: $DazSdkDir"
}

$dzcoreExpected = Join-Path $DazSdkDir "lib\\x64\\dzcore.lib"
if(!(Test-Path $dzcoreExpected)){
    Write-Host "dzcore.lib was not found at the requested root. Searching nested SDK folders..."
    $candidate = Get-ChildItem -Path $DazSdkDir -Recurse -Filter dzcore.lib -File -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -match "[\\/]lib[\\/](x64|Win64)[\\/]dzcore\.lib$" } |
        Select-Object -First 1

    if($candidate){
        $DazSdkDir = $candidate.Directory.Parent.Parent.FullName
        Write-Host "Detected DAZ SDK root: $DazSdkDir"
    } else {
        Write-Host ""
        Write-Host "Could not locate dzcore.lib under: $DazSdkDir"
        Write-Host "Expected SDK files include:"
        Write-Host "  <SDK>\\include\\dzplugin.h"
        Write-Host "  <SDK>\\lib\\x64\\dzcore.lib"
        Write-Host "  <SDK>\\lib\\x64\\QtCore4.lib"
        Write-Host "  <SDK>\\bin\\x64\\qmake.exe"
        throw "Official DAZ Studio SDK was not found. Point -DazSdkDir to the SDK root folder."
    }
}

$required = @(
    (Join-Path $DazSdkDir "include\\dzplugin.h"),
    (Join-Path $DazSdkDir "lib\\x64\\dzcore.lib"),
    (Join-Path $DazSdkDir "lib\\x64\\QtCore4.lib"),
    (Join-Path $DazSdkDir "bin\\x64\\qmake.exe")
)
foreach($item in $required){
    if(!(Test-Path $item)){
        throw "DAZ SDK appears incomplete. Missing: $item"
    }
}

Write-Host "Using DAZ SDK: $DazSdkDir"
Write-Host "Daz Studio: $DazStudioDir"

cmake -S $root -B $build -A x64 -DDAZ_SDK_DIR="$DazSdkDir"
if($LASTEXITCODE -ne 0){throw "CMake configure failed."}

cmake --build $build --config Release
if($LASTEXITCODE -ne 0){throw "Build failed."}

$builtDll = Get-ChildItem -Path $build -Recurse -Filter "bdfrbodyauthoring.dll" -File -ErrorAction SilentlyContinue |
    Select-Object -First 1

if(!$builtDll){
    throw "Build reported success but bdfrbodyauthoring.dll was not found under: $build"
}

$stageRoot = Join-Path $root "dist\\BDFR_BodyAuthoring_Daz_v0.5.0"
$stagePlugins = Join-Path $stageRoot "plugins"
New-Item -ItemType Directory -Force -Path $stagePlugins | Out-Null
$stagedDll = Join-Path $stagePlugins "bdfrbodyauthoring.dll"
Copy-Item $builtDll.FullName $stagedDll -Force

Write-Host ""
Write-Host "BUILD SUCCESS"
Write-Host "Built DLL: $($builtDll.FullName)"
Write-Host "Staged DLL: $stagedDll"

if($SkipInstall){
    Write-Host "Install step skipped."
    exit 0
}

$pluginDir = Join-Path $DazStudioDir "plugins"
$installedDll = Join-Path $pluginDir "bdfrbodyauthoring.dll"

try {
    if(!(Test-Path $pluginDir)){
        New-Item -ItemType Directory -Force -Path $pluginDir | Out-Null
    }
    Copy-Item $stagedDll $installedDll -Force
    Write-Host "Installed DLL: $installedDll"
    Write-Host "Restart Daz Studio, then run BDFR_BodyAuthoring_Launcher.dsa."
}
catch {
    Write-Warning "The plugin built successfully, but Windows denied installation into Program Files."
    Write-Host ""
    Write-Host "Run PowerShell as Administrator and copy:"
    Write-Host "  Copy-Item `"$stagedDll`" `"$installedDll`" -Force"
    Write-Host ""
    Write-Host "No rebuild is required."
}
