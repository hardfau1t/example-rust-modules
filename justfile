alias cm := create-module
set positional-arguments
export ARCH := "arm"
export LLVM := "1"

kernel_dir := '../linux'

makeflags := '-C ' + kernel_dir

build module="gdev":
    make {{makeflags}} M="$PWD/{{module}}" modules

clean module="gdev":
	make {{makeflags}} M="$PWD/{{module}}" clean

rust-analyzer:
	make {{makeflags}} rust-analyzer

com dev="/dev/ttyUSB0":
    -picocom  {{dev}} -b 115200

# creates module structure with Kbuild file
# don't have spaces or special characters which will messup
@create-module module_name:
    echo 'creating module {{module_name}}'
    mkdir "{{module_name}}"
    echo "obj-m := {{module_name}}.o" > "{{module_name}}/Kbuild"
    echo 'use kernel::prelude::*;' > "{{module_name}}/{{module_name}}.rs"
