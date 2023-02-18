//! Safe abstractions over bindings for c functions of beaglebone black

mod binds {

    #![allow(non_camel_case_types)]
    #![allow(non_snake_case)]
    #![allow(non_upper_case_globals)]
    #![allow(improper_ctypes)]
    #![allow(missing_docs)]
    #![allow(unused)]

    use kernel::bindings::*;
    include!("binds.rs");
}
use kernel::{bindings as kbinds, device::RawDevice};

/// extension functions for kernel structs for device::Device
pub(crate) trait KernExtDevice {
    fn mkcalloc<T>(&mut self, count: usize, flags: u32) -> kernel::error::Result<&mut [T]>;
    fn request_irq<T>(
        &mut self,
        irq: u32,
        handler: Option<unsafe extern "C" fn(i32, *mut core::ffi::c_void) -> u32>,
        irq_flags: u32,
        dev_id: *mut T,
    ) -> kernel::error::Result;
    fn runtime_enable(&mut self);
}

impl KernExtDevice for kernel::device::Device {
    fn mkcalloc<T>(&mut self, count: usize, flags: u32) -> kernel::error::Result<&mut [T]> {
        // SAFETY: TODO
        let ptr = unsafe {
            kbinds::devm_kcalloc(self.raw_device(), count, core::mem::size_of::<T>(), flags)
                as *mut T
        };
        if ptr.is_null() {
            Err(kernel::error::code::ENOMEM)
        } else {
            // SAFETY: allocation is successfull for count number of values and it will be freed with self
            Ok(unsafe { core::slice::from_raw_parts_mut(ptr, count) })
        }
    }
    fn request_irq<T>(
        &mut self,
        irq: u32,
        handler: Option<unsafe extern "C" fn(i32, *mut core::ffi::c_void) -> u32>,
        irq_flags: u32,
        dev_id: *mut T,
    ) -> kernel::error::Result {
        kernel::error::to_result(unsafe {
            kbinds::devm_request_irq(
                self.raw_device(),
                irq,
                handler,
                irq_flags,
                self.name().as_char_ptr(),
                dev_id as *mut _,
            )
        })
    }
    fn runtime_enable(&mut self) {
        unsafe { kbinds::pm_runtime_enable(self.raw_device()) }
    }
}

pub(crate) use binds::{
    cpsw_common, cpsw_priv, cpsw_rx_handler, cpsw_rx_interrupt, cpsw_slave, cpsw_ss_regs,
    cpsw_tx_handler, cpsw_tx_interrupt, CPSW2_BD_OFFSET, CPSW2_WR_OFFSET, CPSW_ALE_AGEOUT_DEFAULT,
    CPSW_CPDMA_DESCS_POOL_SIZE_DEFAULT, CPSW_MAX_PACKET_SIZE, CPSW_SLAVE_PORTS_NUM,
};

impl cpsw_common {
    pub(crate) fn probe_dt(&mut self) -> kernel::error::Result<()> {
        let res = unsafe { binds::cpsw_probe_dt(self as *mut _) };
        kernel::error::to_result(res)
    }
    pub(crate) fn init_common(
        &mut self,
        ale_ageout: i32,
        start: u32,
        desc_pool_size: i32,
    ) -> kernel::error::Result {
        let ret = unsafe {
            binds::cpsw_init_common(
                self as *mut _,
                self.regs as *mut _,
                ale_ageout,
                start,
                desc_pool_size,
            )
        };
        kernel::error::to_result(ret)
    }
    pub(crate) fn split_res(&mut self) {
        //SAFETY: TODO
        unsafe { binds::cpsw_split_res(self as *mut _) }
    }
    pub(crate) fn create_ports(&mut self) -> kernel::error::Result {
        kernel::error::to_result(unsafe { binds::cpsw_create_ports(self as *mut _) })
    }
    pub(crate) fn register_ports(&mut self) -> kernel::error::Result {
        kernel::error::to_result(unsafe { binds::cpsw_register_ports(self as *mut _) })
    }
    pub(crate) fn remove(&self) {
        // SAFETY: TODO
        unsafe { binds::cpsw_remove(self as *const _ as *mut _) };
    }
}

pub(crate) mod cpdma {
    use super::binds;
    use kernel::bindings as kbinds;
    pub(crate) fn chan_create(
        ctlr: *mut binds::cpdma_ctlr,
        chan_num: i32,
        handler: binds::cpdma_handler_fn,
        rx_type: i32,
    ) -> kernel::error::Result<*mut binds::cpdma_chan> {
        let ret = unsafe { binds::cpdma_chan_create(ctlr, chan_num, handler, rx_type) };
        if unsafe { kbinds::IS_ERR(ret as *const _) } {
            Err(kernel::error::code::EAGAIN)
        } else {
            Ok(ret)
        }
    }
}
