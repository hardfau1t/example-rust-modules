#![allow(non_upper_case_globals)]

//! snull interface from linux device driver 3 book

use kernel::net::device::NetDevice;
use kernel::net::prelude::{NetDeviceAdapter, NetDeviceOps, EthToolOps};
use kernel::net::rtnl_link_ops;
use kernel::prelude::*;
use kernel::SavedAsPointer;

module! {
    type: Snull,
    name: "snull",
    author: "hardfault",
    description: "snull driver from ldd3",
    license: "GPL",
}

fn setup(dev: &mut NetDevice<SnullPriv>) {
    dev.ether_setup();
    dev.set_ops();
}

rtnl_link_ops! {
    kind: b"snull",
    type: SnullPriv,
    setup: setup,
}

struct Snull;
struct SnullPriv;
impl NetDeviceAdapter for SnullPriv{
    type Inner = Self;

    type Ops = Self;

    type EthOps = Self;

    fn setup(dev: &mut NetDevice<Self>) {
        pr_info!("Snull Priv Setup");
    }
}

impl NetDeviceOps<SnullPriv> for SnullPriv{

    fn init(dev: &mut NetDevice<Self>) -> Result {
        Ok(())
    }

    fn uninit(dev: &mut NetDevice<Self>) {
    }
    kernel::declare_net_device_ops!();
}
impl EthToolOps<SnullPriv> for SnullPriv{
    kernel::declare_eth_tool_ops!();
}

impl kernel::Module for Snull {
    fn init(_name: &'static CStr, _module: &'static ThisModule) -> Result<Self> {
        unsafe{snull_LINK_OPS.register()};
        Ok(Self)
    }
}

impl Drop for Snull{
    fn drop(&mut self) {
        unsafe{snull_LINK_OPS.unregister()};
    }
}
