param([Parameter(Mandatory=$true)][string]$DazSdkDir,[string]$DazStudioDir="C:\\Program Files\\DAZ 3D\\DAZStudio4",[string]$BuildDir="build")
$ErrorActionPreference="Stop"
$root=Split-Path -Parent $MyInvocation.MyCommand.Path
$build=Join-Path $root $BuildDir
Write-Host "BDFR Body Authoring v0.5.0 native build"
cmake -S $root -B $build -A x64 -DDAZ_SDK_DIR="$DazSdkDir" -DDAZ_STUDIO_EXE_DIR="$DazStudioDir"
if($LASTEXITCODE -ne 0){throw "CMake configure failed."}
cmake --build $build --config Release
if($LASTEXITCODE -ne 0){throw "Build failed."}
$dll=Join-Path $DazStudioDir "plugins\\bdfrbodyauthoring.dll"
if(Test-Path $dll){Write-Host "SUCCESS: $dll"; Write-Host "Restart Daz Studio, then run BDFR_BodyAuthoring_Launcher.dsa."} else {Write-Warning "Build completed but DLL was not found at $dll"}