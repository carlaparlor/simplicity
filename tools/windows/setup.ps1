# Developer bootstrap only. End users run the packaged installer; they need none of these tools.
[CmdletBinding()]
param([switch]$NoInstall, [ValidateSet('Debug', 'Release')][string]$Configuration = 'Release')
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

function Invoke-Checked {
    param([string]$Executable, [string[]]$Arguments)
    & $Executable @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Executable failed (exit $LASTEXITCODE). See the output above." }
}
function Update-Path {
    $env:Path = [Environment]::GetEnvironmentVariable('Path', 'Machine') + ';' +
                [Environment]::GetEnvironmentVariable('Path', 'User') + ';' + $env:Path
}
function Install-Tool {
    param([string]$Id, [string[]]$Extra = @())
    if ($NoInstall) { throw "Missing prerequisite: $Id. Re-run setup.cmd without -NoInstall to install it." }
    if (-not (Get-Command winget.exe -ErrorAction SilentlyContinue)) {
        throw 'Install/update App Installer from Microsoft Store (winget), then re-run setup.cmd. See docs/windows-setup.md.'
    }
    Write-Host "Installing $Id through Windows Package Manager..."
    Invoke-Checked 'winget.exe' (@('install', '--id', $Id, '--exact', '--source', 'winget',
        '--silent', '--accept-package-agreements', '--accept-source-agreements', '--disable-interactivity') + $Extra)
    Update-Path
}
function Find-Python {
    # Store aliases and broken launchers must not abort discovery before winget can help.
    $ErrorActionPreference = 'Continue'
    foreach ($name in @('py.exe', 'python.exe', 'python3.exe')) {
        $command = Get-Command $name -ErrorAction SilentlyContinue
        if ($command) {
            $arguments = @('-c', 'import sys; print(sys.executable) if sys.version_info >= (3,11) else sys.exit(1)')
            if ($name -eq 'py.exe') { $arguments = @('-3') + $arguments }
            try { $result = & $command.Source @arguments 2>$null } catch { continue }
            if ($LASTEXITCODE -eq 0 -and $result) { return [string]($result | Select-Object -Last 1) }
        }
    }
    return $null
}
function Find-VisualStudio {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        return & $vswhere -latest -products '*' -version '[17.0,18.0)' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    }
    return $null
}

try {
    if ([Environment]::OSVersion.Platform -ne [PlatformID]::Win32NT -or -not [Environment]::Is64BitOperatingSystem) {
        throw 'Simplicity development requires 64-bit Windows 10/11.'
    }
    if ($env:PROCESSOR_ARCHITECTURE -eq 'ARM64' -or $env:PROCESSOR_ARCHITEW6432 -eq 'ARM64') {
        throw 'Native ARM64 is not supported yet. Use an x64 Windows development machine.'
    }
    $root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
    Set-Location -LiteralPath $root
    Write-Host 'Simplicity Windows setup: check tools, fetch pinned JUCE, build, test.'
    Write-Host 'Missing developer tools may require UAC/elevation, internet and several GB. No global execution policy changes.'
    if (-not (Get-Command git.exe -ErrorAction SilentlyContinue)) { Install-Tool 'Git.Git' }
    if (-not (Get-Command cmake.exe -ErrorAction SilentlyContinue)) { Install-Tool 'Kitware.CMake' }
    $python = Find-Python
    if (-not $python) { Install-Tool 'Python.Python.3.13'; $python = Find-Python }
    if (-not $python) { throw 'Python 3.11+ was not found after installation. Open a fresh terminal and re-run setup.cmd.' }
    if (-not (Find-VisualStudio)) {
        Install-Tool 'Microsoft.VisualStudio.2022.BuildTools' @('--override', '--wait --passive --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended')
    }
    if (-not (Find-VisualStudio)) { throw 'Visual Studio 2022 C++ tools are unavailable. Reboot if requested, then re-run setup.cmd.' }
    Invoke-Checked $python @('tools/check_architecture.py')
    Invoke-Checked 'cmake.exe' @('--preset', 'windows', "-DPython3_EXECUTABLE=$python")
    $preset = if ($Configuration -eq 'Debug') { 'windows-debug' } else { 'windows' }
    Invoke-Checked 'cmake.exe' @('--build', '--preset', $preset)
    Invoke-Checked 'ctest.exe' @('--preset', $preset)
    Write-Host "Ready: $root\build\windows\app\$Configuration\simplicity.exe"
    Write-Host 'This is currently a headless bootstrap, not a complete DAW.'
    exit 0
} catch {
    Write-Host "SETUP FAILED: $($_.Exception.Message)" -ForegroundColor Red
    if ($env:GITHUB_ACTIONS -eq 'true') { Write-Host "::error::SETUP FAILED: $($_.Exception.Message)" }
    Write-Host 'Fix the reported prerequisite and re-run setup.cmd. Troubleshooting: docs/windows-setup.md'
    exit 1
}
