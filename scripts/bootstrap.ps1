# SPDX-License-Identifier: GPL-3.0-only
param([string]$ToolkitSource = 'https://github.com/tmcleroy/cps2recomp.git')
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$repo = Join-Path $root 'work/cps2recomp'
$revision = 'fdd55ecbd20c9197c85ef526ecebd44879ced82e'
if (Test-Path -LiteralPath $repo) {
    throw 'work/cps2recomp already exists. Bootstrap never resets existing work.'
}
New-Item -ItemType Directory -Force (Join-Path $root 'work') | Out-Null
& git clone --no-checkout -- $ToolkitSource $repo
if ($LASTEXITCODE) { throw 'Could not clone the toolkit.' }
& git -C $repo checkout --detach $revision
if ($LASTEXITCODE) { throw 'The pinned toolkit revision is unavailable.' }
$patch = Join-Path $root 'patches/cps2recomp.patch'
& git -C $repo apply --check $patch
if ($LASTEXITCODE) { throw 'The source patch does not match the pinned toolkit.' }
& git -C $repo apply $patch
if ($LASTEXITCODE) { throw 'Could not apply the source patch.' }
foreach ($gameId in @('sfz2al','sfz2alb')) {
    Copy-Item -LiteralPath (Join-Path $root "profiles/$gameId") -Destination (Join-Path $repo 'games') -Recurse
}
Write-Host 'Toolkit and source profile ready. No game data was downloaded.'
