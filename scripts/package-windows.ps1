# Makes the Windows release files from a finished build:
#   GlossCode-<version>-windows-x64-setup.exe      (Inno Setup, packaging/windows/glosscode.iss)
#   GlossCode-<version>-windows-x64-portable.zip
#
#   pwsh scripts/package-windows.ps1 [-Build build] [-Out dist]
#
# Needs Inno Setup 6.3 or later (https://jrsoftware.org/isinfo.php, or: winget install JRSoftware.InnoSetup).
param([string]$Build = "build", [string]$Out = "dist")
$ErrorActionPreference = "Stop"

$src = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$Build = (Resolve-Path $Build).Path
New-Item -ItemType Directory -Force $Out | Out-Null
$Out = (Resolve-Path $Out).Path
$version = (Get-Content (Join-Path $src "VERSION") -Raw).Trim()
$exe = Join-Path $Build "GlossCode.exe"
if (-not (Test-Path $exe)) { throw "$exe isn't there; build first (docs/building.md)" }

# Installer.
$iscc = @(
    (Get-Command iscc -ErrorAction SilentlyContinue).Source,
    "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe",
    "$env:ProgramFiles\Inno Setup 6\ISCC.exe",
    "$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe"
) | Where-Object { $_ -and (Test-Path $_) } | Select-Object -First 1
if (-not $iscc) { throw "Inno Setup isn't installed; see the top of this script" }
& $iscc /Q "/DAppVersion=$version" "/DSourceDir=$src" "/DExe=$exe" "/DOutputDir=$Out" (Join-Path $src "packaging\windows\glosscode.iss")
if ($LASTEXITCODE -ne 0) { throw "Inno Setup failed ($LASTEXITCODE)" }

# Portable zip: the program and an empty "portable" file, so settings stay in GlossCode-data beside it.
$top = "GlossCode-$version"
$stage = Join-Path $Build "package\portable"
$p = Join-Path $stage $top
if (Test-Path $stage) { Remove-Item -Recurse -Force $stage }
New-Item -ItemType Directory -Force $p | Out-Null
Copy-Item $exe (Join-Path $p "GlossCode.exe")
New-Item -ItemType File (Join-Path $p "portable") | Out-Null
Copy-Item (Join-Path $src "LICENSE") (Join-Path $p "LICENSE.txt")
Copy-Item (Join-Path $src "THIRD_PARTY_NOTICES.md") (Join-Path $p "THIRD_PARTY_NOTICES.txt")
Copy-Item (Join-Path $src "CHANGELOG.md") (Join-Path $p "CHANGELOG.txt")
Copy-Item (Join-Path $src "packaging\portable-readme.txt") (Join-Path $p "README.txt")
$zip = Join-Path $Out "GlossCode-$version-windows-x64-portable.zip"
if (Test-Path $zip) { Remove-Item $zip }
Compress-Archive -Path $p -DestinationPath $zip

Get-ChildItem $Out | Format-Table Name, Length
