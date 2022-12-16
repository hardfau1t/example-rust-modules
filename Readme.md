# Rust Modules
This project contains simple example modules written in rust for linux kernel. Currently it uses [rust-for-linux](https://github.com/Rust-for-Linux/linux) kernel repo.

## Setup
TODO: Default it is built for beaglebone, later make it generic
- run `just init` this will do the necessory steps like pulling kernel applying patches

or to make it manually
### manually setup
- `git clone https://github.com/Rust-for-Linux/linux.git --depth 1 linux`
- `cd linux `
- `git checkout bd1234712693` #patches provided are applicable to this commit only. until [initial netdev support](https://github.com/Rust-for-Linux/linux/pull/908) or [draf:netdev wrapper](https://github.com/Rust-for-Linux/linux/pull/439) gets merged, it has to be done this way only.
- `git am ../patches/*`
- `rustup override set $(scripts/min-tool-version.sh rustc)` follow [these](https://github.com/Rust-for-Linux/linux/blob/rust/Documentation/rust/quick-start.rst) steps to exactly replicate this env setup
- `cargo install --locked --version $(scripts/min-tool-version.sh bindgen) bindgen`
- `make LLVM=1 ARCH=arm allnoconfig`
- `make LLVM=1 ARCH=arm multi_v7_defconfig`
- `make LLVM=1 ARCH=arm rustavailable`
- `make LLVM=1 ARCH=arm rust.config`
- `make -j $(nproc)`


## Building
For building module use [just](https://github.com/casey/just) command, if you don't have just installed then you can build each module by cd into that directory and run `make -C <kernel dir> ARCH=arm LLVM=1 M=$PWD [modules|clean]`

If you have [just](https://github.com/casey/just) installed
- To build specific module `just <module>`
- clean `just clean`
- `just -l` will list all the available options

**NOTE**: You may need to edit justfile and set kernel directory and other parameters according to your need
