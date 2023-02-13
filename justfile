set positional-arguments
alias cm := create-module
alias flash := flash-root-fs
export ARCH := "arm"
export LLVM := "1"

kernel_dir := './linux'

makeflags := '-C ' + kernel_dir

# builds specific module <name of the directory>
build module="gdev":
    compiledb make {{makeflags}} M="$PWD/modules/{{module}}" modules

clean module="gdev":
	make {{makeflags}} M="$PWD/modules/{{module}}" clean

rust-analyzer:
	make {{makeflags}} rust-analyzer

# setup rust-analyzer for given external module
mod-rs module:
    make {{makeflags}} M="$PWD/modules/{{module}}" rust-analyzer

com dev="/dev/ttyUSB0":
    -picocom  {{dev}} -b 115200

# creates module structure with Kbuild file
# don't have spaces or special characters which will messup
@create-module module_name:
    echo 'creating module {{module_name}}'
    mkdir "modules/{{module_name}}"
    echo "obj-m := {{module_name}}.o" > "modules/{{module_name}}/Kbuild"
    echo 'use kernel::prelude::*;' > "modules/{{module_name}}/{{module_name}}.rs"

# send the given module to through zmodem, Default: /dev/ttyUSB0
@send module tty="/dev/ttyUSB0" :
    # sz -y --zmodem "modules/{{module}}/{{module}}.ko" 1> {{tty}} 0< {{tty}}
    # md5sum "modules/{{module}}/{{module}}.ko"
    scp "modules/{{module}}/{{module}}.ko" beagle:{{module}}.ko
# 
# qemu:
#     qemu-system-arm -nographic -kernel linux/vmlinux -initrd busybox/initrd.img -nic user,model=rtl8139,hostfwd=tcp::5555-:23 -machine virt

kernel:
    make -j $(expr $(nproc) - 1) {{makeflags}}
# 
# qemu-config:
#     cd linux && make ARCH=arm LLVM=1 allnoconfig qemu-busybox-min.config rust.config rustavailable

beagle-config:
    make  {{makeflags}} allnoconfig multi_v7_defconfig rustavailable rust.config

install-modules:
    # sudo -E make {{makeflags}} INSTALL_MOD_PATH=/mnt/ modules_install
    mkdir -p /tmp/mods/
    make {{makeflags}} INSTALL_MOD_PATH=/tmp/mods modules_install
    ssh -t beagle "bash -c 'rm -rf ~/mods/*'"
    cd /tmp/mods/lib/modules&& tar -cvzf mods.tar.gz * && scp mods.tar.gz beagle:mods
    rm -rf /tmp/mods
    ssh -t beagle 'bash -c "rm -rf /lib/modules/*"'
    ssh -t beagle 'bash -c "tar -C /lib/modules/ -xvf mods"'
    ssh -t beagle 'bash -c "rm -rf /lib/modules/*/kernel/drivers/net/ethernet/ti"'

install-kernel:
    # sudo cp ./linux/arch/arm/boot/{zImage,dts/am335x-boneblack.dtb} /mnt/boot
    ssh -t beagle "bash -c 'rm -rf ~/boot/*'"
    scp ./linux/arch/arm/boot/zImage beagle:/boot/vmlinuz-6.1.0-rc1+
    scp ./linux/arch/arm/boot/dts/am335x-boneblack.dtb beagle:/boot/


install-tools:
    sudo cp ./linux/tools/gpio/{gpio-event-mon,gpio-hammer,gpio-watch,lsgpio} /mnt/bin

install: install-kernel install-modules

flash-root-fs file="../poky/build/tmp/deploy/images/beaglebone-yocto/core-image-minimal-beaglebone-yocto.tar.bz2":
    sudo tar -C /mnt -xvf "{{file}}"

mount drive="/dev/sdb2":
    sudo mount {{drive}} /mnt

umount :
    sudo umount /mnt

# initialize whole environment for running
init:
    git clone https://github.com/Rust-for-Linux/linux.git --depth 1 linux
    git -C linux checkout bd1234712693
    cd linux && git am ../patches/*
    cd linux && rustup override set $(scripts/min-tool-version.sh rustc) && rustup component add rust-src
    cd linux && cargo install --locked --version $(scripts/min-tool-version.sh bindgen) bindgen
    just beagle-config
    just kernel
