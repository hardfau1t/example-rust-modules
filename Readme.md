# Rust Modules
This project contains simple example modules written in rust for linux kernel. Currently it uses [rust-for-linux](https://github.com/Rust-for-Linux/linux) kernel repo.

## Setup
- TODO

## Building
For building module use [just](https://github.com/casey/just) command, if you don't have just installed then you can build each module by cd into that directory and run `make -C <kernel dir> ARCH=arm LLVM=1 M=$PWD [modules|clean]`

If you have [just](https://github.com/casey/just) installed
- To build specific module `just <module>`
- clean `just clean`
- `just -l` will list all the available options

**NOTE**: You may need to edit justfile and set kernel directory and other parameters according to your need
