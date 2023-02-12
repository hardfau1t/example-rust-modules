// SPDX-License-Identifier: GPL-2.0

//! Broadcom BCM2835 Random Number Generator support.
use core::marker::{Send, Sync};

use kernel::{
    c_str, device::RawDevice, driver::DeviceRemoval, module_platform_driver, of, platform,
    prelude::*,
};

mod binds;

module_platform_driver! {
    type: CpswDriver,
    name: "TI_Cpsw_driver",
    author: "hardfault",
    description: "ti cpsw driver in rust",
    license: "GPL v2",
}

/// cpsw platform data
struct CpswPlatData {
    cpsw: binds::cpsw_common,
}

impl DeviceRemoval for CpswPlatData {
    fn device_remove(&self) {
        pr_info!("Data removal requested\n");
    }
}

unsafe impl Sync for CpswPlatData {}
unsafe impl Send for CpswPlatData {}

struct CpswDriver;
impl platform::Driver for CpswDriver {
    kernel::define_of_id_table! {(), [
            (of::DeviceId::Compatible(b"ti,cpsw-switch"), None),
    ]}
    fn probe(
        dev: &mut platform::Device,
        _id_info: Option<&Self::IdInfo>,
    ) -> Result<Box<CpswPlatData>> {
        dev.pr_info(format_args!("probing cpsw device"));
        let mut cpsw = binds::cpsw_common::default();
        cpsw.dev = dev.raw_device();
        cpsw.irqs_table[0] = dev.platform_get_irq_byname(c_str!("rx")) as u32;
        Ok(Box::try_new(CpswPlatData { cpsw })?)
    }

    fn remove(_data: &Self::Data) -> Result {
        pr_info!("platform driver removal requested\n");
        // unsafe { cpsw_remove(data.pdev) };
        core::result::Result::Ok(())
    }

    type Data = Box<CpswPlatData>;

    type IdInfo = ();
}
