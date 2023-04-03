//! ethernet driver written in rust for TI's  CPSW for beaglebone black
#![deny(warnings)]
use kernel::{
    c_str, device::RawDevice, driver::DeviceRemoval, module_platform_driver, of, platform,
    prelude::*, ForeignOwnable,
};

mod net;

module_platform_driver! {
    type: CpswPlatform,
    name: "TI_Cpsw_driver",
    author: "hardfault",
    description: "ti cpsw driver in rust",
    license: "GPL v2",
}

struct CpswOperations;
struct CpswData;

impl ForeignOwnable for CpswData{
    type Borrowed<'a> = CpswData;

    fn into_foreign(self) -> *const core::ffi::c_void {
        &self as *const Self as *const _
    }

    unsafe fn borrow<'a>(_ptr: *const core::ffi::c_void) -> Self::Borrowed<'a> {
        todo!()
    }

    unsafe fn from_foreign(_ptr: *const core::ffi::c_void) -> Self {
        todo!()
    }
}

#[vtable]
impl net::DeviceOperations for CpswOperations {
    type Data = CpswData;

    fn open(
        _dev: &net::Device,
        _data: <Self::Data as kernel::ForeignOwnable>::Borrowed<'_>,
    ) -> Result {
        pr_info!("Netdev opened\n");
        Ok(())
    }

    fn stop(
        _dev: &net::Device,
        _data: <Self::Data as kernel::ForeignOwnable>::Borrowed<'_>,
    ) -> Result {
        pr_info!("Netdev stopped\n");
        Ok(())
    }

    fn start_xmit(
        _skb: &kernel::net::SkBuff,
        _dev: &net::Device,
        _data: <Self::Data as kernel::ForeignOwnable>::Borrowed<'_>,
    ) -> net::NetdevTx {
        pr_info!("xmit called\n");
        net::NetdevTx::Ok
    }
}

extern "C" fn setup(_dev: *mut kernel::bindings::net_device){

}

struct CpswPlatform;
struct CpswPlatData;

impl platform::Driver for CpswPlatform {
    kernel::define_of_id_table! {(), [
            (of::DeviceId::Compatible(b"ti,cpsw-switch"), None),
    ]}
    fn probe(
        pdev: &mut platform::Device,
        _id_info: Option<&Self::IdInfo>,
    ) -> Result<Box<CpswPlatData>> {
        pdev.pr_info(format_args!("probing cpsw device"));
        let mut ndev = net::Device::new::<CpswData>(c_str!("cpsw%d"), setup, 1, 1)?;
        net::Registration::<CpswOperations>::register(&mut ndev, CpswData)?;
        // let mut reg = net::Registration::<CpswOperations>::try_new(&ndev)?;
        // reg.register(())?;
        Ok(Box::try_new(CpswPlatData)?)
    }
    fn remove(_cpsw: &Self::Data) -> Result {
        pr_info!("platform driver removal requested\n");
        core::result::Result::Ok(())
    }

    type Data = Box<CpswPlatData>;

    type IdInfo = ();
}

impl DeviceRemoval for CpswPlatData {
    fn device_remove(&self) {
        pr_info!("CpswPlat data removed");
    }
}
