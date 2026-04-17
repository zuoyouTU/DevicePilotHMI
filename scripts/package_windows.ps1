param(
    [ValidateSet("windows-mingw-release", "windows-msvc-release")]
    [string]$Preset = "windows-mingw-release",
    [int]$Jobs = 0,
    [switch]$SkipBuild
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$buildScript = Join-Path $PSScriptRoot "build.ps1"

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

function Resolve-CMake {
    $command = Get-Command cmake -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    $fallback = "C:\Qt\Tools\CMake_64\bin\cmake.exe"
    if (Test-Path -LiteralPath $fallback) {
        return (Resolve-Path -LiteralPath $fallback).Path
    }

    throw "Unable to find cmake."
}

function Get-InstallArguments {
    param(
        [string]$PresetName,
        [string]$RepositoryRoot
    )

    $buildTree = Join-Path $RepositoryRoot "build\$PresetName"
    $arguments = @("--install", $buildTree)

    switch ($PresetName) {
        "windows-msvc-release" {
            $arguments += @("--config", "Release")
        }
    }

    return $arguments
}

Push-Location $repoRoot
try {
    if ($SkipBuild) {
        $cmakeExe = Resolve-CMake
        Invoke-Step -Executable $cmakeExe -Arguments (Get-InstallArguments -PresetName $Preset -RepositoryRoot $repoRoot)
    } else {
        & $buildScript -Preset $Preset -Jobs $Jobs -SkipTests -Install
        if ($LASTEXITCODE -ne 0) {
            throw "Packaging build step failed for $Preset."
        }
    }

    $stageDir = Join-Path $repoRoot "build\$Preset\stage"
    if (-not (Test-Path -LiteralPath $stageDir)) {
        throw "Install stage directory was not created: $stageDir"
    }

    $artifactSuffix = switch ($Preset) {
        "windows-mingw-release" { "windows-mingw-x64" }
        "windows-msvc-release" { "windows-msvc-x64" }
    }

    $releaseRoot = Join-Path $repoRoot "release"
    $packageDir = Join-Path $releaseRoot "DevicePilotHMI-$artifactSuffix"
    $zipPath = Join-Path $releaseRoot "DevicePilotHMI-$artifactSuffix.zip"

    if (Test-Path -LiteralPath $packageDir) {
        Remove-Item -LiteralPath $packageDir -Recurse -Force
    }
    if (Test-Path -LiteralPath $zipPath) {
        Remove-Item -LiteralPath $zipPath -Force
    }

    New-Item -ItemType Directory -Path $packageDir -Force | Out-Null
    Copy-Item -Path (Join-Path $stageDir "*") -Destination $packageDir -Recurse -Force

    Remove-Item -LiteralPath (Join-Path $packageDir "qmltooling") -Recurse -Force -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath (Join-Path $packageDir "translations") -Recurse -Force -ErrorAction SilentlyContinue

    Compress-Archive -Path (Join-Path $packageDir "*") -DestinationPath $zipPath -Force

    Write-Host ""
    Write-Host "Created package: $zipPath"
} finally {
    Pop-Location
}
