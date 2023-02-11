// SPDX-License-Identifier: GPL-2.0

//! Broadcom BCM2835 Random Number Generator support.

use core::marker::Sync;

use kernel::{driver::DeviceRemoval, module_platform_driver, of, platform, prelude::*, sync::Arc};

module_platform_driver! {
    type: CpswDriver,
    name: "TI_Cpsw_driver",
    author: "hardfault",
    description: "ti cpsw driver in rust",
    license: "GPL v2",
}

extern "C" {
    fn cpsw_probe(ptr: *mut kernel::bindings::platform_device) -> i32;
    fn cpsw_remove(ptr: *mut kernel::bindings::platform_device) -> i32;
}

/// cpsw platform data
struct CpswPlatData {
    pdev: *mut kernel::bindings::platform_device,
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
    ) -> Result<Arc<CpswPlatData>> {
        unsafe { cpsw_probe(dev.inne()) };
        Arc::try_new(CpswPlatData { pdev: dev.inne() })
    }

    fn remove(data: &Self::Data) -> Result {
        pr_info!("platform driver removal requested\n");
        // unsafe { cpsw_remove(data.pdev) };
        core::result::Result::Ok(())
    }

    type Data = Arc<CpswPlatData>;

    type IdInfo = ();
}
