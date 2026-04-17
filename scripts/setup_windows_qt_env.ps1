param(
    [string]$QtRoot = "C:\Qt",
    [string]$QtVersion = "6.11.0",
    [string]$MingwToolsDirName = "mingw1310_64",
    [string]$NinjaDirName = "Ninja",
    [ValidateSet("Process", "User")]
    [string]$Scope = "User"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Convert-ToForwardSlashPath {
    param([string]$Path)

    return $Path.Replace('\', '/')
}

function Set-ScopedEnvironmentVariable {
    param(
        [Parameter(Mandatory)]
        [string]$Name,
        [Parameter(Mandatory)]
        [string]$Value,
        [Parameter(Mandatory)]
        [string]$Scope
    )

    [Environment]::SetEnvironmentVariable($Name, $Value, "Process")
    if ($Scope -eq "User") {
        [Environment]::SetEnvironmentVariable($Name, $Value, "User")
    }
}

function Register-EnvPath {
    param(
        [Parameter(Mandatory)]
        [hashtable]$Values,
        [Parameter(Mandatory)]
        [string]$Name,
        [Parameter(Mandatory)]
        [string]$CandidatePath,
        [Parameter(Mandatory)]
        [string]$Description
    )

    if (Test-Path -LiteralPath $CandidatePath) {
        $Values[$Name] = Convert-ToForwardSlashPath ((Resolve-Path -LiteralPath $CandidatePath).Path)
        return
    }

    Write-Warning "$Description was not found: $CandidatePath"
}

$qtRootResolved = (Resolve-Path -LiteralPath $QtRoot).Path
$qtVersionRoot = Join-Path $qtRootResolved $QtVersion

$values = [ordered]@{}

Register-EnvPath -Values $values `
    -Name "QT_WIN_MINGW_ROOT" `
    -CandidatePath (Join-Path $qtVersionRoot "mingw_64") `
    -Description "Qt MinGW kit"

Register-EnvPath -Values $values `
    -Name "QT_WIN_MSVC_ROOT" `
    -CandidatePath (Join-Path $qtVersionRoot "msvc2022_64") `
    -Description "Qt MSVC kit"

Register-EnvPath -Values $values `
    -Name "QT_WIN_MINGW_TOOLS_ROOT" `
    -CandidatePath (Join-Path $qtRootResolved "Tools\$MingwToolsDirName") `
    -Description "MinGW tools"

Register-EnvPath -Values $values `
    -Name "QT_WIN_NINJA_ROOT" `
    -CandidatePath (Join-Path $qtRootResolved "Tools\$NinjaDirName") `
    -Description "Ninja tools"

if ($values.Count -eq 0) {
    throw "No supported Qt toolchain paths were found under $qtRootResolved."
}

foreach ($entry in $values.GetEnumerator()) {
    Set-ScopedEnvironmentVariable -Name $entry.Key -Value $entry.Value -Scope $Scope
    Write-Host "$($entry.Key)=$($entry.Value)"
}

Write-Host ""
if ($Scope -eq "User") {
    Write-Host "Windows Qt preset environment variables were saved for the current user."
    Write-Host "Restart Qt Creator, terminals, and Developer PowerShell sessions to pick up the new values."
} else {
    Write-Host "Windows Qt preset environment variables were applied to the current PowerShell process."
}
