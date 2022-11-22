export ARCH := "arm"
export LLVM := "1"

kernel_dir := '../linux'

makeflags := '-C ' + kernel_dir

all: gdev

gdev:
    make {{makeflags}} M=$PWD/gdev modules

gdev-clean:
	make {{makeflags}} M=$PWD/gdev clean

clean: gdev-clean

rust-analyzer:
	make {{makeflags}} rust-analyzer
