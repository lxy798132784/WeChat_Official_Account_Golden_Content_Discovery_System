$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$Build = Join-Path $Root "build-windows"
$Dist = Join-Path $Root "dist\windows-x64"
$Version = if ($env:VERSION) { $env:VERSION } else { "1.0.0" }

cmake -S $Root -B $Build -DCMAKE_BUILD_TYPE=Release
cmake --build $Build --config Release
ctest --test-dir $Build --output-on-failure

$Package = Join-Path $Dist "premium-content-radar-$Version-windows-x64"
Remove-Item -Recurse -Force $Dist -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path "$Package\bin", "$Package\plugins", "$Package\docs" | Out-Null
Copy-Item "$Build\Release\premium-content-radar.exe" "$Package\bin\" -ErrorAction SilentlyContinue
Copy-Item "$Build\plugins\Release\*WeChatProviderPlugin*" "$Package\plugins\" -ErrorAction SilentlyContinue
Copy-Item "$Root\README.md", "$Root\DEVELOPER_GUIDE.md", "$Root\LICENSE", "$Root\CHANGELOG.md", "$Root\SECURITY.md", "$Root\CONTRIBUTING.md", "$Root\CODE_OF_CONDUCT.md" "$Package\docs\"
Copy-Item "$Root\docs" "$Package\docs\project-docs" -Recurse
Compress-Archive -Path $Package -DestinationPath "$Dist\premium-content-radar-$Version-windows-x64.zip" -Force
Write-Host "Package written: $Dist\premium-content-radar-$Version-windows-x64.zip"
