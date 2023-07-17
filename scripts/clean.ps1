$path_root            = git rev-parse --show-toplevel
$path_bootstrap       = Join-Path $path_root bootstrap
$path_bootstrap_build = Join-Path $path_bootstrap build
$path_project         = Join-Path $path_root project
$path_scripts         = Join-Path $path_root scripts
$path_test            = Join-Path $path_root test

if ( Test-Path $path_bootstrap_build )
{
	Remove-Item $path_bootstrap_build -Recurse
}

if ( $false ) {
if ( Test-Path $path_test_build )
{
	Remove-Item $path_test_build -Recurse
}
}

[string[]] $include = 'genc.h', 'genc.c'
[string[]] $exclude =

$files = Get-ChildItem -Recurse -Path $path_project -Include $include -Exclude $exclude

if ( $files )
{
	Remove-Item $files
}
