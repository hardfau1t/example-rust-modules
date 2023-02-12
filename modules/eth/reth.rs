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
        pdev: &mut platform::Device,
        _id_info: Option<&Self::IdInfo>,
    ) -> Result<Box<CpswPlatData>> {
        pdev.pr_info(format_args!("probing cpsw device"));
        let mut data = Box::try_new(CpswPlatData { cpsw: binds::cpsw_common::default() })?;
        data.cpsw.dev = pdev.raw_device();
        let mut ss_res: *mut kernel::bindings::resource = core::ptr::null_mut();
        data.cpsw.regs = pdev.devm_platform_get_and_ioremap_resource(0, &mut ss_res)?
            as *mut binds::cpsw_ss_regs;
        data.cpsw.irqs_table[0] = pdev.platform_get_irq_byname(c_str!("rx"))? as u32;
        data.cpsw.irqs_table[1] = pdev.platform_get_irq_byname(c_str!("tx"))? as u32;
        data.cpsw.misc_irq = pdev.platform_get_irq_byname(c_str!("misc"))?;
        let ret = unsafe { binds::cpsw_probe(&mut data.cpsw as *mut _, ss_res) };
        if ret == 0 {
            pdev.pr_info(format_args!("probing cpsw successfull"));
            Ok(data)
        } else {
            Err(kernel::error::code::EAGAIN)
        }
    }

    fn remove(data: &Self::Data) -> Result {
        pr_info!("platform driver removal requested\n");
        unsafe { binds::cpsw_remove(&data.cpsw as *const _) };
        core::result::Result::Ok(())
    }

    type Data = Box<CpswPlatData>;

    type IdInfo = ();
}
