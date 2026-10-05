param([Parameter(Mandatory=$true)][string]$PluginDll,[string]$OutputDir="dist")
$ErrorActionPreference="Stop"
$root=Split-Path -Parent $MyInvocation.MyCommand.Path
$out=Join-Path $root $OutputDir
$stage=Join-Path $out "BDFR_BodyAuthoring_Daz_v0.5.0"
$zip=Join-Path $out "BDFR_BodyAuthoring_Daz_v0.5.0-win64.zip"
if(!(Test-Path $PluginDll)){throw "Plugin DLL not found: $PluginDll"}
Remove-Item $stage -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path (Join-Path $stage "plugins") | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $stage "Scripts\\Utilities") | Out-Null
Copy-Item $PluginDll (Join-Path $stage "plugins\\bdfrbodyauthoring.dll")
Copy-Item (Join-Path $root "BDFR_BodyAuthoring_Launcher.dsa") (Join-Path $stage "Scripts\\Utilities\\BDFR_BodyAuthoring_Launcher.dsa")
Copy-Item (Join-Path $root "README.md") (Join-Path $stage "README.md")
Copy-Item (Join-Path $root "THIRD_PARTY.md") (Join-Path $stage "THIRD_PARTY.md")
Remove-Item $zip -Force -ErrorAction SilentlyContinue
Compress-Archive -Path (Join-Path $stage "*") -DestinationPath $zip -CompressionLevel Optimal
Write-Host "Package created: $zip"