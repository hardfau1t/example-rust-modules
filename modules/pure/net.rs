#![allow(unused)]
use core::{
    cell::UnsafeCell,
    default::Default,
    marker::PhantomData,
    mem::size_of,
    ptr::{addr_of, addr_of_mut, NonNull},
};
use kernel::{
    bindings,
    device::{self, RawDevice},
    error::code::ENOMEM,
    macros::vtable,
    net::SkBuff,
    str::CStr,
    to_result,
    types::ForeignOwnable,
    ARef, AlwaysRefCounted, Error, Result,
};

pub(crate) struct Device {
    inner: &'static mut bindings::net_device,
}

impl Device {
    pub(crate) fn new<T>(
        fmt_name: &CStr,
        setup: unsafe extern "C" fn(*mut bindings::net_device) -> (),
        txqs: u32,
        rxqs: u32,
    ) -> Result<Self> {
        let dev = unsafe {
            bindings::alloc_netdev_mqs(
                size_of::<T>().try_into().expect("sizeof private element should be expected to be less than 32 bit size"),
                fmt_name.as_char_ptr(),
                bindings::NET_NAME_ENUM.try_into().unwrap(), // this is ok 
                Some(setup),
                txqs,
                rxqs,
            ).as_mut().ok_or(kernel::error::code::EAGAIN)
        };
        Ok(Self{
            inner: dev?
        })
    }
    pub(crate) unsafe fn from_ptr<'a>(_dev: *mut bindings::net_device) -> &'a Self {
        todo!()
    }
}

unsafe impl AlwaysRefCounted for Device {
    fn inc_ref(&self) {
        todo!()
    }

    unsafe fn dec_ref(obj: NonNull<Self>) {
        todo!()
    }
}

unsafe impl device::RawDevice for Device {
    fn raw_device(&self) -> *mut bindings::device {
        // NOTE: This is not safe
        &self.inner.dev as *const _ as *mut _
    }
}

/// Registration structure for a network device.
pub(crate) struct Registration<T: DeviceOperations> {
    _p: PhantomData<T>,
}

impl<T: DeviceOperations> Registration<T> {
    /// Register a network device.
    pub(crate) fn register(dev: &mut Device, data: T::Data) -> Result {
        // SAFETY: `dev` was allocated during initialization and is guaranteed to be valid.
        let ret = unsafe {
            dev.inner.netdev_ops = Self::build_device_ops();
            bindings::register_netdev(dev.inner)
        };
        if ret != 0 {
            Err(Error::from_kernel_errno(ret))
        } else {
            unsafe {
                // SAFETY: The C contract guarantees that `data` is available
                // for implementers of the net_device operations (no other C code accesses
                // it), so we know that there are no concurrent threads/CPUs accessing
                // it (it's not visible to any other Rust code).
                bindings::dev_set_drvdata(dev.raw_device(), data.into_foreign() as _);
            }
            Ok(())
        }
    }
}
pub(crate) fn from_kernel_result_helper<T>(r: Result<T>) -> T
where
    T: From<i16>,
{
    match r {
        Ok(v) => v,
        // NO-OVERFLOW: negative `errno`s are no smaller than `-bindings::MAX_ERRNO`,
        // `-bindings::MAX_ERRNO` fits in an `i16` as per invariant above,
        // therefore a negative `errno` always fits in an `i16` and will not overflow.
        Err(e) => T::from(e.to_kernel_errno() as i16),
    }
}

macro_rules! from_kernel_result {
    ($($tt:tt)*) => {{
        $crate::net::from_kernel_result_helper((|| {
            $($tt)*
        })())
    }};
}

pub(crate) use from_kernel_result;

