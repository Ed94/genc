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

# Your paths
$path_root            = git rev-parse --show-toplevel
$path_bootstrap       = Join-Path $path_root bootstrap
$path_bootstrap_build = Join-Path $path_bootstrap build
$path_project         = Join-Path $path_root project
$path_scripts         = Join-Path $path_root scripts
$path_test            = Join-Path $path_root test

write-host $path_bootstrap
write-host $path_bootstrap_build

# Edition and VS paths

$edition = "Professional"

$vcvarsall_2022 = "C:\Program Files\Microsoft Visual Studio\2022\"       + $edition + "\VC\Auxiliary\Build\vcvarsall.bat"
$vcvarsall_2019 = "C:\Program Files (x86)\Microsoft Visual Studio\2019\" + $edition + "\VC\Auxiliary\Build\vcvarsall.bat"
$vcvarsall_2017 = "C:\Program Files (x86)\Microsoft Visual Studio\2017\" + $edition + "\VC\Auxiliary\Build\vcvarsall.bat"
$vcvarsalls     = @($vcvarsall_2022, $vcvarsall_2019, $vcvarsall_2017)

# For Cygwin
$cygwin = "C:\cygwin64\bin"
$env:Path = $env:Path + ";" + $cygwin

# Your compilers
$compilers = @{
	"clang" = @{
        "CC" = "clang"
        "CXX" = "clang++"
    }
    "gcc" = @{
        "CC" = "gcc"
        "CXX" = "g++"
    }
    "msvc" = @{
        "CC" = "cl"
        "CXX" = "cl"
    }
}

write-host "`n`genc`n"

# Save current environment variables
$originalEnv = Get-ChildItem Env: | Out-String

foreach ($compiler in $compilers.GetEnumerator()) {
    write-host "`nBuilding for  $($compiler.Name)`n"
    $env:CC  = $compiler.Value.CC
    $env:CXX = $compiler.Value.CXX

    $path_bootstrap_build = Join-Path $path_bootstrap "build_$($compiler.Name)"

    if ( $compiler.Name.Equals("msvc") ) {
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
						$args_meson += Join-Path $path_bootstrap build_msvc
						& meson $args_meson
						Pop-Location
					}
					# Compile meta-program
					Push-Location $path_root
					$args_ninja = @()
					$args_ninja += "-C"
					$args_ninja += Join-Path $path_bootstrap build_msvc
					& ninja $args_ninja
					Pop-Location
				}
				finally {
					# Return to the original location
					Pop-Location
				}
			}
		}
    }

	# Setup environment for gcc
	if ( $compiler.Name.Equals("gcc")) {
        if (!(Get-Command "gcc" -errorAction SilentlyContinue)) {
            Write-Host "`n`Error: GCC is not installed or not found in PATH."
            Write-Host "Please ensure you've installed Cygwin with GCC and added it to your system PATH.`n"
            continue
        }
	}

	# Clang works out of the box so no need to setup env.

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

	# Reset environment variables
	$originalEnv.Split("`n") | ForEach-Object {
		if ($_ -match "^(.*?)=(.*)$") {
			Set-Item -Path "Env:\$($matches[1])" -Value "$($matches[2])"
		}
	}
}
