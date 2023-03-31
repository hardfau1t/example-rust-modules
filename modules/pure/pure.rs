use kernel::{
    device::RawDevice, driver::DeviceRemoval, module_platform_driver, of, platform, prelude::*,
};

module_platform_driver! {
    type: CpswPlatform,
    name: "TI_Cpsw_driver",
    author: "hardfault",
    description: "ti cpsw driver in rust",
    license: "GPL v2",
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
        Ok(Box::try_new(CpswPlatData)?)
    }
    fn remove(cpsw: &Self::Data) -> Result {
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
