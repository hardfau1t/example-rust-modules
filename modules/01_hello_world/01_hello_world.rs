use kernel::prelude::*;

// simple hello world module
module! {
    type: Hello,
    name: "hello_world",
    license: "GPL",
}

struct Hello;

impl kernel::Module for Hello {
    // this will be called at the init
    fn init(name: &'static CStr, module: &'static ThisModule) -> Result<Self> {
        pr_info!("Module {} inserted", name);
        Ok(Hello)
    }
}

// when module is  removed this will be called
impl Drop for Hello {
    fn drop(&mut self) {
        pr_info!("Removing module");
    }
}
