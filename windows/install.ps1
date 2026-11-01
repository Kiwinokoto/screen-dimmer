# Installs a portable ScreenDimmer.exe for the current user only (no elevation).
param([switch]$NoAutostart)

$ErrorActionPreference = 'Stop'
$source = Join-Path $PSScriptRoot 'ScreenDimmer.exe'
if (-not (Test-Path -LiteralPath $source -PathType Leaf)) {
    throw 'Placez ScreenDimmer.exe dans le meme dossier que install.ps1.'
}

if (Get-Process -Name ScreenDimmer -ErrorAction SilentlyContinue) {
    throw 'Quittez Screen Dimmer depuis son icone pres de l horloge avant l installation.'
}

$installDir = Join-Path $env:LOCALAPPDATA 'Programs\ScreenDimmer'
New-Item -ItemType Directory -Path $installDir -Force | Out-Null
$target = Join-Path $installDir 'ScreenDimmer.exe'
Copy-Item -LiteralPath $source -Destination $target -Force

$shell = New-Object -ComObject WScript.Shell
function Set-Shortcut([string]$path, [string]$arguments, [string]$description) {
    $shortcut = $shell.CreateShortcut($path)
    $shortcut.TargetPath = $target
    $shortcut.Arguments = $arguments
    $shortcut.WorkingDirectory = $installDir
    $shortcut.Description = $description
    $shortcut.IconLocation = $target
    $shortcut.Save()
}

$desktop = [System.Environment]::GetFolderPath([System.Environment+SpecialFolder]::DesktopDirectory)
$programs = [System.Environment]::GetFolderPath([System.Environment+SpecialFolder]::Programs)
$startup = [System.Environment]::GetFolderPath([System.Environment+SpecialFolder]::Startup)

Set-Shortcut (Join-Path $desktop 'Screen Dimmer.lnk') '--toggle' 'Basculer le filtre de luminosite'
Set-Shortcut (Join-Path $programs 'Screen Dimmer.lnk') '--ui' 'Regler Screen Dimmer'
if (-not $NoAutostart) {
    Set-Shortcut (Join-Path $startup 'Screen Dimmer.lnk') '--startup' 'Reinitialiser le filtre et demarrer dans la barre des taches'
}

Write-Host "Screen Dimmer installe : $target"
Write-Host 'Bureau : double-clic pour activer/desactiver.'
Write-Host 'Zone de notification : clic pour regler, clic droit pour quitter.'
if ($NoAutostart) { Write-Host 'Demarrage automatique non modifie.' }
