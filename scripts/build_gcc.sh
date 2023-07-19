#!/bin/bash

# This file has only been tested in an ubuntu WSL env and is not guaranteed to work in other environments

type=""
test=false

for arg in "$@"
do
	if [ "$arg" == "test" ]
	then
		test=true
	else
		type=$arg
	fi
done

path_root=$(git rev-parse --show-toplevel)
path_bootstrap="${path_root}/bootstrap"
path_bootstrap_build="${path_bootstrap}/build_gcc"
path_project="${path_root}/project"
path_scripts="${path_root}/scripts"
path_test="${path_root}/test"

echo $path_bootstrap
echo $path_bootstrap_build

echo -e "\n\nBuilding genc\n"

if [ ! -d "$path_bootstrap_build" ]
then
	# Generate build files for meta-program
	pushd $path_scripts
	meson setup $path_bootstrap_build
	popd
fi

# Compile meta-program
pushd $path_root
	ninja -C $path_bootstrap_build
popd
