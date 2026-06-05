$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$Build = Join-Path $Root "build-windows"
$Dist = Join-Path $Root "dist\windows-x64"
$Version = if ($env:VERSION) { $env:VERSION } else { "1.0.1" }

cmake -S $Root -B $Build -DCMAKE_BUILD_TYPE=Release
cmake --build $Build --config Release
ctest --test-dir $Build -C Release --output-on-failure

$Package = Join-Path $Dist "premium-content-radar-$Version-windows-x64"
Remove-Item -Recurse -Force $Dist -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path "$Package\bin", "$Package\plugins", "$Package\docs" | Out-Null

$Exe = Get-ChildItem -Path $Build -Recurse -Filter "premium-content-radar.exe" | Select-Object -First 1
if (-not $Exe) { throw "premium-content-radar.exe was not found under $Build" }
Copy-Item $Exe.FullName "$Package\bin\"

$PluginFiles = Get-ChildItem -Path $Build -Recurse | Where-Object { $_.Name -match "WeChatProviderPlugin" -and ($_.Extension -in ".dll", ".so", ".dylib") }
if (-not $PluginFiles) { throw "WeChatProviderPlugin binary was not found under $Build" }
$PluginFiles | ForEach-Object { Copy-Item $_.FullName "$Package\plugins\" }

$Windeployqt = Get-Command windeployqt -ErrorAction SilentlyContinue
if ($Windeployqt) {
  & $Windeployqt.Source --release --no-translations --no-compiler-runtime "$Package\bin\premium-content-radar.exe"
}

Copy-Item "$Root\README.md", "$Root\LICENSE", "$Root\CHANGELOG.md", "$Root\SECURITY.md", "$Root\CONTRIBUTING.md", "$Root\CODE_OF_CONDUCT.md" "$Package\docs\"
Copy-Item "$Root\docs\en" "$Package\docs\en" -Recurse
Copy-Item "$Root\docs\zh-CN" "$Package\docs\zh-CN" -Recurse
Copy-Item "$Root\docs\README.zh-CN.md" "$Package\docs\README.zh-CN.md"
Copy-Item "$Root\docs\assets" "$Package\docs\assets" -Recurse
Compress-Archive -Path "$Package\*" -DestinationPath "$Dist\premium-content-radar-$Version-windows-x64.zip" -Force
Get-FileHash "$Dist\premium-content-radar-$Version-windows-x64.zip" -Algorithm SHA256 | Format-List | Out-File "$Dist\SHA256SUMS-windows-x64.txt"
Write-Host "Package written: $Dist\premium-content-radar-$Version-windows-x64.zip"
