$path_root            = git rev-parse --show-toplevel
$path_bootstrap       = Join-Path $path_root bootstrap
$path_bootstrap_build = Join-Path $path_bootstrap build
$path_project         = Join-Path $path_root project
$path_scripts         = Join-Path $path_root scripts
$path_test            = Join-Path $path_root test

$compilers = @("clang", "gcc", "msvc")

if ( Test-Path $path_bootstrap_build )
{
	Remove-Item $path_bootstrap_build -Recurse
}

$compilers | ForEach-Object {
	$build_path = Join-Path $path_bootstrap "build_$_"

	if ( Test-Path $build_path ) {
		Remove-Item $build_path -Recurse
	}
}

if ( $false ) {
if ( Test-Path $path_test_build )
{
	Remove-Item $path_test_build -Recurse
}
}

[string[]] $include = 'genc.h', 'genc.c',
	'genc.dep.h', 'genc.dep.c',
	'genc_builder.h', 'genc_builder.c',
	'genc_editor.h', 'genc_editor.c',
	'genc_scanner.h', 'genc_scanner.c'
[string[]] $exclude =

$files = Get-ChildItem -Recurse -Path $path_project -Include $include -Exclude $exclude

if ( $files )
{
	Remove-Item $files
}
