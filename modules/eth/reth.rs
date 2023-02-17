//! cpsw driver for beaglebone black
#![allow(non_upper_case_globals)]
use core::{
    marker::{Send, Sync},
    ops::{Deref, DerefMut},
};
use sys::KernExtDevice;

use kernel::{
    bindings as kbinds, c_str, device::RawDevice, driver::DeviceRemoval, module_platform_driver,
    of, platform, prelude::*,
};

mod net;
mod sys;

module_platform_driver! {
    type: CpswDriver,
    name: "TI_Cpsw_driver",
    author: "hardfault",
    description: "ti cpsw driver in rust",
    license: "GPL v2",
}

/// cpsw platform data
struct CpswPlatData(sys::cpsw_common);
impl Deref for CpswPlatData {
    type Target = sys::cpsw_common;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}
impl DerefMut for CpswPlatData {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}

impl DeviceRemoval for CpswPlatData {
    fn device_remove(&self) {
        pr_info!("Data removal requested\n");
    }
}

unsafe impl Sync for CpswPlatData {}
unsafe impl Send for CpswPlatData {}

struct CpswAdapter(sys::cpsw_priv);

impl Deref for CpswAdapter {
    type Target = sys::cpsw_priv;

    fn deref(&self) -> &Self::Target {
        &self.0
    }
}
impl DerefMut for CpswAdapter {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.0
    }
}

/* fn init(dev: &mut NetDevice<CpswAdapter>) -> Result {
    let tx_ch_num;
    let rx_ch_num;
    let r_priv = dev.get_priv_data();
    pr_info!(
        "r_priv {:p}, priv.priv {:p}",
        r_priv,
        &(r_priv.private_data)
    );
    // SAFETY: in cpsw_create_ports private_data.cpsw is initialized
    unsafe {
        tx_ch_num = (*r_priv.private_data.cpsw).tx_ch_num as u32;
        rx_ch_num = (*r_priv.private_data.cpsw).rx_ch_num as u32;
    }
    // to call this dev needs to be mutable borrow, but r_priv needs to be dropped for that
    dev.netif_set_real_num_tx_queues(tx_ch_num)?;
    dev.netif_set_real_num_rx_queues(rx_ch_num)?;
    // re initializing r_priv
    let r_priv = dev.get_priv_data_mut();
    // SAFETY: TODO
    unsafe {
        binds::cpsw_ndo_open(&mut r_priv.private_data as *mut _);
    }
    Ok(())
} */

