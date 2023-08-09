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
$path_bootstrap_build = Join-Path $path_bootstrap build
$path_project         = Join-Path $path_root project
$path_scripts         = Join-Path $path_root scripts
$path_test            = Join-Path $path_root test

write-host $path_boostrap
write-host $path_bootstrap_build

write-host "`n`nBuilding genc`n"

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

Push-location $path_project
# Run meta-program
	$genc = Join-Path $path_bootstrap_build genc.exe

	Write-Host `nRunning tests...
	& $genc

# Format generated files
	Write-Host `nBeginning format...
	$formatParams = @(
		'-i'          # In-place
		'-style=file' # Search for a .clang-format file in the parent directory of the source file.
		'-verbose'
	)

	$include = @('genc.h', 'genc.c',
		'genc.dep.h', 'genc.dep.c',
		'genc_builder.h', 'genc_builder.c',
		'genc_editor.h', 'genc_editor.c',
		'genc_scanner.h', 'genc_scanner.c')
	$exclude = $null

	$targetFiles = @(Get-ChildItem -Recurse -Path $path_gen -Include $include -Exclude $exclude | Select-Object -ExpandProperty FullName)

	clang-format $formatParams $targetFiles
	Write-Host "`nFormatting complete"
Pop-Location
