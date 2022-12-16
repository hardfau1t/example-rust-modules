#![allow(non_upper_case_globals)]

//! snull interface from linux device driver 3 book

use kernel::net::device::NetDevice;
use kernel::net::prelude::{NetDeviceAdapter, NetDeviceOps, EthToolOps};
use kernel::prelude::*;

module! {
    type: Snull,
    name: "snull",
    author: "hardfault",
    description: "snull driver from ldd3",
    license: "GPL",
}

struct Snull{
    devs: [NetDevice<SnullPriv>;2],
}
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
        todo!()
    }

    fn uninit(dev: &mut NetDevice<Self>) {
        todo!()
    }
    kernel::declare_net_device_ops!();
}
impl EthToolOps<SnullPriv> for SnullPriv{
    kernel::declare_eth_tool_ops!();
}

impl kernel::Module for Snull {
    fn init(_name: &'static CStr, _module: &'static ThisModule) -> Result<Self> {
        let dev0 = NetDevice::new(
            SnullPriv,
            kernel::c_str!("sn%d"),
            kernel::net::device::NetNameAssingType::Enum,
            1,
            1,
        )?;
        let dev1 = NetDevice::new(
            SnullPriv,
            kernel::c_str!("sn%d"),
            kernel::net::device::NetNameAssingType::Enum,
            1,
            1,
        )?;
        Ok(Self{
            devs: [dev0, dev1]
        })
    }
}
