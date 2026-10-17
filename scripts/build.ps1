# SPDX-License-Identifier: GPL-3.0-only
param(
    [Parameter(Mandatory=$true)][string]$Rom,
    [Parameter(Mandatory=$true)][string]$SdlDir,
    [Parameter(Mandatory=$true)][string]$ZlibSource,
    [string]$Python = 'python'
)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$repo = Join-Path $root 'work/cps2recomp'
function Run([string]$Tool,[string[]]$Arguments) {
    & $Tool @Arguments
    if ($LASTEXITCODE) { throw "$Tool failed with exit code $LASTEXITCODE" }
}
foreach ($tool in @('cl','cmake','ninja',$Python)) {
    if (!(Get-Command $tool -ErrorAction SilentlyContinue)) { throw "Missing $tool. Use Visual Studio Developer PowerShell with CMake, Ninja and Python." }
}
$Rom = (Resolve-Path -LiteralPath $Rom).Path
$SdlDir = (Resolve-Path -LiteralPath $SdlDir).Path
$ZlibSource = (Resolve-Path -LiteralPath $ZlibSource).Path
if (!(Test-Path -LiteralPath (Join-Path $repo 'port/themes.h'))) { throw 'Run bootstrap.ps1 first.' }
$sdlCmake = Join-Path $SdlDir 'cmake'
$dll = Join-Path $SdlDir 'lib/x64/SDL3.dll'
if (!(Test-Path -LiteralPath $dll)) { throw 'Use the SDL3 Visual C++ x64 development package.' }
$zbuild = Join-Path $root 'work/zlib-build'
Run cmake @('-S',$ZlibSource,'-B',$zbuild,'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DCMAKE_POLICY_VERSION_MINIMUM=3.5','-DZLIB_BUILD_EXAMPLES=OFF')
Run cmake @('--build',$zbuild,'--target','zlibstatic')
$zlib = Join-Path $zbuild 'zlibstatic.lib'
$zinclude = Join-Path $root 'work/zlib-include'
New-Item -ItemType Directory -Force $zinclude | Out-Null
Copy-Item -LiteralPath (Join-Path $ZlibSource 'zlib.h'),(Join-Path $zbuild 'zconf.h') -Destination $zinclude
$generated = Join-Path $root 'work/generated/sfz2al'
$gen = Join-Path $generated 'gen'
New-Item -ItemType Directory -Force $gen | Out-Null
Push-Location $repo
try { Run $Python @('tools/gamecfg.py','--header',(Join-Path $gen 'game_info.h'),'sfz2al') }
finally { Pop-Location }
$native = Join-Path $root 'work/game-build'
function Configure-Game {
    Run cmake @('-S',$repo,'-B',$native,'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DCMAKE_OBJECT_PATH_MAX=240',"-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=$native/bin",'-DGAME=sfz2al',"-DGAME_DIR=$repo/games/sfz2al","-DWORK_DIR=$generated","-DSDL3_DIR=$sdlCmake","-DZLIB_LIBRARY=$zlib","-DZLIB_INCLUDE_DIR=$zinclude",'-DSFZ2AL_LADDER_RESEARCH=OFF','-DCMAKE_CXX_FLAGS=/utf-8')
}
Configure-Game
Run cmake @('--build',$native,'--target','sfz2al','--parallel','4')
$game = Join-Path $native 'bin/sfz2al.exe'
Copy-Item -LiteralPath $dll -Destination (Split-Path $game)
Run $Python @('-c','import subprocess,sys; subprocess.run(sys.argv[1:],check=True)',$game,'--rom',$Rom,'--cpu','blocks','--sound','z80','--headless','--dump-program',(Join-Path $generated 'program.bin'),'--eeprom',(Join-Path $generated 'prepare.eeprom'))
Run $Python @((Join-Path $repo 'tools/recomp.py'),'--program',(Join-Path $generated 'program.bin'),'--entries',(Join-Path $root 'profiles/sfz2al/entries-main.txt'),'--out',$gen)
Run $Python @((Join-Path $repo 'tools/recompz80.py'),'--program',(Join-Path $generated 'program.bin.z80'),'--entries',(Join-Path $root 'profiles/sfz2al/entries-z80.txt'),'--out',$gen)
Configure-Game
Run cmake @('--build',$native,'--target','sfz2al','sfz2al-check','--parallel','4')
$ui = Join-Path $root 'build'
Run cmake @('-S',$root,'-B',$ui,'-G','Ninja','-DCMAKE_BUILD_TYPE=Release',"-DSDL3_DIR=$sdlCmake",'-DBUILD_TESTING=ON')
Run cmake @('--build',$ui,'--parallel','4')
Copy-Item -LiteralPath $dll -Destination $ui
Run ctest @('--test-dir',$ui,'--output-on-failure')
$dist = Join-Path $root 'dist'
New-Item -ItemType Directory -Force $dist,(Join-Path $dist 'licenses') | Out-Null
Copy-Item -LiteralPath (Join-Path $ui 'SFZ2AL.exe'),$dll -Destination $dist
Copy-Item -LiteralPath $game -Destination (Join-Path $dist 'sfz2al-game.exe')
if (!(Test-Path -LiteralPath (Join-Path $dist 'controles.ini'))) {
    Copy-Item -LiteralPath (Join-Path $root 'profiles/controles.ini') -Destination $dist
}
Copy-Item -LiteralPath (Join-Path $root 'LICENSE'),(Join-Path $root 'UPSTREAM_NOTICES.md') -Destination (Join-Path $dist 'licenses')
Copy-Item -LiteralPath (Join-Path $SdlDir 'LICENSE.txt') -Destination (Join-Path $dist 'licenses/SDL3.txt')
Copy-Item -LiteralPath (Join-Path $ZlibSource 'LICENSE') -Destination (Join-Path $dist 'licenses/zlib.txt')
Write-Host "Local build complete: $dist. Open SFZ2AL.exe and choose your own ROM ZIP."
Write-Host 'Keep dist/ and work/ private: they contain locally generated game code.'
