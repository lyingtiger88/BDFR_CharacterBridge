param(
    [Parameter(Mandatory=$true)][string]$DazSdkDir,
    [string]$DazStudioDir="C:\\Program Files\\DAZ 3D\\DAZStudio4",
    [string]$BuildDir="build"
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

cmake -S $root -B $build -A x64 -DDAZ_SDK_DIR="$DazSdkDir" -DDAZ_STUDIO_EXE_DIR="$DazStudioDir"
if($LASTEXITCODE -ne 0){throw "CMake configure failed."}

cmake --build $build --config Release
if($LASTEXITCODE -ne 0){throw "Build failed."}

$dll=Join-Path $DazStudioDir "plugins\\bdfrbodyauthoring.dll"
if(Test-Path $dll){
    Write-Host "SUCCESS: $dll"
    Write-Host "Restart Daz Studio, then run BDFR_BodyAuthoring_Launcher.dsa."
} else {
    Write-Warning "Build completed but DLL was not found at $dll"
}
