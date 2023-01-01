#![allow(non_upper_case_globals)]
#![allow(unused)]

//! snull interface from linux device driver 3 book

use core::slice;

use kernel::net::device::{Flag, NetdevTX};
use kernel::net::prelude::{EthToolOps, NetDevice, NetDeviceAdapter, NetDeviceOps, SkBuff};
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

fn print_ipv6(data: &[u8]) {
    if data.len() < 40 {
        pr_err!("ipv6 buffer with less than 40 bytes\n");
        return;
    }
    pr_info!("IPV6 packet:\n");
    pr_info!("\tversion: {:x?}\n", data[0] >> 4);
    pr_info!("\ttraffic class: {:x?}\n", (data[0] & ((1 << 4) - 1)) | (data[1]>>4));
    pr_info!("\tflow label: {:x?}\n", [data[1] & ((1<< 4)-1), data[2], data[3]]);
    pr_info!("\tpayload length: {:x?}\n", &data[4..6]);
    pr_info!("\tnext header: {:x?}\n", data[6]);
    pr_info!("\thop limit: {:x?}\n", data[7]);
    pr_info!("\tsource address: {:x?}\n", &data[8..24]);
    pr_info!("\tdestination address: {:x?}\n", &data[24..40]);
    pr_info!("\tdata: {:x?}\n", &data[40..]);
}

struct Snull {
    sn0: NetDevice<SnullPriv>,
    sn1: NetDevice<SnullPriv>,
}
struct SnullPriv;
impl NetDeviceAdapter for SnullPriv {
    type Inner = Self;

    type Ops = Self;

    type EthOps = Self;

    fn setup(dev: &mut NetDevice<Self>) {
        dev.ether_setup();
        dev.set_ops();
        dev.add_flag(Flag::NOARP); // snull doesn't supports ARP
        dev.remove_flag(Flag::MULTICAST);
        dev.hw_addr_random();
        // pr_info!("Snull setup done\n");
    }
}

impl NetDeviceOps<SnullPriv> for SnullPriv {
    fn init(dev: &mut NetDevice<Self>) -> Result {
        // pr_info!("Snull dev initialized\n");
        Ok(())
    }
    fn uninit(dev: &mut NetDevice<Self>) {
        // pr_info!("Snull Device is deinitialized\n");
    }

    fn start_xmit(skb: SkBuff, dev: &mut NetDevice<Self>) -> NetdevTX {
        let len = skb.len();
        let internal = skb.get_internal();
        let tail = internal.tail;
        let end = internal.end;
        let data_len = internal.data_len;
        let data: &[u8] = unsafe { slice::from_raw_parts(internal.data, len.try_into().unwrap()) };
        // pr_err!("Not Implement: start_xmit\n");
        // pr_info!("tail {tail:?}, end {end:?}, len {len:?}, data_len {data_len:?}\n");
        pr_info!("destination: {:x?}\n", &data[..6]);
        pr_info!("source: {:x?}\n", &data[6..12]);
        pr_info!("ethertype: {:x?}\n", &data[12..14]);
        if data[12] == 0x86 && data[13] == 0xdd{
        print_ipv6(&data[14..data.len() - 4]);
        }else{
            pr_info!("unknown packet: {:x?}\n", &data[14..data.len() - 4]);
        }
        pr_info!("crc: {:x?}\n", &data[data.len() - 4..]);
        drop(skb);
        NetdevTX::TX_OK
    }
    fn get_stats64(dev: &mut NetDevice<SnullPriv>, stats: &mut kernel::net::rtnl::RtnlLinkStats64) {
        pr_err!("Not Implement: get_stats");
    }

    fn change_carrier(dev: &mut NetDevice<SnullPriv>, new_carrier: bool) -> Result {
        pr_err!("Not Implemented: Carrier change request\n");
        core::result::Result::Err(kernel::error::code::EINVAL)
    }

    fn validate_addr(dev: &mut NetDevice<SnullPriv>) -> Result {
        pr_err!("Not Implemented: validate_addr\n");
        // core::result::Result::Err(kernel::error::code::EINVAL)
        Ok(())
    }

    fn set_mac_addr(dev: &mut NetDevice<SnullPriv>, p: *mut core::ffi::c_void) -> Result {
        pr_err!("Not Implemented: set_mac_addr\n");
        core::result::Result::Err(kernel::error::code::EINVAL)
    }

    fn set_rx_mode(dev: &mut NetDevice<SnullPriv>) {
        pr_err!("Not Implemented: set_rx_mode\n");
    }

    // kernel::declare_net_device_ops!(get_stats64, change_carrier, validate_addr, set_mac_addr, set_rx_mode);
    kernel::declare_net_device_ops!();
}
impl EthToolOps<SnullPriv> for SnullPriv {
    kernel::declare_eth_tool_ops!();
}

impl kernel::Module for Snull {
    fn init(_name: &'static CStr, _module: &'static ThisModule) -> Result<Self> {
        // pr_info!("Snull Init \n");
        // pr_info!("Setting up dev1\n");
        let mut dev1 = NetDevice::new(
            SnullPriv,
            kernel::c_str!("snull%d"),
            kernel::net::device::NetNameAssingType::Enum,
            1,
            1,
        )?;
        // pr_info!("Registering up dev1\n");
        if let Err(e) = dev1.register() {
            pr_err!("Failed to register snull0\n");
            return Err(e);
        }
        // pr_info!("Setting up dev2\n");
        let mut dev2 = NetDevice::new(
            SnullPriv,
            kernel::c_str!("snull%d"),
            kernel::net::device::NetNameAssingType::Enum,
            1,
            1,
        )?;
        // pr_info!("registering up dev2\n");
        if let Err(e) = dev2.register() {
            pr_err!("Failed to register snull1\n");
            return Err(e);
        }
        Ok(Self {
            sn0: dev1,
            sn1: dev2,
        })
    }
}

impl Drop for Snull {
    fn drop(&mut self) {}
}
