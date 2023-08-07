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
$path_bootstrap_build = Join-Path $path_bootstrap build_msvc
$path_project         = Join-Path $path_root project
$path_scripts         = Join-Path $path_root scripts
$path_test            = Join-Path $path_root test

write-host $path_bootstrap
write-host $path_bootstrap_build

$edition = "Professional"

$vcvarsall_2022 = "C:\Program Files\Microsoft Visual Studio\2022\"       + $edition + "\VC\Auxiliary\Build\vcvarsall.bat"
$vcvarsall_2019 = "C:\Program Files (x86)\Microsoft Visual Studio\2019\" + $edition + "\VC\Auxiliary\Build\vcvarsall.bat"
$vcvarsall_2017 = "C:\Program Files (x86)\Microsoft Visual Studio\2017\" + $edition + "\VC\Auxiliary\Build\vcvarsall.bat"
$vcvarsalls     = @($vcvarsall_2022, $vcvarsall_2019, $vcvarsall_2017)

write-host "`n`nBuilding genc via msvc`n"

$vcvarsalls | ForEach-Object {
    # Check if file exists
    if (Test-Path $_) {
        # Push location to return to it later
        Push-Location

        # Invoke vcvarsall.bat
        write-host "Running $_"
        try {
            cmd /c "`"$_`" x64 && set" |
                ForEach-Object {
                    if ($_ -match "^(.*?)=(.*)$") {
                        Set-Content "env:\$($matches[1])" $matches[2]
                    }
                }

            if ( -not( Test-Path $path_bootstrap_build ) ) {
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
        }
        finally {
            # Return to the original location
            Pop-Location
        }
    }
}
