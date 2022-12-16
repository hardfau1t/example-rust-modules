#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]

//! Example for passing parameter for module
use kernel::bindings::kernel_param_ops;
use kernel::module_param::ArrayParam;
use kernel::prelude::*;

module! {
    type: Arguments,
    name: "arguments",
    author: "hardfault",
    description: "pass parameters to module",
    license: "GPL",
    params: {
        num: i64{
            default: 179,
            permissions: 0o000,
            description: "readonly number",
        },
        mod_num: i64{
            default: 179,
            permissions: 0o644,
            description: "modable number",
        },
        arr: ArrayParam<i64,3>{
            default: [1, 2, 3],
            permissions: 0o644,
            description: "i dont know whats wrong with this",
        },
   },
}

struct Arguments;

impl kernel::Module for Arguments {
    fn init(name: &'static CStr, module: &'static ThisModule) -> Result<Self> {
        pr_info!("module {} inserted\n", name);
        let lock = module.kernel_param_lock();
        pr_info!("num {}, writable num {}\n", num.read(), mod_num.read(&lock));
        Ok(Self)
    }
}

impl Drop for Arguments {
    fn drop(&mut self) {
        pr_info!("Dropping Arguments module\n");
        let lock = THIS_MODULE.kernel_param_lock();
        pr_info!(
            "num {}, mod_num {}, arr {:?}\n",
            num.read(),
            mod_num.read(&lock),
            arr.read(&lock)
        );
    }
}
