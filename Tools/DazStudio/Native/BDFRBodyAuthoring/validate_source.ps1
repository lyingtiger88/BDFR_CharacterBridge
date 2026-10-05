$ErrorActionPreference="Stop"
$root=Split-Path -Parent $MyInvocation.MyCommand.Path
$required=@("CMakeLists.txt","src\\pluginmain.cpp","src\\BDFRBodyAuthoringAction.cpp","src\\BDFRBodyAuthoringDialog.cpp","src\\BDFRBodyViewer.cpp","src\\BDFRBodyProfile.cpp","resources\\BDFRBodyAuthoring.qrc","resources\\female_body_map.png","resources\\male_body_map.png")
$missing=@()
foreach($item in $required){$p=Join-Path $root $item; if(!(Test-Path $p)){$missing+=$item}}
if($missing.Count -gt 0){Write-Error ("Missing required files:`n - "+($missing -join "`n - ")); exit 1}
Write-Host "BDFR Body Authoring native source layout: OK"