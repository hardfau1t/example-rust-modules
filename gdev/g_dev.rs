//! my simple module
use kernel::prelude::*;
use kernel::chrdev::Registration;
use kernel::file::Operations;

module!{
    type: GDev,
    name: "Gdevmodule",
    author: "unknown",
    description: "nobody knows",
    license: "GPL",
    params: {
        my_num: u32{
            default: 179,
            permissions: 0o444,
            description: "is this is a number",
        },
    },
}

struct GDev{
    _reg: Pin<Box<kernel::chrdev::Registration<5>>>,
}

struct Ops;

impl kernel::Module for GDev{
    fn init(name: &'static CStr, module: &'static ThisModule) -> Result<Self> {
        let reg = Registration::new_pinned(name, (*my_num.read()).try_into()?, module)?;
        pr_info!("-------------------------");
        pr_info!("wow my very own module");
        pr_info!("param is {}", my_num.read());
        pr_info!("-------------------------");
        Ok(Self{_reg: reg})
    }

}
impl Drop for GDev {
    fn drop(&mut self) {
        pr_info!("------------------------------------");
        pr_info!("There goes everything into the water");
        pr_info!("------------------------------------");
    }
}

#[vtable]
impl Operations for Ops{

    type Data = ();

    type OpenData = ();

    fn open(context: &Self::OpenData,file: &kernel::file::File) -> Result<Self::Data> {
        Ok(())
    }
}
