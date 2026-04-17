param(
    [string]$Preset = "windows-mingw-debug",
    [int]$Jobs = 0,
    [switch]$SkipTests,
    [switch]$Install,
    [switch]$Fresh
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path

function Invoke-Step {
    param(
        [Parameter(Mandatory)]
        [string]$Executable,
        [Parameter(Mandatory)]
        [string[]]$Arguments
    )

    Write-Host ""
    Write-Host "> $Executable $($Arguments -join ' ')"
    & $Executable @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code ${LASTEXITCODE}: $Executable $($Arguments -join ' ')"
    }
}

function Resolve-Executable {
    param(
        [Parameter(Mandatory)]
        [string]$CommandName,
        [string[]]$Fallbacks = @()
    )

    $command = Get-Command $CommandName -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    foreach ($candidate in $Fallbacks) {
        if (Test-Path -LiteralPath $candidate) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }

    throw "Unable to find $CommandName. Add it to PATH or install the matching tool."
}

function Ensure-WindowsPresetEnvironment {
    param([string]$PresetName)

    $requiredVariables = @(switch -Wildcard ($PresetName) {
        "windows-mingw-*" { @("QT_WIN_MINGW_ROOT", "QT_WIN_MINGW_TOOLS_ROOT", "QT_WIN_NINJA_ROOT") }
        "windows-msvc-*" { @("QT_WIN_MSVC_ROOT") }
        default { @() }
    })

    if ($requiredVariables.Count -eq 0) {
        return
    }

    $missingBeforeSetup = @(
        $requiredVariables | Where-Object {
            -not [Environment]::GetEnvironmentVariable($_, "Process")
        }
    )
    if ($missingBeforeSetup.Count -gt 0) {
        & (Join-Path $PSScriptRoot "setup_windows_qt_env.ps1") -Scope Process
    }

    $missingAfterSetup = @(
        $requiredVariables | Where-Object {
            -not [Environment]::GetEnvironmentVariable($_, "Process")
        }
    )
    if ($missingAfterSetup.Count -gt 0) {
        throw "Missing preset environment variables for ${PresetName}: $($missingAfterSetup -join ', ')"
    }
}

function Get-InstallArguments {
    param(
        [string]$PresetName,
        [string]$RepositoryRoot
    )

    $buildTree = Join-Path $RepositoryRoot "build\$PresetName"
    $arguments = @("--install", $buildTree)

    switch ($PresetName) {
        "windows-msvc-debug" {
            $arguments += @("--config", "Debug")
        }
        "windows-msvc-release" {
            $arguments += @("--config", "Release")
        }
    }

    return $arguments
}

$cmakeExe = Resolve-Executable -CommandName "cmake" -Fallbacks @(
    "C:\Qt\Tools\CMake_64\bin\cmake.exe"
)

$ctestExe = Resolve-Executable -CommandName "ctest" -Fallbacks @(
    (Join-Path (Split-Path -Parent $cmakeExe) "ctest.exe")
)

if ($Preset -like "windows-*") {
    Ensure-WindowsPresetEnvironment -PresetName $Preset
}

Push-Location $repoRoot
try {
    if ($Fresh) {
        $buildTree = Join-Path $repoRoot "build\$Preset"
        if (Test-Path -LiteralPath $buildTree) {
            Write-Host ""
            Write-Host "> Remove-Item -LiteralPath $buildTree -Recurse -Force"
            Remove-Item -LiteralPath $buildTree -Recurse -Force
        }
    }

    Invoke-Step -Executable $cmakeExe -Arguments @("--preset", $Preset)

    $buildArguments = @("--build", "--preset", $Preset)
    if ($Jobs -gt 0) {
        $buildArguments += @("--parallel", $Jobs.ToString())
    } else {
        $buildArguments += "--parallel"
    }

    Invoke-Step -Executable $cmakeExe -Arguments $buildArguments

    if (-not $SkipTests) {
        Invoke-Step -Executable $ctestExe -Arguments @("--preset", $Preset)
    }

    if ($Install) {
        Invoke-Step -Executable $cmakeExe -Arguments (Get-InstallArguments -PresetName $Preset -RepositoryRoot $repoRoot)
    }
} finally {
    Pop-Location
}
