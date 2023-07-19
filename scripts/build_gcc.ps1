[string] $type = $null
[string] $test = $false

foreach ( $arg in $args )
{
    if ( $arg -eq "test" )
    {
        $test = $true
    }
    else
    {
        $type = $arg
    }
}

$path_root            = git rev-parse --show-toplevel
$path_bootstrap       = Join-Path $path_root bootstrap
$path_bootstrap_build = Join-Path $path_bootstrap build_gcc
$path_project         = Join-Path $path_root project
$path_scripts         = Join-Path $path_root scripts
$path_test            = Join-Path $path_root test

write-host "`n`nBuilding genc via gcc`n"

if ( -not( Test-Path $path_bootstrap_build ) )
{
# Generate build files for meta-program
Push-Location $path_scripts
	$args_meson  = @()
	$args_meson += "setup"
	$args_meson += $path_bootstrap_build

	& meson $args_meson
Pop-Location
}

# Compile meta-program
Push-Location $path_root
	$args_ninja = @()
	$args_ninja += "-C"
	$args_ninja += $path_bootstrap_build

	& ninja $args_ninja
Pop-Location