impl<T: DeviceOperations> Registration<T> {
    const DEVICE_OPS: bindings::net_device_ops = bindings::net_device_ops {
        ndo_init: None,
        ndo_uninit: None,
        ndo_open: if <T>::HAS_OPEN {
            Some(Self::open_callback)
        } else {
            None
        },
        ndo_stop: if <T>::HAS_STOP {
            Some(Self::stop_callback)
        } else {
            None
        },
        ndo_start_xmit: if <T>::HAS_START_XMIT {
            Some(Self::start_xmit_callback)
        } else {
            None
        },
        ndo_features_check: None,
        ndo_select_queue: None,
        ndo_change_rx_flags: None,
        ndo_set_rx_mode: None,
        ndo_set_mac_address: None,
        ndo_validate_addr: None,
        ndo_do_ioctl: None,
        ndo_eth_ioctl: None,
        ndo_siocbond: None,
        ndo_siocwandev: None,
        ndo_siocdevprivate: None,
        ndo_set_config: None,
        ndo_change_mtu: None,
        ndo_neigh_setup: None,
        ndo_tx_timeout: None,
        ndo_get_stats64: None,
        ndo_has_offload_stats: None,
        ndo_get_offload_stats: None,
        ndo_get_stats: None,
        ndo_vlan_rx_add_vid: None,
        ndo_vlan_rx_kill_vid: None,
        #[cfg(CONFIG_NET_POLL_CONTROLLER)]
        ndo_poll_controller: None,
        #[cfg(CONFIG_NET_POLL_CONTROLLER)]
        ndo_netpoll_setup: None,
        #[cfg(CONFIG_NET_POLL_CONTROLLER)]
        ndo_netpoll_cleanup: None,
        ndo_set_vf_mac: None,
        ndo_set_vf_vlan: None,
        ndo_set_vf_rate: None,
        ndo_set_vf_spoofchk: None,
        ndo_set_vf_trust: None,
        ndo_get_vf_config: None,
        ndo_set_vf_link_state: None,
        ndo_get_vf_stats: None,
        ndo_set_vf_port: None,
        ndo_get_vf_port: None,
        ndo_get_vf_guid: None,
        ndo_set_vf_guid: None,
        ndo_set_vf_rss_query_en: None,
        ndo_setup_tc: None,
        #[cfg(CONFIG_FCOE)]
        ndo_fcoe_enable: None,
        #[cfg(CONFIG_FCOE)]
        ndo_fcoe_disable: None,
        #[cfg(CONFIG_FCOE)]
        ndo_fcoe_ddp_setup: None,
        #[cfg(CONFIG_FCOE)]
        ndo_fcoe_ddp_done: None,
        #[cfg(CONFIG_FCOE)]
        ndo_fcoe_ddp_target: None,
        #[cfg(CONFIG_FCOE)]
        ndo_fcoe_get_hbainfo: None,
        #[cfg(CONFIG_FCOE)]
        ndo_fcoe_get_wwn: None,
        #[cfg(CONFIG_RFS_ACCEL)]
        ndo_rx_flow_steer: None,
        ndo_add_slave: None,
        ndo_del_slave: None,
        ndo_get_xmit_slave: None,
        ndo_sk_get_lower_dev: None,
        ndo_fix_features: None,
        ndo_set_features: None,
        ndo_neigh_construct: None,
        ndo_neigh_destroy: None,
        ndo_fdb_add: None,
        ndo_fdb_del: None,
        ndo_fdb_del_bulk: None,
        ndo_fdb_dump: None,
        ndo_fdb_get: None,
        ndo_bridge_setlink: None,
        ndo_bridge_getlink: None,
        ndo_bridge_dellink: None,
        ndo_change_carrier: None,
        ndo_get_phys_port_id: None,
        ndo_get_port_parent_id: None,
        ndo_get_phys_port_name: None,
        ndo_dfwd_add_station: None,
        ndo_dfwd_del_station: None,
        ndo_set_tx_maxrate: None,
        ndo_get_iflink: None,
        ndo_fill_metadata_dst: None,
        ndo_set_rx_headroom: None,
        ndo_bpf: None,
        ndo_xdp_xmit: None,
        ndo_xdp_get_xmit_slave: None,
        ndo_xsk_wakeup: None,
        ndo_tunnel_ctl: None,
        ndo_get_peer_dev: None,
        ndo_fill_forward_path: None,
        ndo_get_tstamp: None,
    };

