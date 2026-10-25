# Packages an already-tested Release build; does not install tools or publish releases.
[CmdletBinding()]
param([string]$BuildDir = 'build/windows', [string]$OutputDir = 'build/packages', [string]$Iscc = '')
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
function Invoke-Checked {
    param([string]$Executable, [string[]]$Arguments)
    & $Executable @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Executable failed (exit $LASTEXITCODE)" }
}
try {
    $root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
    Set-Location -LiteralPath $root
    $build = (Resolve-Path -LiteralPath $BuildDir).Path
    $output = [IO.Path]::GetFullPath((Join-Path $root $OutputDir))
    $stage = Join-Path $build 'package-stage'
    # Only remove our dedicated staging directory, never an arbitrary caller-supplied root.
    if (Test-Path $stage) { Remove-Item -LiteralPath $stage -Recurse -Force }
    New-Item -ItemType Directory -Force $stage, $output | Out-Null
    if (-not $Iscc) { $Iscc = "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe" }
    if (-not (Test-Path -LiteralPath $Iscc)) { throw 'Inno Setup compiler missing. See docs/windows-setup.md (packaging tools).' }
    # A Debug executable must never accidentally be packaged as a release build.
    if (-not (Test-Path (Join-Path $build 'app/Release/simplicity.exe'))) { throw 'Release executable missing. Run setup.cmd first.' }
    Invoke-Checked 'ctest.exe' @('--test-dir', $build, '-C', 'Release', '--output-on-failure', '--no-tests=error')
    Invoke-Checked 'cmake.exe' @('--install', $build, '--config', 'Release', '--prefix', $stage)
    $version = (Select-String -Path 'CMakeLists.txt' -Pattern 'project\(simplicity VERSION (\d+\.\d+\.\d+)').Matches[0].Groups[1].Value
    Invoke-Checked $Iscc @("/DStageDir=$stage", "/DOutputDir=$output", "/DAppVersion=$version", 'packaging/windows/simplicity.iss')
    $zip = Join-Path $output 'Simplicity-Portable.zip'
    if (Test-Path $zip) { Remove-Item -LiteralPath $zip }
    Compress-Archive -Path "$stage/*" -DestinationPath $zip
    $lines = foreach ($name in @('Simplicity-Setup.exe', 'Simplicity-Portable.zip')) {
        $hash = (Get-FileHash -LiteralPath (Join-Path $output $name) -Algorithm SHA256).Hash.ToLowerInvariant()
        "$hash  $name"
    }
    $lines | Set-Content -LiteralPath (Join-Path $output 'SHA256SUMS.txt') -Encoding ascii
    Write-Host "Prepared development artifacts in $output. Public distribution remains licensing-gated."
    exit 0
} catch {
    Write-Host "PACKAGING FAILED: $($_.Exception.Message)" -ForegroundColor Red
    if ($env:GITHUB_ACTIONS -eq 'true') { Write-Host "::error::PACKAGING FAILED: $($_.Exception.Message)" }
    exit 1
}