/* fn uninit(dev: &mut NetDevice<CpswAdapter>) {
    let r_priv = dev.get_priv_data_mut();
    // SAFETY: TODO
    unsafe {
        binds::cpsw_ndo_open(&mut r_priv.private_data as *mut _);
    }
} */
/* fn start_xmit(mut skb: SkBuff, dev: &mut NetDevice<CpswAdapter>) -> NetdevTX {
    unsafe { binds::cpsw_ndo_start_xmit(skb.ptr, dev.ptr) };
    /* let priv_data = dev.get_priv_data();
    if let Err(e) = skb.put_padto(priv_data.private_data.tx_packet_min) {
        pr_err!("Cpsw packet pad failed: {e:?}");
        return NetdevTX::XMIT_DROP;
    }
    // SAFETY: cpsw is already initialized
    let q_idx = skb.get_queue_mapping() as usize
        % unsafe { (*priv_data.private_data.cpsw).tx_ch_num } as usize;
    skb.tx_timestamp();
    let tx_ch = unsafe { (*priv_data.private_data.cpsw).txv[q_idx].ch };
    let skb_len = skb.len();
    let inner = skb.get_internal();
    pr_info!("Sent token  {:x} {:p}, data: {:x} {:p}, len {}\n", skb.ptr as u32, skb.ptr, inner.data as u32, inner.data, skb.len());
    let ret = unsafe {
        binds::cpdma_chan_submit(
            tx_ch,
            skb.ptr as *mut core::ffi::c_void,
            inner.data as *mut _,
            skb.len() as i32,
            priv_data.private_data.emac_port as i32,
        )
    };
    if ret != 0 {
        todo!("Handle cpdma full status");
    } */
    NetdevTX::TX_OK
} */

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
        // allocate platform data
        let mut cpsw = Box::try_new(CpswPlatData(sys::cpsw_common::default()))?;
        // get device struct
        cpsw.dev = pdev.raw_device();
        let mut dev = kernel::device::Device::from_dev(pdev);
        // allocate cpsw slaves
        let slaves = dev
            .mkcalloc::<sys::cpsw_slave>(sys::CPSW_SLAVE_PORTS_NUM as usize, kbinds::GFP_KERNEL)?;
        cpsw.slaves = slaves.as_mut_ptr();
        // clock frequency
        let clk = dev.clk_get(Some(c_str!("fck")))?;
        cpsw.bus_freq_mhz = (clk.get_rate() / 1_000_000).try_into().unwrap();
        let mut ss_res_ptr: *mut kernel::bindings::resource = core::ptr::null_mut();
        // get the registers
        cpsw.regs = pdev.devm_platform_get_and_ioremap_resource(0, &mut ss_res_ptr)?
            as *mut sys::cpsw_ss_regs;
        // SAFETY: ss_res_ptr is initialized in pdev.devm_platform_get_and_ioremap_resource
        let ss_res = unsafe { ss_res_ptr.as_mut() }.unwrap();

        // get irqs
        cpsw.irqs_table[0] = pdev.platform_get_irq_byname(c_str!("rx"))? as u32;
        cpsw.irqs_table[1] = pdev.platform_get_irq_byname(c_str!("tx"))? as u32;
        cpsw.misc_irq = pdev.platform_get_irq_byname(c_str!("misc"))?;

        dev.runtime_enable();
        cpsw.probe_dt()?;

        cpsw.rx_packet_max = sys::CPSW_MAX_PACKET_SIZE as i32;
        cpsw.descs_pool_size = sys::CPSW_CPDMA_DESCS_POOL_SIZE_DEFAULT as i32;

        cpsw.init_common(
            sys::CPSW_ALE_AGEOUT_DEFAULT as i32,
            ss_res.start + sys::CPSW2_BD_OFFSET,
            sys::CPSW_CPDMA_DESCS_POOL_SIZE_DEFAULT as i32,
        )?;
        // WARN: cvoid addition may be result in incorrect value
        // SAFETY: offset is in limit of isize and wont wrap, bounds are valid
        cpsw.wr_regs =
            unsafe { (cpsw.regs as *mut core::ffi::c_void).add(sys::CPSW2_WR_OFFSET as usize) }
                as *mut _;

        net::eth_random_addr(&mut cpsw.base_mac)?;

        // TODO: change 0 1 to enum
        if let Ok(ch) = sys::cpdma::chan_create(cpsw.dma, 7, Some(sys::cpsw_tx_handler), 0) {
            cpsw.txv[0].ch = ch;
            if let Ok(ch) = sys::cpdma::chan_create(cpsw.dma, 0, Some(sys::cpsw_rx_handler), 1) {
                cpsw.rxv[0].ch = ch;
                cpsw.split_res();
                if let Ok(_) = cpsw.create_ports() {
                    if let Ok(_) = dev.request_irq(
                        cpsw.irqs_table[0],
                        Some(sys::cpsw_rx_interrupt),
                        0,
                        cpsw.deref_mut().deref_mut() as *mut _,
                    ) {
                        if let Ok(_) = dev.request_irq(
                            cpsw.irqs_table[1],
                            Some(sys::cpsw_tx_interrupt),
                            0,
                            cpsw.deref_mut().deref_mut() as *mut _,
                        ) {
                            if let Ok(_) = cpsw.register_ports() {
                                // SAFETY: cpsw_probe will allocate slaves number of slave_data, and it is called
                                pdev.pr_info(format_args!("probing cpsw successfull"));
                                return Ok(cpsw);
                            }
                        }
                    }
                }
                /* clean_unregister_netdev:
                cpsw_unregister_ports(cpsw); */
            }
        }
        /* cpts_release(cpsw->cpts);
        clean_cpts:
            cpdma_ctlr_destroy(cpsw->dma);
        clean_dt_ret:
            cpsw_remove_dt(cpsw);
            pm_runtime_put_sync(dev);
            pm_runtime_disable(dev); */
        Err(kernel::error::code::EAGAIN)
    }

    fn remove(cpsw: &Self::Data) -> Result {
        pr_info!("platform driver removal requested\n");
        unsafe { sys::cpsw_remove(cpsw.deref().deref() as *const _ as *mut _) };
        core::result::Result::Ok(())
    }

    type Data = Box<CpswPlatData>;

    type IdInfo = ();
}
