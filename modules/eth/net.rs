//! This module implements all the kernel netdev interfaces

use kernel::bindings as kbinds;

#[repr(transparent)]
pub(crate) struct Device{
    inner: kbinds::net_device,
}

impl NetDev{
}

pub(crate) fn eth_random_addr(addr: &mut [u8;6])->kernel::error::Result{
    kernel::random::getrandom(addr)?;
	addr[0] &= 0xfe;	/* clear multicast bit */
	addr[0] |= 0x02;	/* set local assignment bit (IEEE802) */
    Ok(())
}