    const fn build_device_ops() -> &'static bindings::net_device_ops {
        &Self::DEVICE_OPS
    }

    unsafe extern "C" fn open_callback(netdev: *mut bindings::net_device) -> core::ffi::c_int {
        from_kernel_result! {
            // SAFETY: The C API guarantees that `net_device` isn't released while this function is running.
            let dev = unsafe { Device::from_ptr(netdev) };
            // SAFETY: The value stored as driver data was returned by `into_foreign` during registration.
            let data = unsafe { T::Data::borrow(bindings::dev_get_drvdata(&mut (*netdev).dev)) };
            T::open(dev, data)?;
            Ok(0)
        }
    }

    unsafe extern "C" fn stop_callback(netdev: *mut bindings::net_device) -> core::ffi::c_int {
        from_kernel_result! {
            // SAFETY: The C API guarantees that `net_device` isn't released while this function is running.
            let dev = unsafe { Device::from_ptr(netdev) };
            // SAFETY: The value stored as driver data was returned by `into_foreign` during registration.
            let data = unsafe { T::Data::borrow(bindings::dev_get_drvdata(&mut (*netdev).dev)) };
            T::stop(dev, data)?;
            Ok(0)
        }
    }

    unsafe extern "C" fn start_xmit_callback(
        skb: *mut bindings::sk_buff,
        netdev: *mut bindings::net_device,
    ) -> bindings::netdev_tx_t {
        // SAFETY: The C API guarantees that `net_device` isn't released while this function is running.
        let dev = unsafe { Device::from_ptr(netdev) };
        // SAFETY: The C API guarantees that `sk_buff` isn't released while this function is running.
        let skb = unsafe { SkBuff::from_ptr(skb) };
        // SAFETY: The value stored as driver data was returned by `into_foreign` during registration.
        let data = unsafe { T::Data::borrow(bindings::dev_get_drvdata(&mut (*netdev).dev)) };
        T::start_xmit(skb, dev, data) as bindings::netdev_tx_t
    }
}

/// Driver transmit return codes.
#[repr(i32)]
pub(crate) enum NetdevTx {
    /// Driver took care of packet.
    Ok = bindings::netdev_tx_NETDEV_TX_OK,
    /// Driver tx path was busy.
    Busy = bindings::netdev_tx_NETDEV_TX_BUSY,
}

// SAFETY: `Registration` does not expose any of its state across threads.
unsafe impl<T: DeviceOperations> Sync for Registration<T> {}

// SAFETY: `Registration` is not restricted to a single thread,
// its `T::Data` is also `Send` so it may be moved to different threads.
#[allow(clippy::non_send_fields_in_send_ty)]
unsafe impl<T: DeviceOperations> Send for Registration<T> {}

/// Corresponds to the kernel's `struct net_device_ops`.
#[vtable]
pub(crate) trait DeviceOperations {
    /// The pointer type that will be used to hold driver-defined data type.
    type Data: ForeignOwnable + Send + Sync;

    /// Corresponds to `ndo_open` in `struct net_device_ops`.
    fn open(dev: &Device, data: <Self::Data as ForeignOwnable>::Borrowed<'_>) -> Result;

    /// Corresponds to `ndo_stop` in `struct net_device_ops`.
    fn stop(dev: &Device, data: <Self::Data as ForeignOwnable>::Borrowed<'_>) -> Result;

    /// Corresponds to `ndo_start_xmit` in `struct net_device_ops`.
    fn start_xmit(
        skb: &SkBuff,
        dev: &Device,
        data: <Self::Data as ForeignOwnable>::Borrowed<'_>,
    ) -> NetdevTx;
}
