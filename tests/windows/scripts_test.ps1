# Runs with Windows PowerShell 5.1 too (setup.cmd's host), without Pester dependencies.
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
foreach ($file in @('tools/windows/setup.ps1', 'tools/windows/package.ps1', 'tests/windows/installer_test.ps1')) {
    $tokens, $errors = $null, $null
    [void][Management.Automation.Language.Parser]::ParseFile((Join-Path $root $file), [ref]$tokens, [ref]$errors)
    if ($errors.Count) { throw "$file syntax errors: $errors" }
}
# Parameter validation must propagate a nonzero result through the cmd launcher.
$ErrorActionPreference = 'Continue'
& "$root/setup.cmd" -Configuration Invalid 2>&1 | Out-String | Write-Host
$code = $LASTEXITCODE
$ErrorActionPreference = 'Stop'
if ($code -eq 0) { throw 'setup.cmd swallowed a failed setup exit code' }
# Exercise bootstrap helpers with controlled dependencies, without installing anything.
$ast = [Management.Automation.Language.Parser]::ParseFile((Join-Path $root 'tools/windows/setup.ps1'), [ref]$null, [ref]$null)
$functions = $ast.FindAll({ param($node) $node -is [Management.Automation.Language.FunctionDefinitionAst] }, $true)
foreach ($function in $functions) { Invoke-Expression $function.Extent.Text }
$NoInstall = $true
try { Install-Tool 'Missing.Test.Tool'; throw 'Expected NoInstall rejection' }
catch { if ($_.Exception.Message -notmatch 'Missing prerequisite: Missing.Test.Tool') { throw } }
try { Invoke-Checked 'cmd.exe' @('/c', 'exit', '37'); throw 'Expected native command failure' }
catch { if ($_.Exception.Message -notmatch 'exit 37') { throw } }
Invoke-Checked 'cmd.exe' @('/c', 'exit', '0')
Write-Host 'PASS: PowerShell 5.1 parsing, cmd failure propagation, NoInstall, native command success/failure.'
