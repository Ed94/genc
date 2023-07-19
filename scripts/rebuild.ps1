cls
Invoke-Expression "& $(Join-Path $PSScriptRoot 'clean.ps1') $args" 
Invoke-Expression "& $(Join-Path $PSScriptRoot 'build.ci.ps1') $args" 
