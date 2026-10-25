# Real Windows install/reinstall/uninstall + portable smoke test, not a mock installer.
[CmdletBinding()]
param([string]$PackageDir = 'build/packages')
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
trap {
    if ($env:GITHUB_ACTIONS -eq 'true') { Write-Host "::error::INSTALLER TEST FAILED: $($_.Exception.Message)" }
    throw $_
}
function Assert-True($Condition, [string]$Message) { if (-not $Condition) { throw $Message } }
function Run-Installer([string]$File, [string[]]$Arguments) {
    $process = Start-Process -FilePath $File -ArgumentList $Arguments -Wait -PassThru
    Assert-True ($process.ExitCode -eq 0) "Installer failed: $File ($($process.ExitCode))"
}
function Test-AppIcon([string]$Exe) {
    Add-Type -AssemblyName System.Drawing
    $icon = [Drawing.Icon]::ExtractAssociatedIcon($Exe)
    Assert-True ($null -ne $icon) 'Compiled application icon missing'
    $bitmap = $icon.ToBitmap()
    try {
        $grey = 0; $ink = 0
        for ($y = 0; $y -lt $bitmap.Height; $y++) {
            for ($x = 0; $x -lt $bitmap.Width; $x++) {
                $pixel = $bitmap.GetPixel($x, $y)
                if ($pixel.A -eq 0) { continue }
                Assert-True ($pixel.R -eq $pixel.G -and $pixel.G -eq $pixel.B) 'Executable icon must be grayscale'
                if ($pixel.R -eq 91) { $grey++ }
                if ($pixel.R -eq 238) { $ink++ }
            }
        }
        Assert-True ($grey -gt ($bitmap.Width * $bitmap.Height / 2) -and $ink -gt 10) 'Executable does not contain the generated s icon'
    } finally { $bitmap.Dispose(); $icon.Dispose() }
}
function Test-Bootstrap([string]$Exe) {
    $report = Join-Path ([IO.Path]::GetTempPath()) ('simplicity-gui-' + [guid]::NewGuid() + '.json')
    try {
        $process = Start-Process -FilePath $Exe -ArgumentList '--smoke-test', '--report', "`"$report`"" -PassThru
        if (-not $process.WaitForExit(60000)) { $process.Kill(); throw 'GUI smoke test timed out' }
        $process.Refresh()
        Assert-True ($process.ExitCode -eq 0) "GUI smoke test failed: $($process.ExitCode)"
        Assert-True (Test-Path $report) 'GUI report missing'
        $result = Get-Content $report -Raw | ConvertFrom-Json
        Assert-True ($result.ok -and $result.application -eq 'Simplicity GUI shell') "GUI checks failed: $($result.error)"
    } finally { if (Test-Path $report) { Remove-Item -LiteralPath $report } }
}
$packages = (Resolve-Path -LiteralPath $PackageDir).Path
$workspace = Join-Path ([IO.Path]::GetTempPath()) ('Simplicity test & spaces ' + [guid]::NewGuid())
$installed = Join-Path $workspace 'installed app'
$portable = Join-Path $workspace 'portable app'
$key = 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall\{A92718CD-EC5F-4B46-8DDC-39B8D853A2CC}_is1'
$shortcut = Join-Path ([Environment]::GetFolderPath('Programs')) 'Simplicity.lnk'
$installer = Join-Path $packages 'Simplicity-Setup.exe'
# Never replace/remove a real user's existing installation during a test run.
Assert-True (-not (Test-Path $key)) 'Lifecycle tests require a clean disposable user account: Simplicity is already installed'
Assert-True (-not (Test-Path $shortcut)) 'Lifecycle tests require no existing Simplicity Start menu shortcut'
try {
    New-Item -ItemType Directory -Force $workspace | Out-Null
    $checksums = @(Get-Content (Join-Path $packages 'SHA256SUMS.txt'))
    Assert-True ($checksums.Count -eq 2) 'Expected exactly two checksums'
    foreach ($line in $checksums) {
        Assert-True ($line -match '^([0-9a-f]{64})  (Simplicity-Setup.exe|Simplicity-Portable.zip)$') 'Malformed checksum manifest'
        $expected, $name = $Matches[1], $Matches[2]
        Assert-True ((Get-FileHash (Join-Path $packages $name) -Algorithm SHA256).Hash -eq $expected) "Checksum mismatch: $name"
    }
    # Verify the *compiled* installer requests normal-user privileges, not just its source.
    $sdk = "${env:ProgramFiles(x86)}\Windows Kits\10\bin"
    $mt = Get-ChildItem "$sdk/*/x64/mt.exe" | Sort-Object FullName -Descending | Select-Object -First 1
    Assert-True ($null -ne $mt) 'Windows SDK manifest tool missing'
    $manifest = Join-Path $workspace 'installer.manifest'
    & $mt.FullName "-inputresource:$installer;#1" "-out:$manifest"
    Assert-True ($LASTEXITCODE -eq 0) 'Cannot read installer manifest'
    Assert-True ((Get-Content $manifest -Raw) -match 'level="asInvoker"') 'Installer must not request elevation'
    Expand-Archive -LiteralPath (Join-Path $packages 'Simplicity-Portable.zip') -DestinationPath $portable
    foreach ($file in @('simplicity.exe', 'README.txt', 'notices/JUCE-LICENSE.md', 'notices/dependencies.md')) {
        Assert-True (Test-Path (Join-Path $portable $file)) "Missing portable file: $file"
    }
    Test-AppIcon (Join-Path $portable 'simplicity.exe')
    Test-Bootstrap (Join-Path $portable 'simplicity.exe')
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    $vs = & $vswhere -latest -products '*' -version '[17.0,18.0)' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    $dumpbin = Get-ChildItem "$vs/VC/Tools/MSVC/*/bin/Hostx64/x64/dumpbin.exe" | Sort-Object FullName -Descending | Select-Object -First 1
    Assert-True ($null -ne $dumpbin) 'dumpbin missing'
    $dependencies = & $dumpbin.FullName /dependents (Join-Path $portable 'simplicity.exe') | Out-String
    Assert-True ($LASTEXITCODE -eq 0) 'Cannot inspect executable dependencies'
    Assert-True ($dependencies -notmatch '(?i)(VCRUNTIME|MSVCP\d|MSVCR\d)') 'Executable needs a separately installed MSVC runtime'
    foreach ($attempt in 1..2) {
        Run-Installer $installer @('/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART', '/SP-', "/DIR=`"$installed`"")
        Test-AppIcon (Join-Path $installed 'simplicity.exe')
        Test-Bootstrap (Join-Path $installed 'simplicity.exe')
        Assert-True (Test-Path $key) 'Per-user uninstaller registration missing'
        Assert-True (Test-Path $shortcut) 'Start menu shortcut missing'
        $link = (New-Object -ComObject WScript.Shell).CreateShortcut($shortcut)
        Assert-True ($link.TargetPath -eq (Join-Path $installed 'simplicity.exe')) 'Shortcut must launch the native GUI directly'
        Assert-True ((Get-ItemProperty $key).InstallLocation.TrimEnd('\') -eq $installed) 'Wrong install location'
        $legacy = Join-Path $installed 'Launch Simplicity.cmd'
        if ($attempt -eq 1) { Set-Content -LiteralPath $legacy -Value '@echo legacy wrapper' -Encoding ascii }
        else { Assert-True (-not (Test-Path $legacy)) 'Upgrade left the obsolete console wrapper behind' }
    }
    Run-Installer (Join-Path $installed 'unins000.exe') @('/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART')
    Assert-True (-not (Test-Path (Join-Path $installed 'simplicity.exe'))) 'Executable left behind after uninstall'
    Assert-True (-not (Test-Path $key)) 'Uninstall registry entry left behind'
    Assert-True (-not (Test-Path $shortcut)) 'Start menu shortcut left behind'
    Test-Bootstrap (Join-Path $portable 'simplicity.exe') # installer removal must not affect portable copy
    Write-Host 'PASS: compiled grayscale app icon, checksums, asInvoker, static runtime, portable launch, per-user install/reinstall, uninstall.'
} finally {
    if (Test-Path (Join-Path $installed 'unins000.exe')) {
        Run-Installer (Join-Path $installed 'unins000.exe') @('/VERYSILENT', '/SUPPRESSMSGBOXES', '/NORESTART')
    }
    if (Test-Path $workspace) { Remove-Item -LiteralPath $workspace -Recurse -Force }
}
