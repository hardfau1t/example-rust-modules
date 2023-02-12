#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(non_upper_case_globals)]
#![allow(improper_ctypes)]
#![allow(missing_docs)]
#![allow(unused)]

use kernel::bindings::*;
// Generated
pub(crate) const CPDMA_MAX_CHANNELS: u32 = 32;
pub(crate) const CPDMA_EOI_RX_THRESH: u32 = 0;
pub(crate) const CPDMA_EOI_RX: u32 = 1;
pub(crate) const CPDMA_EOI_TX: u32 = 2;
pub(crate) const CPDMA_EOI_MISC: u32 = 3;
pub(crate) const ALE_ALL_PORTS: u32 = 7;
pub(crate) const CPSW_VERSION_1: u32 = 1638666;
pub(crate) const CPSW_VERSION_2: u32 = 1638668;
pub(crate) const CPSW_VERSION_3: u32 = 1638671;
pub(crate) const CPSW_VERSION_4: u32 = 1638674;
pub(crate) const HOST_PORT_NUM: u32 = 0;
pub(crate) const CPSW_ALE_PORTS_NUM: u32 = 3;
pub(crate) const CPSW_SLAVE_PORTS_NUM: u32 = 2;
pub(crate) const SLIVER_SIZE: u32 = 64;
pub(crate) const CPSW1_HOST_PORT_OFFSET: u32 = 40;
pub(crate) const CPSW1_SLAVE_OFFSET: u32 = 80;
pub(crate) const CPSW1_SLAVE_SIZE: u32 = 64;
pub(crate) const CPSW1_CPDMA_OFFSET: u32 = 256;
pub(crate) const CPSW1_STATERAM_OFFSET: u32 = 512;
pub(crate) const CPSW1_HW_STATS: u32 = 1024;
pub(crate) const CPSW1_CPTS_OFFSET: u32 = 1280;
pub(crate) const CPSW1_ALE_OFFSET: u32 = 1536;
pub(crate) const CPSW1_SLIVER_OFFSET: u32 = 1792;
pub(crate) const CPSW1_WR_OFFSET: u32 = 2304;
pub(crate) const CPSW2_HOST_PORT_OFFSET: u32 = 264;
pub(crate) const CPSW2_SLAVE_OFFSET: u32 = 512;
pub(crate) const CPSW2_SLAVE_SIZE: u32 = 256;
pub(crate) const CPSW2_CPDMA_OFFSET: u32 = 2048;
pub(crate) const CPSW2_HW_STATS: u32 = 2304;
pub(crate) const CPSW2_STATERAM_OFFSET: u32 = 2560;
pub(crate) const CPSW2_CPTS_OFFSET: u32 = 3072;
pub(crate) const CPSW2_ALE_OFFSET: u32 = 3328;
pub(crate) const CPSW2_SLIVER_OFFSET: u32 = 3456;
pub(crate) const CPSW2_BD_OFFSET: u32 = 8192;
pub(crate) const CPSW2_WR_OFFSET: u32 = 4608;
pub(crate) const CPDMA_RXTHRESH: u32 = 192;
pub(crate) const CPDMA_RXFREE: u32 = 224;
pub(crate) const CPDMA_TXHDP: u32 = 0;
pub(crate) const CPDMA_RXHDP: u32 = 32;
pub(crate) const CPDMA_TXCP: u32 = 64;
pub(crate) const CPDMA_RXCP: u32 = 96;
pub(crate) const CPSW_RX_VLAN_ENCAP_HDR_SIZE: u32 = 4;
pub(crate) const CPSW_MIN_PACKET_SIZE_VLAN: u32 = 64;
pub(crate) const CPSW_MIN_PACKET_SIZE: u32 = 60;
pub(crate) const CPSW_MAX_PACKET_SIZE: u32 = 1526;
pub(crate) const RX_PRIORITY_MAPPING: u32 = 1985229328;
pub(crate) const TX_PRIORITY_MAPPING: u32 = 857870592;
pub(crate) const CPDMA_TX_PRIORITY_MAP: u32 = 1985229328;
pub(crate) const CPSW_ALE_VLAN_AWARE: u32 = 1;
pub(crate) const CPSW_FIFO_NORMAL_MODE: u32 = 0;
pub(crate) const CPSW_FIFO_DUAL_MAC_MODE: u32 = 65536;
pub(crate) const CPSW_FIFO_RATE_LIMIT_MODE: u32 = 131072;
pub(crate) const CPSW_INTPACEEN: u32 = 4128768;
pub(crate) const CPSW_INTPRESCALE_MASK: u32 = 2047;
pub(crate) const CPSW_CMINTMAX_CNT: u32 = 63;
pub(crate) const CPSW_CMINTMIN_CNT: u32 = 2;
pub(crate) const CPSW_CMINTMAX_INTVL: u32 = 500;
pub(crate) const CPSW_CMINTMIN_INTVL: u32 = 16;
pub(crate) const IRQ_NUM: u32 = 2;
pub(crate) const CPSW_MAX_QUEUES: u32 = 8;
pub(crate) const CPSW_CPDMA_DESCS_POOL_SIZE_DEFAULT: u32 = 256;
pub(crate) const CPSW_ALE_AGEOUT_DEFAULT: u32 = 10;
pub(crate) const CPSW_FIFO_QUEUE_TYPE_SHIFT: u32 = 16;
pub(crate) const CPSW_FIFO_SHAPE_EN_SHIFT: u32 = 16;
pub(crate) const CPSW_FIFO_RATE_EN_SHIFT: u32 = 20;
pub(crate) const CPSW_TC_NUM: u32 = 4;
pub(crate) const CPSW_FIFO_SHAPERS_NUM: u32 = 3;
pub(crate) const CPSW_PCT_MASK: u32 = 127;
pub(crate) const CPSW_BD_RAM_SIZE: u32 = 8192;
pub(crate) const CPSW_RX_VLAN_ENCAP_HDR_PRIO_SHIFT: u32 = 29;
pub(crate) const CPSW_RX_VLAN_ENCAP_HDR_VID_SHIFT: u32 = 16;
pub(crate) const CPSW_RX_VLAN_ENCAP_HDR_PKT_TYPE_SHIFT: u32 = 8;
pub(crate) const CPSW1_MAX_BLKS: u32 = 0;
pub(crate) const CPSW1_BLK_CNT: u32 = 4;
pub(crate) const CPSW1_TX_IN_CTL: u32 = 8;
pub(crate) const CPSW1_PORT_VLAN: u32 = 12;
pub(crate) const CPSW1_TX_PRI_MAP: u32 = 16;
pub(crate) const CPSW1_TS_CTL: u32 = 20;
pub(crate) const CPSW1_TS_SEQ_LTYPE: u32 = 24;
pub(crate) const CPSW1_TS_VLAN: u32 = 28;
pub(crate) const CPSW2_CONTROL: u32 = 0;
pub(crate) const CPSW2_MAX_BLKS: u32 = 8;
pub(crate) const CPSW2_BLK_CNT: u32 = 12;
pub(crate) const CPSW2_TX_IN_CTL: u32 = 16;
pub(crate) const CPSW2_PORT_VLAN: u32 = 20;
pub(crate) const CPSW2_TX_PRI_MAP: u32 = 24;
pub(crate) const CPSW2_TS_SEQ_MTYPE: u32 = 28;
pub(crate) const SA_LO: u32 = 32;
pub(crate) const SA_HI: u32 = 36;
pub(crate) const SEND_PERCENT: u32 = 40;
pub(crate) const RX_DSCP_PRI_MAP0: u32 = 48;
pub(crate) const RX_DSCP_PRI_MAP1: u32 = 52;
pub(crate) const RX_DSCP_PRI_MAP2: u32 = 56;
pub(crate) const RX_DSCP_PRI_MAP3: u32 = 60;
pub(crate) const RX_DSCP_PRI_MAP4: u32 = 64;
pub(crate) const RX_DSCP_PRI_MAP5: u32 = 68;
pub(crate) const RX_DSCP_PRI_MAP6: u32 = 72;
pub(crate) const RX_DSCP_PRI_MAP7: u32 = 76;
pub(crate) const TS_SEQ_ID_OFFSET_SHIFT: u32 = 16;
pub(crate) const TS_SEQ_ID_OFFSET_MASK: u32 = 63;
pub(crate) const TS_MSG_TYPE_EN_SHIFT: u32 = 0;
pub(crate) const TS_MSG_TYPE_EN_MASK: u32 = 65535;
pub(crate) const EVENT_MSG_BITS: u32 = 15;
pub(crate) const CPSW_V1_MSG_TYPE_OFS: u32 = 16;
pub(crate) const CPSW_V1_SEQ_ID_OFS_SHIFT: u32 = 16;
pub(crate) const CPSW_MAX_BLKS_TX: u32 = 15;
pub(crate) const CPSW_MAX_BLKS_TX_SHIFT: u32 = 4;
pub(crate) const CPSW_MAX_BLKS_RX: u32 = 5;
pub(crate) const CPSW_XDP_CONSUMED: u32 = 1;
pub(crate) const CPSW_XDP_PASS: u32 = 0;
pub(crate) const BINDINGS_GFP_KERNEL: gfp_t = 3264;
pub(crate) const BINDINGS___GFP_ZERO: gfp_t = 256;
pub(crate) const BINDINGS_EPOLLIN: __poll_t = 1;
pub(crate) const BINDINGS_EPOLLOUT: __poll_t = 4;
pub(crate) const BINDINGS_EPOLLERR: __poll_t = 8;
pub(crate) const BINDINGS_EPOLLHUP: __poll_t = 16;
pub(crate) const BINDINGS_MAX_LFS_FILESIZE: loff_t = 17592186040320;
pub(crate) const BINDINGS_NLA_HDRLEN: core::ffi::c_int = 4;
pub(crate) const CPSW_ALE: core::ffi::c_int = 1;
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct cpsw_ale_params {
    pub(crate) dev: *mut device,
    pub(crate) ale_regs: *mut core::ffi::c_void,
    pub(crate) ale_ageout: core::ffi::c_ulong,
    pub(crate) ale_entries: core::ffi::c_ulong,
    pub(crate) ale_ports: core::ffi::c_ulong,
    pub(crate) nu_switch_ale: bool_,
    pub(crate) major_ver_mask: u32_,
    pub(crate) dev_id: *const core::ffi::c_char,
    pub(crate) bus_freq: core::ffi::c_ulong,
}
impl Default for cpsw_ale_params {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct ale_entry_fld {
    _unused: [u8; 0],
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct cpsw_ale {
    pub(crate) params: cpsw_ale_params,
    pub(crate) timer: timer_list,
    pub(crate) ageout: core::ffi::c_ulong,
    pub(crate) version: u32_,
    pub(crate) features: u32_,
    pub(crate) port_mask_bits: u32_,
    pub(crate) port_num_bits: u32_,
    pub(crate) vlan_field_bits: u32_,
    pub(crate) p0_untag_vid_mask: *mut core::ffi::c_ulong,
    pub(crate) vlan_entry_tbl: *const ale_entry_fld,
}
impl Default for cpsw_ale {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
pub(crate) const cpsw_ale_control_ALE_ENABLE: cpsw_ale_control = 0;
pub(crate) const cpsw_ale_control_ALE_CLEAR: cpsw_ale_control = 1;
pub(crate) const cpsw_ale_control_ALE_AGEOUT: cpsw_ale_control = 2;
pub(crate) const cpsw_ale_control_ALE_P0_UNI_FLOOD: cpsw_ale_control = 3;
pub(crate) const cpsw_ale_control_ALE_VLAN_NOLEARN: cpsw_ale_control = 4;
pub(crate) const cpsw_ale_control_ALE_NO_PORT_VLAN: cpsw_ale_control = 5;
pub(crate) const cpsw_ale_control_ALE_OUI_DENY: cpsw_ale_control = 6;
pub(crate) const cpsw_ale_control_ALE_BYPASS: cpsw_ale_control = 7;
pub(crate) const cpsw_ale_control_ALE_RATE_LIMIT_TX: cpsw_ale_control = 8;
pub(crate) const cpsw_ale_control_ALE_VLAN_AWARE: cpsw_ale_control = 9;
pub(crate) const cpsw_ale_control_ALE_AUTH_ENABLE: cpsw_ale_control = 10;
pub(crate) const cpsw_ale_control_ALE_RATE_LIMIT: cpsw_ale_control = 11;
pub(crate) const cpsw_ale_control_ALE_PORT_STATE: cpsw_ale_control = 12;
pub(crate) const cpsw_ale_control_ALE_PORT_DROP_UNTAGGED: cpsw_ale_control = 13;
pub(crate) const cpsw_ale_control_ALE_PORT_DROP_UNKNOWN_VLAN: cpsw_ale_control = 14;
pub(crate) const cpsw_ale_control_ALE_PORT_NOLEARN: cpsw_ale_control = 15;
pub(crate) const cpsw_ale_control_ALE_PORT_NO_SA_UPDATE: cpsw_ale_control = 16;
pub(crate) const cpsw_ale_control_ALE_PORT_UNKNOWN_VLAN_MEMBER: cpsw_ale_control = 17;
pub(crate) const cpsw_ale_control_ALE_PORT_UNKNOWN_MCAST_FLOOD: cpsw_ale_control = 18;
pub(crate) const cpsw_ale_control_ALE_PORT_UNKNOWN_REG_MCAST_FLOOD: cpsw_ale_control = 19;
pub(crate) const cpsw_ale_control_ALE_PORT_UNTAGGED_EGRESS: cpsw_ale_control = 20;
pub(crate) const cpsw_ale_control_ALE_PORT_MACONLY: cpsw_ale_control = 21;
pub(crate) const cpsw_ale_control_ALE_PORT_MACONLY_CAF: cpsw_ale_control = 22;
pub(crate) const cpsw_ale_control_ALE_PORT_BCAST_LIMIT: cpsw_ale_control = 23;
pub(crate) const cpsw_ale_control_ALE_PORT_MCAST_LIMIT: cpsw_ale_control = 24;
pub(crate) const cpsw_ale_control_ALE_DEFAULT_THREAD_ID: cpsw_ale_control = 25;
pub(crate) const cpsw_ale_control_ALE_DEFAULT_THREAD_ENABLE: cpsw_ale_control = 26;
pub(crate) const cpsw_ale_control_ALE_NUM_CONTROLS: cpsw_ale_control = 27;
pub(crate) type cpsw_ale_control = core::ffi::c_uint;
pub(crate) const cpsw_ale_port_state_ALE_PORT_STATE_DISABLE: cpsw_ale_port_state = 0;
pub(crate) const cpsw_ale_port_state_ALE_PORT_STATE_BLOCK: cpsw_ale_port_state = 1;
pub(crate) const cpsw_ale_port_state_ALE_PORT_STATE_LEARN: cpsw_ale_port_state = 2;
pub(crate) const cpsw_ale_port_state_ALE_PORT_STATE_FORWARD: cpsw_ale_port_state = 3;
pub(crate) type cpsw_ale_port_state = core::ffi::c_uint;
extern "C" {
    pub(crate) fn cpsw_ale_create(params: *mut cpsw_ale_params) -> *mut cpsw_ale;
}
extern "C" {
    pub(crate) fn cpsw_ale_start(ale: *mut cpsw_ale);
}
extern "C" {
    pub(crate) fn cpsw_ale_stop(ale: *mut cpsw_ale);
}
extern "C" {
    pub(crate) fn cpsw_ale_flush_multicast(
        ale: *mut cpsw_ale,
        port_mask: core::ffi::c_int,
        vid: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_ale_add_ucast(
        ale: *mut cpsw_ale,
        addr: *const u8_,
        port: core::ffi::c_int,
        flags: core::ffi::c_int,
        vid: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_ale_del_ucast(
        ale: *mut cpsw_ale,
        addr: *const u8_,
        port: core::ffi::c_int,
        flags: core::ffi::c_int,
        vid: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_ale_add_mcast(
        ale: *mut cpsw_ale,
        addr: *const u8_,
        port_mask: core::ffi::c_int,
        flags: core::ffi::c_int,
        vid: u16_,
        mcast_state: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_ale_del_mcast(
        ale: *mut cpsw_ale,
        addr: *const u8_,
        port_mask: core::ffi::c_int,
        flags: core::ffi::c_int,
        vid: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_ale_add_vlan(
        ale: *mut cpsw_ale,
        vid: u16_,
        port: core::ffi::c_int,
        untag: core::ffi::c_int,
        reg_mcast: core::ffi::c_int,
        unreg_mcast: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_ale_del_vlan(
        ale: *mut cpsw_ale,
        vid: u16_,
        port: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_ale_set_allmulti(
        ale: *mut cpsw_ale,
        allmulti: core::ffi::c_int,
        port: core::ffi::c_int,
    );
}
extern "C" {
    pub(crate) fn cpsw_ale_rx_ratelimit_bc(
        ale: *mut cpsw_ale,
        port: core::ffi::c_int,
        ratelimit_pps: core::ffi::c_uint,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_ale_rx_ratelimit_mc(
        ale: *mut cpsw_ale,
        port: core::ffi::c_int,
        ratelimit_pps: core::ffi::c_uint,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_ale_control_get(
        ale: *mut cpsw_ale,
        port: core::ffi::c_int,
        control: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_ale_control_set(
        ale: *mut cpsw_ale,
        port: core::ffi::c_int,
        control: core::ffi::c_int,
        value: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_ale_dump(ale: *mut cpsw_ale, data: *mut u32_);
}
extern "C" {
    pub(crate) fn cpsw_ale_get_num_entries(ale: *mut cpsw_ale) -> u32_;
}
extern "C" {
    pub(crate) fn cpsw_ale_vlan_add_modify(
        ale: *mut cpsw_ale,
        vid: u16_,
        port_mask: core::ffi::c_int,
        untag_mask: core::ffi::c_int,
        reg_mcast: core::ffi::c_int,
        unreg_mcast: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_ale_vlan_del_modify(
        ale: *mut cpsw_ale,
        vid: u16_,
        port_mask: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_ale_set_unreg_mcast(
        ale: *mut cpsw_ale,
        unreg_mcast_mask: core::ffi::c_int,
        add: bool_,
    );
}
pub(crate) const CPSW: core::ffi::c_int = 1;
extern "C" {
    pub(crate) fn linkmode_resolve_pause(
        local_adv: *const core::ffi::c_ulong,
        partner_adv: *const core::ffi::c_ulong,
        tx_pause: *mut bool_,
        rx_pause: *mut bool_,
    );
}
extern "C" {
    pub(crate) fn linkmode_set_pause(advertisement: *mut core::ffi::c_ulong, tx: bool_, rx: bool_);
}
#[repr(C)]
#[derive(Default, Copy, Clone)]
pub(crate) struct mii_ioctl_data {
    pub(crate) phy_id: __u16,
    pub(crate) reg_num: __u16,
    pub(crate) val_in: __u16,
    pub(crate) val_out: __u16,
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct mii_if_info {
    pub(crate) phy_id: core::ffi::c_int,
    pub(crate) advertising: core::ffi::c_int,
    pub(crate) phy_id_mask: core::ffi::c_int,
    pub(crate) reg_num_mask: core::ffi::c_int,
    pub(crate) _bitfield_1: __BindgenBitfieldUnit<[u8; 1usize], u8>,
    pub(crate) dev: *mut net_device,
    pub(crate) mdio_read: ::core::option::Option<
        unsafe extern "C" fn(
            dev: *mut net_device,
            phy_id: core::ffi::c_int,
            location: core::ffi::c_int,
        ) -> core::ffi::c_int,
    >,
    pub(crate) mdio_write: ::core::option::Option<
        unsafe extern "C" fn(
            dev: *mut net_device,
            phy_id: core::ffi::c_int,
            location: core::ffi::c_int,
            val: core::ffi::c_int,
        ),
    >,
}
impl Default for mii_if_info {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
impl mii_if_info {
    #[inline]
    pub(crate) fn full_duplex(&self) -> core::ffi::c_uint {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(0usize, 1u8) as u32) }
    }
    #[inline]
    pub(crate) fn set_full_duplex(&mut self, val: core::ffi::c_uint) {
        unsafe {
            let val: u32 = ::core::mem::transmute(val);
            self._bitfield_1.set(0usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn force_media(&self) -> core::ffi::c_uint {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(1usize, 1u8) as u32) }
    }
    #[inline]
    pub(crate) fn set_force_media(&mut self, val: core::ffi::c_uint) {
        unsafe {
            let val: u32 = ::core::mem::transmute(val);
            self._bitfield_1.set(1usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn supports_gmii(&self) -> core::ffi::c_uint {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(2usize, 1u8) as u32) }
    }
    #[inline]
    pub(crate) fn set_supports_gmii(&mut self, val: core::ffi::c_uint) {
        unsafe {
            let val: u32 = ::core::mem::transmute(val);
            self._bitfield_1.set(2usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn new_bitfield_1(
        full_duplex: core::ffi::c_uint,
        force_media: core::ffi::c_uint,
        supports_gmii: core::ffi::c_uint,
    ) -> __BindgenBitfieldUnit<[u8; 1usize], u8> {
        let mut __bindgen_bitfield_unit: __BindgenBitfieldUnit<[u8; 1usize], u8> =
            Default::default();
        __bindgen_bitfield_unit.set(0usize, 1u8, {
            let full_duplex: u32 = unsafe { ::core::mem::transmute(full_duplex) };
            full_duplex as u64
        });
        __bindgen_bitfield_unit.set(1usize, 1u8, {
            let force_media: u32 = unsafe { ::core::mem::transmute(force_media) };
            force_media as u64
        });
        __bindgen_bitfield_unit.set(2usize, 1u8, {
            let supports_gmii: u32 = unsafe { ::core::mem::transmute(supports_gmii) };
            supports_gmii as u64
        });
        __bindgen_bitfield_unit
    }
}
extern "C" {
    pub(crate) fn mii_link_ok(mii: *mut mii_if_info) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mii_nway_restart(mii: *mut mii_if_info) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mii_ethtool_gset(mii: *mut mii_if_info, ecmd: *mut ethtool_cmd);
}
extern "C" {
    pub(crate) fn mii_ethtool_get_link_ksettings(mii: *mut mii_if_info, cmd: *mut ethtool_link_ksettings);
}
extern "C" {
    pub(crate) fn mii_ethtool_sset(mii: *mut mii_if_info, ecmd: *mut ethtool_cmd) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mii_ethtool_set_link_ksettings(
        mii: *mut mii_if_info,
        cmd: *const ethtool_link_ksettings,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mii_check_gmii_support(mii: *mut mii_if_info) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mii_check_link(mii: *mut mii_if_info);
}
extern "C" {
    pub(crate) fn mii_check_media(
        mii: *mut mii_if_info,
        ok_to_print: core::ffi::c_uint,
        init_media: core::ffi::c_uint,
    ) -> core::ffi::c_uint;
}
extern "C" {
    pub(crate) fn generic_mii_ioctl(
        mii_if: *mut mii_if_info,
        mii_data: *mut mii_ioctl_data,
        cmd: core::ffi::c_int,
        duplex_changed: *mut core::ffi::c_uint,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn __field_overflow();
}
extern "C" {
    pub(crate) fn __bad_mask();
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct reset_control {
    _unused: [u8; 0],
}
pub(crate) const mdio_mutex_lock_class_MDIO_MUTEX_NORMAL: mdio_mutex_lock_class = 0;
pub(crate) const mdio_mutex_lock_class_MDIO_MUTEX_MUX: mdio_mutex_lock_class = 1;
pub(crate) const mdio_mutex_lock_class_MDIO_MUTEX_NESTED: mdio_mutex_lock_class = 2;
pub(crate) type mdio_mutex_lock_class = core::ffi::c_uint;
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct mdio_device {
    pub(crate) dev: device,
    pub(crate) bus: *mut mii_bus,
    pub(crate) modalias: [core::ffi::c_char; 32usize],
    pub(crate) bus_match: ::core::option::Option<
        unsafe extern "C" fn(dev: *mut device, drv: *mut device_driver) -> core::ffi::c_int,
    >,
    pub(crate) device_free: ::core::option::Option<unsafe extern "C" fn(mdiodev: *mut mdio_device)>,
    pub(crate) device_remove: ::core::option::Option<unsafe extern "C" fn(mdiodev: *mut mdio_device)>,
    pub(crate) addr: core::ffi::c_int,
    pub(crate) flags: core::ffi::c_int,
    pub(crate) reset_gpio: *mut gpio_desc,
    pub(crate) reset_ctrl: *mut reset_control,
    pub(crate) reset_assert_delay: core::ffi::c_uint,
    pub(crate) reset_deassert_delay: core::ffi::c_uint,
}
impl Default for mdio_device {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct mdio_driver_common {
    pub(crate) driver: device_driver,
    pub(crate) flags: core::ffi::c_int,
}
impl Default for mdio_driver_common {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct mdio_driver {
    pub(crate) mdiodrv: mdio_driver_common,
    pub(crate) probe:
        ::core::option::Option<unsafe extern "C" fn(mdiodev: *mut mdio_device) -> core::ffi::c_int>,
    pub(crate) remove: ::core::option::Option<unsafe extern "C" fn(mdiodev: *mut mdio_device)>,
    pub(crate) shutdown: ::core::option::Option<unsafe extern "C" fn(mdiodev: *mut mdio_device)>,
}
impl Default for mdio_driver {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
extern "C" {
    pub(crate) fn mdio_device_free(mdiodev: *mut mdio_device);
}
extern "C" {
    pub(crate) fn mdio_device_create(bus: *mut mii_bus, addr: core::ffi::c_int) -> *mut mdio_device;
}
extern "C" {
    pub(crate) fn mdio_device_register(mdiodev: *mut mdio_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mdio_device_remove(mdiodev: *mut mdio_device);
}
extern "C" {
    pub(crate) fn mdio_device_reset(mdiodev: *mut mdio_device, value: core::ffi::c_int);
}
extern "C" {
    pub(crate) fn mdio_driver_register(drv: *mut mdio_driver) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mdio_driver_unregister(drv: *mut mdio_driver);
}
extern "C" {
    pub(crate) fn mdio_device_bus_match(dev: *mut device, drv: *mut device_driver) -> core::ffi::c_int;
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct mdio_if_info {
    pub(crate) prtad: core::ffi::c_int,
    pub(crate) mmds: u32_,
    pub(crate) mode_support: core::ffi::c_uint,
    pub(crate) dev: *mut net_device,
    pub(crate) mdio_read: ::core::option::Option<
        unsafe extern "C" fn(
            dev: *mut net_device,
            prtad: core::ffi::c_int,
            devad: core::ffi::c_int,
            addr: u16_,
        ) -> core::ffi::c_int,
    >,
    pub(crate) mdio_write: ::core::option::Option<
        unsafe extern "C" fn(
            dev: *mut net_device,
            prtad: core::ffi::c_int,
            devad: core::ffi::c_int,
            addr: u16_,
            val: u16_,
        ) -> core::ffi::c_int,
    >,
}
impl Default for mdio_if_info {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
extern "C" {
    pub(crate) fn mdio45_probe(mdio: *mut mdio_if_info, prtad: core::ffi::c_int) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mdio_set_flag(
        mdio: *const mdio_if_info,
        prtad: core::ffi::c_int,
        devad: core::ffi::c_int,
        addr: u16_,
        mask: core::ffi::c_int,
        sense: bool_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mdio45_links_ok(mdio: *const mdio_if_info, mmds: u32_) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mdio45_nway_restart(mdio: *const mdio_if_info) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mdio45_ethtool_gset_npage(
        mdio: *const mdio_if_info,
        ecmd: *mut ethtool_cmd,
        npage_adv: u32_,
        npage_lpa: u32_,
    );
}
extern "C" {
    pub(crate) fn mdio45_ethtool_ksettings_get_npage(
        mdio: *const mdio_if_info,
        cmd: *mut ethtool_link_ksettings,
        npage_adv: u32_,
        npage_lpa: u32_,
    );
}
extern "C" {
    pub(crate) fn mdio_mii_ioctl(
        mdio: *const mdio_if_info,
        mii_data: *mut mii_ioctl_data,
        cmd: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn __mdiobus_read(
        bus: *mut mii_bus,
        addr: core::ffi::c_int,
        regnum: u32_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn __mdiobus_write(
        bus: *mut mii_bus,
        addr: core::ffi::c_int,
        regnum: u32_,
        val: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn __mdiobus_modify_changed(
        bus: *mut mii_bus,
        addr: core::ffi::c_int,
        regnum: u32_,
        mask: u16_,
        set: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mdiobus_read(
        bus: *mut mii_bus,
        addr: core::ffi::c_int,
        regnum: u32_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mdiobus_read_nested(
        bus: *mut mii_bus,
        addr: core::ffi::c_int,
        regnum: u32_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mdiobus_write(
        bus: *mut mii_bus,
        addr: core::ffi::c_int,
        regnum: u32_,
        val: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mdiobus_write_nested(
        bus: *mut mii_bus,
        addr: core::ffi::c_int,
        regnum: u32_,
        val: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mdiobus_modify(
        bus: *mut mii_bus,
        addr: core::ffi::c_int,
        regnum: u32_,
        mask: u16_,
        set: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mdiobus_modify_changed(
        bus: *mut mii_bus,
        addr: core::ffi::c_int,
        regnum: u32_,
        mask: u16_,
        set: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mdiobus_register_device(mdiodev: *mut mdio_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mdiobus_unregister_device(mdiodev: *mut mdio_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mdiobus_is_registered_device(bus: *mut mii_bus, addr: core::ffi::c_int) -> bool_;
}
extern "C" {
    pub(crate) fn mdiobus_get_phy(bus: *mut mii_bus, addr: core::ffi::c_int) -> *mut phy_device;
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct mii_timestamper {
    pub(crate) rxtstamp: ::core::option::Option<
        unsafe extern "C" fn(
            mii_ts: *mut mii_timestamper,
            skb: *mut sk_buff,
            type_: core::ffi::c_int,
        ) -> bool_,
    >,
    pub(crate) txtstamp: ::core::option::Option<
        unsafe extern "C" fn(
            mii_ts: *mut mii_timestamper,
            skb: *mut sk_buff,
            type_: core::ffi::c_int,
        ),
    >,
    pub(crate) hwtstamp: ::core::option::Option<
        unsafe extern "C" fn(mii_ts: *mut mii_timestamper, ifreq: *mut ifreq) -> core::ffi::c_int,
    >,
    pub(crate) link_state: ::core::option::Option<
        unsafe extern "C" fn(mii_ts: *mut mii_timestamper, phydev: *mut phy_device),
    >,
    pub(crate) ts_info: ::core::option::Option<
        unsafe extern "C" fn(
            mii_ts: *mut mii_timestamper,
            ts_info: *mut ethtool_ts_info,
        ) -> core::ffi::c_int,
    >,
    pub(crate) device: *mut device,
}
impl Default for mii_timestamper {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Default, Copy, Clone)]
pub(crate) struct mii_timestamping_ctrl {
    pub(crate) probe_channel: ::core::option::Option<
        unsafe extern "C" fn(device: *mut device, port: core::ffi::c_uint) -> *mut mii_timestamper,
    >,
    pub(crate) release_channel: ::core::option::Option<
        unsafe extern "C" fn(device: *mut device, mii_ts: *mut mii_timestamper),
    >,
}
extern "C" {
    pub(crate) static mut phy_basic_features: [core::ffi::c_ulong; 3usize];
}
extern "C" {
    pub(crate) static mut phy_basic_t1_features: [core::ffi::c_ulong; 3usize];
}
extern "C" {
    pub(crate) static mut phy_gbit_features: [core::ffi::c_ulong; 3usize];
}
extern "C" {
    pub(crate) static mut phy_gbit_fibre_features: [core::ffi::c_ulong; 3usize];
}
extern "C" {
    pub(crate) static mut phy_gbit_all_ports_features: [core::ffi::c_ulong; 3usize];
}
extern "C" {
    pub(crate) static mut phy_10gbit_features: [core::ffi::c_ulong; 3usize];
}
extern "C" {
    pub(crate) static mut phy_10gbit_fec_features: [core::ffi::c_ulong; 3usize];
}
extern "C" {
    pub(crate) static mut phy_10gbit_full_features: [core::ffi::c_ulong; 3usize];
}
extern "C" {
    pub(crate) static phy_basic_ports_array: [core::ffi::c_int; 3usize];
}
extern "C" {
    pub(crate) static phy_fibre_port_array: [core::ffi::c_int; 1usize];
}
extern "C" {
    pub(crate) static phy_all_ports_features_array: [core::ffi::c_int; 7usize];
}
extern "C" {
    pub(crate) static phy_10_100_features_array: [core::ffi::c_int; 4usize];
}
extern "C" {
    pub(crate) static phy_basic_t1_features_array: [core::ffi::c_int; 3usize];
}
extern "C" {
    pub(crate) static phy_gbit_features_array: [core::ffi::c_int; 2usize];
}
extern "C" {
    pub(crate) static phy_10gbit_features_array: [core::ffi::c_int; 1usize];
}
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_NA: phy_interface_t = 0;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_INTERNAL: phy_interface_t = 1;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_MII: phy_interface_t = 2;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_GMII: phy_interface_t = 3;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_SGMII: phy_interface_t = 4;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_TBI: phy_interface_t = 5;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_REVMII: phy_interface_t = 6;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_RMII: phy_interface_t = 7;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_REVRMII: phy_interface_t = 8;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_RGMII: phy_interface_t = 9;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_RGMII_ID: phy_interface_t = 10;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_RGMII_RXID: phy_interface_t = 11;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_RGMII_TXID: phy_interface_t = 12;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_RTBI: phy_interface_t = 13;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_SMII: phy_interface_t = 14;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_XGMII: phy_interface_t = 15;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_XLGMII: phy_interface_t = 16;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_MOCA: phy_interface_t = 17;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_QSGMII: phy_interface_t = 18;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_TRGMII: phy_interface_t = 19;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_100BASEX: phy_interface_t = 20;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_1000BASEX: phy_interface_t = 21;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_2500BASEX: phy_interface_t = 22;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_5GBASER: phy_interface_t = 23;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_RXAUI: phy_interface_t = 24;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_XAUI: phy_interface_t = 25;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_10GBASER: phy_interface_t = 26;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_25GBASER: phy_interface_t = 27;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_USXGMII: phy_interface_t = 28;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_10GKR: phy_interface_t = 29;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_QUSGMII: phy_interface_t = 30;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_1000BASEKX: phy_interface_t = 31;
pub(crate) const phy_interface_t_PHY_INTERFACE_MODE_MAX: phy_interface_t = 32;
pub(crate) type phy_interface_t = core::ffi::c_uint;
extern "C" {
    pub(crate) fn phy_supported_speeds(
        phy: *mut phy_device,
        speeds: *mut core::ffi::c_uint,
        size: core::ffi::c_uint,
    ) -> core::ffi::c_uint;
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct phylink {
    _unused: [u8; 0],
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct sfp_upstream_ops {
    _unused: [u8; 0],
}
#[repr(C)]
#[derive(Default, Copy, Clone)]
pub(crate) struct mdio_bus_stats {
    pub(crate) transfers: u64_stats_t,
    pub(crate) errors: u64_stats_t,
    pub(crate) writes: u64_stats_t,
    pub(crate) reads: u64_stats_t,
    pub(crate) syncp: u64_stats_sync,
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct phy_package_shared {
    pub(crate) addr: core::ffi::c_int,
    pub(crate) refcnt: refcount_t,
    pub(crate) flags: core::ffi::c_ulong,
    pub(crate) priv_size: usize,
    pub(crate) priv_: *mut core::ffi::c_void,
}
impl Default for phy_package_shared {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct mii_bus {
    pub(crate) owner: *mut module,
    pub(crate) name: *const core::ffi::c_char,
    pub(crate) id: [core::ffi::c_char; 61usize],
    pub(crate) priv_: *mut core::ffi::c_void,
    pub(crate) read: ::core::option::Option<
        unsafe extern "C" fn(
            bus: *mut mii_bus,
            addr: core::ffi::c_int,
            regnum: core::ffi::c_int,
        ) -> core::ffi::c_int,
    >,
    pub(crate) write: ::core::option::Option<
        unsafe extern "C" fn(
            bus: *mut mii_bus,
            addr: core::ffi::c_int,
            regnum: core::ffi::c_int,
            val: u16_,
        ) -> core::ffi::c_int,
    >,
    pub(crate) reset: ::core::option::Option<unsafe extern "C" fn(bus: *mut mii_bus) -> core::ffi::c_int>,
    pub(crate) stats: [mdio_bus_stats; 32usize],
    pub(crate) mdio_lock: mutex,
    pub(crate) parent: *mut device,
    pub(crate) state: mii_bus__bindgen_ty_1,
    pub(crate) dev: device,
    pub(crate) mdio_map: [*mut mdio_device; 32usize],
    pub(crate) phy_mask: u32_,
    pub(crate) phy_ignore_ta_mask: u32_,
    pub(crate) irq: [core::ffi::c_int; 32usize],
    pub(crate) reset_delay_us: core::ffi::c_int,
    pub(crate) reset_post_delay_us: core::ffi::c_int,
    pub(crate) reset_gpiod: *mut gpio_desc,
    pub(crate) probe_capabilities: mii_bus__bindgen_ty_2,
    pub(crate) shared_lock: mutex,
    pub(crate) shared: [*mut phy_package_shared; 32usize],
}
pub(crate) const mii_bus_MDIOBUS_ALLOCATED: core::ffi::c_uint = 1;
pub(crate) const mii_bus_MDIOBUS_REGISTERED: core::ffi::c_uint = 2;
pub(crate) const mii_bus_MDIOBUS_UNREGISTERED: core::ffi::c_uint = 3;
pub(crate) const mii_bus_MDIOBUS_RELEASED: core::ffi::c_uint = 4;
pub(crate) type mii_bus__bindgen_ty_1 = core::ffi::c_uint;
pub(crate) const mii_bus_MDIOBUS_NO_CAP: core::ffi::c_uint = 0;
pub(crate) const mii_bus_MDIOBUS_C22: core::ffi::c_uint = 1;
pub(crate) const mii_bus_MDIOBUS_C45: core::ffi::c_uint = 2;
pub(crate) const mii_bus_MDIOBUS_C22_C45: core::ffi::c_uint = 3;
pub(crate) type mii_bus__bindgen_ty_2 = core::ffi::c_uint;
impl Default for mii_bus {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
extern "C" {
    pub(crate) fn mdiobus_alloc_size(size: usize) -> *mut mii_bus;
}
extern "C" {
    pub(crate) fn __mdiobus_register(bus: *mut mii_bus, owner: *mut module) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn __devm_mdiobus_register(
        dev: *mut device,
        bus: *mut mii_bus,
        owner: *mut module,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mdiobus_unregister(bus: *mut mii_bus);
}
extern "C" {
    pub(crate) fn mdiobus_free(bus: *mut mii_bus);
}
extern "C" {
    pub(crate) fn devm_mdiobus_alloc_size(dev: *mut device, sizeof_priv: core::ffi::c_int)
        -> *mut mii_bus;
}
extern "C" {
    pub(crate) fn mdio_find_bus(mdio_name: *const core::ffi::c_char) -> *mut mii_bus;
}
extern "C" {
    pub(crate) fn mdiobus_scan(bus: *mut mii_bus, addr: core::ffi::c_int) -> *mut phy_device;
}
pub(crate) const phy_state_PHY_DOWN: phy_state = 0;
pub(crate) const phy_state_PHY_READY: phy_state = 1;
pub(crate) const phy_state_PHY_HALTED: phy_state = 2;
pub(crate) const phy_state_PHY_UP: phy_state = 3;
pub(crate) const phy_state_PHY_RUNNING: phy_state = 4;
pub(crate) const phy_state_PHY_NOLINK: phy_state = 5;
pub(crate) const phy_state_PHY_CABLETEST: phy_state = 6;
pub(crate) type phy_state = core::ffi::c_uint;
#[repr(C)]
#[derive(Default, Copy, Clone)]
pub(crate) struct phy_c45_device_ids {
    pub(crate) devices_in_package: u32_,
    pub(crate) mmds_present: u32_,
    pub(crate) device_ids: [u32_; 32usize],
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct phy_device {
    pub(crate) mdio: mdio_device,
    pub(crate) drv: *mut phy_driver,
    pub(crate) phy_id: u32_,
    pub(crate) c45_ids: phy_c45_device_ids,
    pub(crate) _bitfield_1: __BindgenBitfieldUnit<[u8; 3usize], u8>,
    pub(crate) rate_matching: core::ffi::c_int,
    pub(crate) state: phy_state,
    pub(crate) dev_flags: u32_,
    pub(crate) interface: phy_interface_t,
    pub(crate) speed: core::ffi::c_int,
    pub(crate) duplex: core::ffi::c_int,
    pub(crate) port: core::ffi::c_int,
    pub(crate) pause: core::ffi::c_int,
    pub(crate) asym_pause: core::ffi::c_int,
    pub(crate) master_slave_get: u8_,
    pub(crate) master_slave_set: u8_,
    pub(crate) master_slave_state: u8_,
    pub(crate) supported: [core::ffi::c_ulong; 3usize],
    pub(crate) advertising: [core::ffi::c_ulong; 3usize],
    pub(crate) lp_advertising: [core::ffi::c_ulong; 3usize],
    pub(crate) adv_old: [core::ffi::c_ulong; 3usize],
    pub(crate) host_interfaces: [core::ffi::c_ulong; 1usize],
    pub(crate) eee_broken_modes: u32_,
    pub(crate) irq: core::ffi::c_int,
    pub(crate) priv_: *mut core::ffi::c_void,
    pub(crate) shared: *mut phy_package_shared,
    pub(crate) skb: *mut sk_buff,
    pub(crate) ehdr: *mut core::ffi::c_void,
    pub(crate) nest: *mut nlattr,
    pub(crate) state_queue: delayed_work,
    pub(crate) lock: mutex,
    pub(crate) sfp_bus_attached: bool_,
    pub(crate) sfp_bus: *mut sfp_bus,
    pub(crate) phylink: *mut phylink,
    pub(crate) attached_dev: *mut net_device,
    pub(crate) mii_ts: *mut mii_timestamper,
    pub(crate) psec: *mut pse_control,
    pub(crate) mdix: u8_,
    pub(crate) mdix_ctrl: u8_,
    pub(crate) pma_extable: core::ffi::c_int,
    pub(crate) phy_link_change:
        ::core::option::Option<unsafe extern "C" fn(phydev: *mut phy_device, up: bool_)>,
    pub(crate) adjust_link: ::core::option::Option<unsafe extern "C" fn(dev: *mut net_device)>,
}
impl Default for phy_device {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
impl phy_device {
    #[inline]
    pub(crate) fn is_c45(&self) -> core::ffi::c_uint {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(0usize, 1u8) as u32) }
    }
    #[inline]
    pub(crate) fn set_is_c45(&mut self, val: core::ffi::c_uint) {
        unsafe {
            let val: u32 = ::core::mem::transmute(val);
            self._bitfield_1.set(0usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn is_internal(&self) -> core::ffi::c_uint {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(1usize, 1u8) as u32) }
    }
    #[inline]
    pub(crate) fn set_is_internal(&mut self, val: core::ffi::c_uint) {
        unsafe {
            let val: u32 = ::core::mem::transmute(val);
            self._bitfield_1.set(1usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn is_pseudo_fixed_link(&self) -> core::ffi::c_uint {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(2usize, 1u8) as u32) }
    }
    #[inline]
    pub(crate) fn set_is_pseudo_fixed_link(&mut self, val: core::ffi::c_uint) {
        unsafe {
            let val: u32 = ::core::mem::transmute(val);
            self._bitfield_1.set(2usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn is_gigabit_capable(&self) -> core::ffi::c_uint {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(3usize, 1u8) as u32) }
    }
    #[inline]
    pub(crate) fn set_is_gigabit_capable(&mut self, val: core::ffi::c_uint) {
        unsafe {
            let val: u32 = ::core::mem::transmute(val);
            self._bitfield_1.set(3usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn has_fixups(&self) -> core::ffi::c_uint {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(4usize, 1u8) as u32) }
    }
    #[inline]
    pub(crate) fn set_has_fixups(&mut self, val: core::ffi::c_uint) {
        unsafe {
            let val: u32 = ::core::mem::transmute(val);
            self._bitfield_1.set(4usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn suspended(&self) -> core::ffi::c_uint {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(5usize, 1u8) as u32) }
    }
    #[inline]
    pub(crate) fn set_suspended(&mut self, val: core::ffi::c_uint) {
        unsafe {
            let val: u32 = ::core::mem::transmute(val);
            self._bitfield_1.set(5usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn suspended_by_mdio_bus(&self) -> core::ffi::c_uint {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(6usize, 1u8) as u32) }
    }
    #[inline]
    pub(crate) fn set_suspended_by_mdio_bus(&mut self, val: core::ffi::c_uint) {
        unsafe {
            let val: u32 = ::core::mem::transmute(val);
            self._bitfield_1.set(6usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn sysfs_links(&self) -> core::ffi::c_uint {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(7usize, 1u8) as u32) }
    }
    #[inline]
    pub(crate) fn set_sysfs_links(&mut self, val: core::ffi::c_uint) {
        unsafe {
            let val: u32 = ::core::mem::transmute(val);
            self._bitfield_1.set(7usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn loopback_enabled(&self) -> core::ffi::c_uint {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(8usize, 1u8) as u32) }
    }
    #[inline]
    pub(crate) fn set_loopback_enabled(&mut self, val: core::ffi::c_uint) {
        unsafe {
            let val: u32 = ::core::mem::transmute(val);
            self._bitfield_1.set(8usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn downshifted_rate(&self) -> core::ffi::c_uint {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(9usize, 1u8) as u32) }
    }
    #[inline]
    pub(crate) fn set_downshifted_rate(&mut self, val: core::ffi::c_uint) {
        unsafe {
            let val: u32 = ::core::mem::transmute(val);
            self._bitfield_1.set(9usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn is_on_sfp_module(&self) -> core::ffi::c_uint {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(10usize, 1u8) as u32) }
    }
    #[inline]
    pub(crate) fn set_is_on_sfp_module(&mut self, val: core::ffi::c_uint) {
        unsafe {
            let val: u32 = ::core::mem::transmute(val);
            self._bitfield_1.set(10usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn mac_managed_pm(&self) -> core::ffi::c_uint {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(11usize, 1u8) as u32) }
    }
    #[inline]
    pub(crate) fn set_mac_managed_pm(&mut self, val: core::ffi::c_uint) {
        unsafe {
            let val: u32 = ::core::mem::transmute(val);
            self._bitfield_1.set(11usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn autoneg(&self) -> core::ffi::c_uint {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(12usize, 1u8) as u32) }
    }
    #[inline]
    pub(crate) fn set_autoneg(&mut self, val: core::ffi::c_uint) {
        unsafe {
            let val: u32 = ::core::mem::transmute(val);
            self._bitfield_1.set(12usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn link(&self) -> core::ffi::c_uint {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(13usize, 1u8) as u32) }
    }
    #[inline]
    pub(crate) fn set_link(&mut self, val: core::ffi::c_uint) {
        unsafe {
            let val: u32 = ::core::mem::transmute(val);
            self._bitfield_1.set(13usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn autoneg_complete(&self) -> core::ffi::c_uint {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(14usize, 1u8) as u32) }
    }
    #[inline]
    pub(crate) fn set_autoneg_complete(&mut self, val: core::ffi::c_uint) {
        unsafe {
            let val: u32 = ::core::mem::transmute(val);
            self._bitfield_1.set(14usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn interrupts(&self) -> core::ffi::c_uint {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(15usize, 1u8) as u32) }
    }
    #[inline]
    pub(crate) fn set_interrupts(&mut self, val: core::ffi::c_uint) {
        unsafe {
            let val: u32 = ::core::mem::transmute(val);
            self._bitfield_1.set(15usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn irq_suspended(&self) -> core::ffi::c_uint {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(16usize, 1u8) as u32) }
    }
    #[inline]
    pub(crate) fn set_irq_suspended(&mut self, val: core::ffi::c_uint) {
        unsafe {
            let val: u32 = ::core::mem::transmute(val);
            self._bitfield_1.set(16usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn irq_rerun(&self) -> core::ffi::c_uint {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(17usize, 1u8) as u32) }
    }
    #[inline]
    pub(crate) fn set_irq_rerun(&mut self, val: core::ffi::c_uint) {
        unsafe {
            let val: u32 = ::core::mem::transmute(val);
            self._bitfield_1.set(17usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn new_bitfield_1(
        is_c45: core::ffi::c_uint,
        is_internal: core::ffi::c_uint,
        is_pseudo_fixed_link: core::ffi::c_uint,
        is_gigabit_capable: core::ffi::c_uint,
        has_fixups: core::ffi::c_uint,
        suspended: core::ffi::c_uint,
        suspended_by_mdio_bus: core::ffi::c_uint,
        sysfs_links: core::ffi::c_uint,
        loopback_enabled: core::ffi::c_uint,
        downshifted_rate: core::ffi::c_uint,
        is_on_sfp_module: core::ffi::c_uint,
        mac_managed_pm: core::ffi::c_uint,
        autoneg: core::ffi::c_uint,
        link: core::ffi::c_uint,
        autoneg_complete: core::ffi::c_uint,
        interrupts: core::ffi::c_uint,
        irq_suspended: core::ffi::c_uint,
        irq_rerun: core::ffi::c_uint,
    ) -> __BindgenBitfieldUnit<[u8; 3usize], u8> {
        let mut __bindgen_bitfield_unit: __BindgenBitfieldUnit<[u8; 3usize], u8> =
            Default::default();
        __bindgen_bitfield_unit.set(0usize, 1u8, {
            let is_c45: u32 = unsafe { ::core::mem::transmute(is_c45) };
            is_c45 as u64
        });
        __bindgen_bitfield_unit.set(1usize, 1u8, {
            let is_internal: u32 = unsafe { ::core::mem::transmute(is_internal) };
            is_internal as u64
        });
        __bindgen_bitfield_unit.set(2usize, 1u8, {
            let is_pseudo_fixed_link: u32 = unsafe { ::core::mem::transmute(is_pseudo_fixed_link) };
            is_pseudo_fixed_link as u64
        });
        __bindgen_bitfield_unit.set(3usize, 1u8, {
            let is_gigabit_capable: u32 = unsafe { ::core::mem::transmute(is_gigabit_capable) };
            is_gigabit_capable as u64
        });
        __bindgen_bitfield_unit.set(4usize, 1u8, {
            let has_fixups: u32 = unsafe { ::core::mem::transmute(has_fixups) };
            has_fixups as u64
        });
        __bindgen_bitfield_unit.set(5usize, 1u8, {
            let suspended: u32 = unsafe { ::core::mem::transmute(suspended) };
            suspended as u64
        });
        __bindgen_bitfield_unit.set(6usize, 1u8, {
            let suspended_by_mdio_bus: u32 =
                unsafe { ::core::mem::transmute(suspended_by_mdio_bus) };
            suspended_by_mdio_bus as u64
        });
        __bindgen_bitfield_unit.set(7usize, 1u8, {
            let sysfs_links: u32 = unsafe { ::core::mem::transmute(sysfs_links) };
            sysfs_links as u64
        });
        __bindgen_bitfield_unit.set(8usize, 1u8, {
            let loopback_enabled: u32 = unsafe { ::core::mem::transmute(loopback_enabled) };
            loopback_enabled as u64
        });
        __bindgen_bitfield_unit.set(9usize, 1u8, {
            let downshifted_rate: u32 = unsafe { ::core::mem::transmute(downshifted_rate) };
            downshifted_rate as u64
        });
        __bindgen_bitfield_unit.set(10usize, 1u8, {
            let is_on_sfp_module: u32 = unsafe { ::core::mem::transmute(is_on_sfp_module) };
            is_on_sfp_module as u64
        });
        __bindgen_bitfield_unit.set(11usize, 1u8, {
            let mac_managed_pm: u32 = unsafe { ::core::mem::transmute(mac_managed_pm) };
            mac_managed_pm as u64
        });
        __bindgen_bitfield_unit.set(12usize, 1u8, {
            let autoneg: u32 = unsafe { ::core::mem::transmute(autoneg) };
            autoneg as u64
        });
        __bindgen_bitfield_unit.set(13usize, 1u8, {
            let link: u32 = unsafe { ::core::mem::transmute(link) };
            link as u64
        });
        __bindgen_bitfield_unit.set(14usize, 1u8, {
            let autoneg_complete: u32 = unsafe { ::core::mem::transmute(autoneg_complete) };
            autoneg_complete as u64
        });
        __bindgen_bitfield_unit.set(15usize, 1u8, {
            let interrupts: u32 = unsafe { ::core::mem::transmute(interrupts) };
            interrupts as u64
        });
        __bindgen_bitfield_unit.set(16usize, 1u8, {
            let irq_suspended: u32 = unsafe { ::core::mem::transmute(irq_suspended) };
            irq_suspended as u64
        });
        __bindgen_bitfield_unit.set(17usize, 1u8, {
            let irq_rerun: u32 = unsafe { ::core::mem::transmute(irq_rerun) };
            irq_rerun as u64
        });
        __bindgen_bitfield_unit
    }
}
#[repr(C)]
#[derive(Default, Copy, Clone)]
pub(crate) struct phy_tdr_config {
    pub(crate) first: u32_,
    pub(crate) last: u32_,
    pub(crate) step: u32_,
    pub(crate) pair: s8,
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct phy_driver {
    pub(crate) mdiodrv: mdio_driver_common,
    pub(crate) phy_id: u32_,
    pub(crate) name: *mut core::ffi::c_char,
    pub(crate) phy_id_mask: u32_,
    pub(crate) features: *const core::ffi::c_ulong,
    pub(crate) flags: u32_,
    pub(crate) driver_data: *const core::ffi::c_void,
    pub(crate) soft_reset:
        ::core::option::Option<unsafe extern "C" fn(phydev: *mut phy_device) -> core::ffi::c_int>,
    pub(crate) config_init:
        ::core::option::Option<unsafe extern "C" fn(phydev: *mut phy_device) -> core::ffi::c_int>,
    pub(crate) probe:
        ::core::option::Option<unsafe extern "C" fn(phydev: *mut phy_device) -> core::ffi::c_int>,
    pub(crate) get_features:
        ::core::option::Option<unsafe extern "C" fn(phydev: *mut phy_device) -> core::ffi::c_int>,
    pub(crate) get_rate_matching: ::core::option::Option<
        unsafe extern "C" fn(phydev: *mut phy_device, iface: phy_interface_t) -> core::ffi::c_int,
    >,
    pub(crate) suspend:
        ::core::option::Option<unsafe extern "C" fn(phydev: *mut phy_device) -> core::ffi::c_int>,
    pub(crate) resume:
        ::core::option::Option<unsafe extern "C" fn(phydev: *mut phy_device) -> core::ffi::c_int>,
    pub(crate) config_aneg:
        ::core::option::Option<unsafe extern "C" fn(phydev: *mut phy_device) -> core::ffi::c_int>,
    pub(crate) aneg_done:
        ::core::option::Option<unsafe extern "C" fn(phydev: *mut phy_device) -> core::ffi::c_int>,
    pub(crate) read_status:
        ::core::option::Option<unsafe extern "C" fn(phydev: *mut phy_device) -> core::ffi::c_int>,
    pub(crate) config_intr:
        ::core::option::Option<unsafe extern "C" fn(phydev: *mut phy_device) -> core::ffi::c_int>,
    pub(crate) handle_interrupt:
        ::core::option::Option<unsafe extern "C" fn(phydev: *mut phy_device) -> irqreturn_t>,
    pub(crate) remove: ::core::option::Option<unsafe extern "C" fn(phydev: *mut phy_device)>,
    pub(crate) match_phy_device:
        ::core::option::Option<unsafe extern "C" fn(phydev: *mut phy_device) -> core::ffi::c_int>,
    pub(crate) set_wol: ::core::option::Option<
        unsafe extern "C" fn(dev: *mut phy_device, wol: *mut ethtool_wolinfo) -> core::ffi::c_int,
    >,
    pub(crate) get_wol: ::core::option::Option<
        unsafe extern "C" fn(dev: *mut phy_device, wol: *mut ethtool_wolinfo),
    >,
    pub(crate) link_change_notify: ::core::option::Option<unsafe extern "C" fn(dev: *mut phy_device)>,
    pub(crate) read_mmd: ::core::option::Option<
        unsafe extern "C" fn(
            dev: *mut phy_device,
            devnum: core::ffi::c_int,
            regnum: u16_,
        ) -> core::ffi::c_int,
    >,
    pub(crate) write_mmd: ::core::option::Option<
        unsafe extern "C" fn(
            dev: *mut phy_device,
            devnum: core::ffi::c_int,
            regnum: u16_,
            val: u16_,
        ) -> core::ffi::c_int,
    >,
    pub(crate) read_page:
        ::core::option::Option<unsafe extern "C" fn(dev: *mut phy_device) -> core::ffi::c_int>,
    pub(crate) write_page: ::core::option::Option<
        unsafe extern "C" fn(dev: *mut phy_device, page: core::ffi::c_int) -> core::ffi::c_int,
    >,
    pub(crate) module_info: ::core::option::Option<
        unsafe extern "C" fn(
            dev: *mut phy_device,
            modinfo: *mut ethtool_modinfo,
        ) -> core::ffi::c_int,
    >,
    pub(crate) module_eeprom: ::core::option::Option<
        unsafe extern "C" fn(
            dev: *mut phy_device,
            ee: *mut ethtool_eeprom,
            data: *mut u8_,
        ) -> core::ffi::c_int,
    >,
    pub(crate) cable_test_start:
        ::core::option::Option<unsafe extern "C" fn(dev: *mut phy_device) -> core::ffi::c_int>,
    pub(crate) cable_test_tdr_start: ::core::option::Option<
        unsafe extern "C" fn(
            dev: *mut phy_device,
            config: *const phy_tdr_config,
        ) -> core::ffi::c_int,
    >,
    pub(crate) cable_test_get_status: ::core::option::Option<
        unsafe extern "C" fn(dev: *mut phy_device, finished: *mut bool_) -> core::ffi::c_int,
    >,
    pub(crate) get_sset_count:
        ::core::option::Option<unsafe extern "C" fn(dev: *mut phy_device) -> core::ffi::c_int>,
    pub(crate) get_strings:
        ::core::option::Option<unsafe extern "C" fn(dev: *mut phy_device, data: *mut u8_)>,
    pub(crate) get_stats: ::core::option::Option<
        unsafe extern "C" fn(dev: *mut phy_device, stats: *mut ethtool_stats, data: *mut u64_),
    >,
    pub(crate) get_tunable: ::core::option::Option<
        unsafe extern "C" fn(
            dev: *mut phy_device,
            tuna: *mut ethtool_tunable,
            data: *mut core::ffi::c_void,
        ) -> core::ffi::c_int,
    >,
    pub(crate) set_tunable: ::core::option::Option<
        unsafe extern "C" fn(
            dev: *mut phy_device,
            tuna: *mut ethtool_tunable,
            data: *const core::ffi::c_void,
        ) -> core::ffi::c_int,
    >,
    pub(crate) set_loopback: ::core::option::Option<
        unsafe extern "C" fn(dev: *mut phy_device, enable: bool_) -> core::ffi::c_int,
    >,
    pub(crate) get_sqi:
        ::core::option::Option<unsafe extern "C" fn(dev: *mut phy_device) -> core::ffi::c_int>,
    pub(crate) get_sqi_max:
        ::core::option::Option<unsafe extern "C" fn(dev: *mut phy_device) -> core::ffi::c_int>,
}
impl Default for phy_driver {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct phy_fixup {
    pub(crate) list: list_head,
    pub(crate) bus_id: [core::ffi::c_char; 64usize],
    pub(crate) phy_uid: u32_,
    pub(crate) phy_uid_mask: u32_,
    pub(crate) run:
        ::core::option::Option<unsafe extern "C" fn(phydev: *mut phy_device) -> core::ffi::c_int>,
}
impl Default for phy_fixup {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
extern "C" {
    pub(crate) fn phy_speed_to_str(speed: core::ffi::c_int) -> *const core::ffi::c_char;
}
extern "C" {
    pub(crate) fn phy_duplex_to_str(duplex: core::ffi::c_uint) -> *const core::ffi::c_char;
}
extern "C" {
    pub(crate) fn phy_rate_matching_to_str(rate_matching: core::ffi::c_int) -> *const core::ffi::c_char;
}
extern "C" {
    pub(crate) fn phy_interface_num_ports(interface: phy_interface_t) -> core::ffi::c_int;
}
#[repr(C)]
#[derive(Default, Copy, Clone)]
pub(crate) struct phy_setting {
    pub(crate) speed: u32_,
    pub(crate) duplex: u8_,
    pub(crate) bit: u8_,
}
extern "C" {
    pub(crate) fn phy_lookup_setting(
        speed: core::ffi::c_int,
        duplex: core::ffi::c_int,
        mask: *const core::ffi::c_ulong,
        exact: bool_,
    ) -> *const phy_setting;
}
extern "C" {
    pub(crate) fn phy_speeds(
        speeds: *mut core::ffi::c_uint,
        size: usize,
        mask: *mut core::ffi::c_ulong,
    ) -> usize;
}
extern "C" {
    pub(crate) fn of_set_phy_supported(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn of_set_phy_eee_broken(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn phy_speed_down_core(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_resolve_aneg_pause(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn phy_resolve_aneg_linkmode(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn phy_check_downshift(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn phy_read_mmd(
        phydev: *mut phy_device,
        devad: core::ffi::c_int,
        regnum: u32_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn __phy_read_mmd(
        phydev: *mut phy_device,
        devad: core::ffi::c_int,
        regnum: u32_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_write_mmd(
        phydev: *mut phy_device,
        devad: core::ffi::c_int,
        regnum: u32_,
        val: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn __phy_write_mmd(
        phydev: *mut phy_device,
        devad: core::ffi::c_int,
        regnum: u32_,
        val: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_modify_changed(
        phydev: *mut phy_device,
        regnum: u32_,
        mask: u16_,
        set: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn __phy_modify(
        phydev: *mut phy_device,
        regnum: u32_,
        mask: u16_,
        set: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_modify(
        phydev: *mut phy_device,
        regnum: u32_,
        mask: u16_,
        set: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn __phy_modify_mmd_changed(
        phydev: *mut phy_device,
        devad: core::ffi::c_int,
        regnum: u32_,
        mask: u16_,
        set: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_modify_mmd_changed(
        phydev: *mut phy_device,
        devad: core::ffi::c_int,
        regnum: u32_,
        mask: u16_,
        set: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn __phy_modify_mmd(
        phydev: *mut phy_device,
        devad: core::ffi::c_int,
        regnum: u32_,
        mask: u16_,
        set: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_modify_mmd(
        phydev: *mut phy_device,
        devad: core::ffi::c_int,
        regnum: u32_,
        mask: u16_,
        set: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_save_page(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_select_page(phydev: *mut phy_device, page: core::ffi::c_int) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_restore_page(
        phydev: *mut phy_device,
        oldpage: core::ffi::c_int,
        ret: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_read_paged(
        phydev: *mut phy_device,
        page: core::ffi::c_int,
        regnum: u32_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_write_paged(
        phydev: *mut phy_device,
        page: core::ffi::c_int,
        regnum: u32_,
        val: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_modify_paged_changed(
        phydev: *mut phy_device,
        page: core::ffi::c_int,
        regnum: u32_,
        mask: u16_,
        set: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_modify_paged(
        phydev: *mut phy_device,
        page: core::ffi::c_int,
        regnum: u32_,
        mask: u16_,
        set: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_device_create(
        bus: *mut mii_bus,
        addr: core::ffi::c_int,
        phy_id: u32_,
        is_c45: bool_,
        c45_ids: *mut phy_c45_device_ids,
    ) -> *mut phy_device;
}
extern "C" {
    pub(crate) fn fwnode_get_phy_id(fwnode: *mut fwnode_handle, phy_id: *mut u32_) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn fwnode_mdio_find_device(fwnode: *mut fwnode_handle) -> *mut mdio_device;
}
extern "C" {
    pub(crate) fn fwnode_phy_find_device(phy_fwnode: *mut fwnode_handle) -> *mut phy_device;
}
extern "C" {
    pub(crate) fn device_phy_find_device(dev: *mut device) -> *mut phy_device;
}
extern "C" {
    pub(crate) fn fwnode_get_phy_node(fwnode: *mut fwnode_handle) -> *mut fwnode_handle;
}
extern "C" {
    pub(crate) fn get_phy_device(
        bus: *mut mii_bus,
        addr: core::ffi::c_int,
        is_c45: bool_,
    ) -> *mut phy_device;
}
extern "C" {
    pub(crate) fn phy_device_register(phy: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_device_free(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn phy_device_remove(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn phy_get_c45_ids(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_init_hw(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_suspend(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_resume(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn __phy_resume(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_loopback(phydev: *mut phy_device, enable: bool_) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_sfp_attach(upstream: *mut core::ffi::c_void, bus: *mut sfp_bus);
}
extern "C" {
    pub(crate) fn phy_sfp_detach(upstream: *mut core::ffi::c_void, bus: *mut sfp_bus);
}
extern "C" {
    pub(crate) fn phy_sfp_probe(phydev: *mut phy_device, ops: *const sfp_upstream_ops)
        -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_attach(
        dev: *mut net_device,
        bus_id: *const core::ffi::c_char,
        interface: phy_interface_t,
    ) -> *mut phy_device;
}
extern "C" {
    pub(crate) fn phy_find_first(bus: *mut mii_bus) -> *mut phy_device;
}
extern "C" {
    pub(crate) fn phy_attach_direct(
        dev: *mut net_device,
        phydev: *mut phy_device,
        flags: u32_,
        interface: phy_interface_t,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_connect_direct(
        dev: *mut net_device,
        phydev: *mut phy_device,
        handler: ::core::option::Option<unsafe extern "C" fn(arg1: *mut net_device)>,
        interface: phy_interface_t,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_connect(
        dev: *mut net_device,
        bus_id: *const core::ffi::c_char,
        handler: ::core::option::Option<unsafe extern "C" fn(arg1: *mut net_device)>,
        interface: phy_interface_t,
    ) -> *mut phy_device;
}
extern "C" {
    pub(crate) fn phy_disconnect(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn phy_detach(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn phy_start(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn phy_stop(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn phy_config_aneg(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_start_aneg(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_aneg_done(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_speed_down(phydev: *mut phy_device, sync: bool_) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_speed_up(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_restart_aneg(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_reset_after_clk_enable(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_start_cable_test(
        phydev: *mut phy_device,
        extack: *mut netlink_ext_ack,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_start_cable_test_tdr(
        phydev: *mut phy_device,
        extack: *mut netlink_ext_ack,
        config: *const phy_tdr_config,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_cable_test_result(
        phydev: *mut phy_device,
        pair: u8_,
        result: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_cable_test_fault_length(
        phydev: *mut phy_device,
        pair: u8_,
        cm: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_attached_print(phydev: *mut phy_device, fmt: *const core::ffi::c_char, ...);
}
extern "C" {
    pub(crate) fn phy_attached_info_irq(phydev: *mut phy_device) -> *mut core::ffi::c_char;
}
extern "C" {
    pub(crate) fn phy_attached_info(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn genphy_read_abilities(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_setup_forced(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_restart_aneg(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_check_and_restart_aneg(
        phydev: *mut phy_device,
        restart: bool_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_config_eee_advert(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn __genphy_config_aneg(phydev: *mut phy_device, changed: bool_) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_aneg_done(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_update_link(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_read_lpa(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_read_status_fixed(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_read_status(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_read_master_slave(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_suspend(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_resume(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_loopback(phydev: *mut phy_device, enable: bool_) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_soft_reset(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_handle_interrupt_no_ack(phydev: *mut phy_device) -> irqreturn_t;
}
extern "C" {
    pub(crate) fn genphy_read_mmd_unsupported(
        phdev: *mut phy_device,
        devad: core::ffi::c_int,
        regnum: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_write_mmd_unsupported(
        phdev: *mut phy_device,
        devnum: core::ffi::c_int,
        regnum: u16_,
        val: u16_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c37_config_aneg(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c37_read_status(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c45_restart_aneg(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c45_check_and_restart_aneg(
        phydev: *mut phy_device,
        restart: bool_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c45_aneg_done(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c45_read_link(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c45_read_lpa(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c45_read_pma(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c45_pma_setup_forced(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c45_pma_baset1_setup_master_slave(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c45_an_config_aneg(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c45_an_disable_aneg(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c45_read_mdix(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c45_pma_read_abilities(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c45_pma_baset1_read_master_slave(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c45_read_status(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c45_baset1_read_status(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c45_config_aneg(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c45_loopback(phydev: *mut phy_device, enable: bool_) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c45_pma_resume(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c45_pma_suspend(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn genphy_c45_fast_retrain(phydev: *mut phy_device, enable: bool_) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) static mut genphy_c45_driver: phy_driver;
}
extern "C" {
    pub(crate) fn gen10g_config_aneg(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_driver_unregister(drv: *mut phy_driver);
}
extern "C" {
    pub(crate) fn phy_drivers_unregister(drv: *mut phy_driver, n: core::ffi::c_int);
}
extern "C" {
    pub(crate) fn phy_driver_register(new_driver: *mut phy_driver, owner: *mut module)
        -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_drivers_register(
        new_driver: *mut phy_driver,
        n: core::ffi::c_int,
        owner: *mut module,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_error(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn phy_state_machine(work: *mut work_struct);
}
extern "C" {
    pub(crate) fn phy_queue_state_machine(phydev: *mut phy_device, jiffies: core::ffi::c_ulong);
}
extern "C" {
    pub(crate) fn phy_trigger_machine(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn phy_mac_interrupt(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn phy_start_machine(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn phy_stop_machine(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn phy_ethtool_ksettings_get(phydev: *mut phy_device, cmd: *mut ethtool_link_ksettings);
}
extern "C" {
    pub(crate) fn phy_ethtool_ksettings_set(
        phydev: *mut phy_device,
        cmd: *const ethtool_link_ksettings,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_mii_ioctl(
        phydev: *mut phy_device,
        ifr: *mut ifreq,
        cmd: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_do_ioctl(
        dev: *mut net_device,
        ifr: *mut ifreq,
        cmd: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_do_ioctl_running(
        dev: *mut net_device,
        ifr: *mut ifreq,
        cmd: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_disable_interrupts(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_request_interrupt(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn phy_free_interrupt(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn phy_print_status(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn phy_get_rate_matching(
        phydev: *mut phy_device,
        iface: phy_interface_t,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_set_max_speed(phydev: *mut phy_device, max_speed: u32_);
}
extern "C" {
    pub(crate) fn phy_remove_link_mode(phydev: *mut phy_device, link_mode: u32_);
}
extern "C" {
    pub(crate) fn phy_advertise_supported(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn phy_support_sym_pause(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn phy_support_asym_pause(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn phy_set_sym_pause(phydev: *mut phy_device, rx: bool_, tx: bool_, autoneg: bool_);
}
extern "C" {
    pub(crate) fn phy_set_asym_pause(phydev: *mut phy_device, rx: bool_, tx: bool_);
}
extern "C" {
    pub(crate) fn phy_validate_pause(phydev: *mut phy_device, pp: *mut ethtool_pauseparam) -> bool_;
}
extern "C" {
    pub(crate) fn phy_get_pause(phydev: *mut phy_device, tx_pause: *mut bool_, rx_pause: *mut bool_);
}
extern "C" {
    pub(crate) fn phy_get_internal_delay(
        phydev: *mut phy_device,
        dev: *mut device,
        delay_values: *const core::ffi::c_int,
        size: core::ffi::c_int,
        is_rx: bool_,
    ) -> s32;
}
extern "C" {
    pub(crate) fn phy_resolve_pause(
        local_adv: *mut core::ffi::c_ulong,
        partner_adv: *mut core::ffi::c_ulong,
        tx_pause: *mut bool_,
        rx_pause: *mut bool_,
    );
}
extern "C" {
    pub(crate) fn phy_register_fixup(
        bus_id: *const core::ffi::c_char,
        phy_uid: u32_,
        phy_uid_mask: u32_,
        run: ::core::option::Option<
            unsafe extern "C" fn(arg1: *mut phy_device) -> core::ffi::c_int,
        >,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_register_fixup_for_id(
        bus_id: *const core::ffi::c_char,
        run: ::core::option::Option<
            unsafe extern "C" fn(arg1: *mut phy_device) -> core::ffi::c_int,
        >,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_register_fixup_for_uid(
        phy_uid: u32_,
        phy_uid_mask: u32_,
        run: ::core::option::Option<
            unsafe extern "C" fn(arg1: *mut phy_device) -> core::ffi::c_int,
        >,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_unregister_fixup(
        bus_id: *const core::ffi::c_char,
        phy_uid: u32_,
        phy_uid_mask: u32_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_unregister_fixup_for_id(bus_id: *const core::ffi::c_char) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_unregister_fixup_for_uid(phy_uid: u32_, phy_uid_mask: u32_) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_init_eee(phydev: *mut phy_device, clk_stop_enable: bool_) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_get_eee_err(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_ethtool_set_eee(phydev: *mut phy_device, data: *mut ethtool_eee)
        -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_ethtool_get_eee(phydev: *mut phy_device, data: *mut ethtool_eee)
        -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_ethtool_set_wol(
        phydev: *mut phy_device,
        wol: *mut ethtool_wolinfo,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_ethtool_get_wol(phydev: *mut phy_device, wol: *mut ethtool_wolinfo);
}
extern "C" {
    pub(crate) fn phy_ethtool_get_link_ksettings(
        ndev: *mut net_device,
        cmd: *mut ethtool_link_ksettings,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_ethtool_set_link_ksettings(
        ndev: *mut net_device,
        cmd: *const ethtool_link_ksettings,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_ethtool_nway_reset(ndev: *mut net_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_package_join(
        phydev: *mut phy_device,
        addr: core::ffi::c_int,
        priv_size: usize,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_package_leave(phydev: *mut phy_device);
}
extern "C" {
    pub(crate) fn devm_phy_package_join(
        dev: *mut device,
        phydev: *mut phy_device,
        addr: core::ffi::c_int,
        priv_size: usize,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mdio_bus_init() -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn mdio_bus_exit();
}
extern "C" {
    pub(crate) fn phy_ethtool_get_strings(phydev: *mut phy_device, data: *mut u8_) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_ethtool_get_sset_count(phydev: *mut phy_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_ethtool_get_stats(
        phydev: *mut phy_device,
        stats: *mut ethtool_stats,
        data: *mut u64_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) static mut mdio_bus_type: bus_type;
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct mdio_board_info {
    pub(crate) bus_id: *const core::ffi::c_char,
    pub(crate) modalias: [core::ffi::c_char; 32usize],
    pub(crate) mdio_addr: core::ffi::c_int,
    pub(crate) platform_data: *const core::ffi::c_void,
}
impl Default for mdio_board_info {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
extern "C" {
    pub(crate) fn mdiobus_register_board_info(
        info: *const mdio_board_info,
        n: core::ffi::c_uint,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn phy_driver_is_genphy(phydev: *mut phy_device) -> bool_;
}
extern "C" {
    pub(crate) fn phy_driver_is_genphy_10g(phydev: *mut phy_device) -> bool_;
}
extern "C" {
    pub(crate) fn ti_cm_get_macid(
        dev: *mut device,
        slave: core::ffi::c_int,
        mac_addr: *mut u8_,
    ) -> core::ffi::c_int;
}
pub(crate) const CPSW_PRIV: core::ffi::c_int = 1;
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct cpdma_params {
    pub(crate) dev: *mut device,
    pub(crate) dmaregs: *mut core::ffi::c_void,
    pub(crate) txhdp: *mut core::ffi::c_void,
    pub(crate) rxhdp: *mut core::ffi::c_void,
    pub(crate) txcp: *mut core::ffi::c_void,
    pub(crate) rxcp: *mut core::ffi::c_void,
    pub(crate) rxthresh: *mut core::ffi::c_void,
    pub(crate) rxfree: *mut core::ffi::c_void,
    pub(crate) num_chan: core::ffi::c_int,
    pub(crate) has_soft_reset: bool_,
    pub(crate) min_packet_size: core::ffi::c_int,
    pub(crate) desc_mem_phys: dma_addr_t,
    pub(crate) desc_hw_addr: dma_addr_t,
    pub(crate) desc_mem_size: core::ffi::c_int,
    pub(crate) desc_align: core::ffi::c_int,
    pub(crate) bus_freq_mhz: u32_,
    pub(crate) descs_pool_size: u32_,
    pub(crate) has_ext_regs: bool_,
}
impl Default for cpdma_params {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Default, Copy, Clone)]
pub(crate) struct cpdma_chan_stats {
    pub(crate) head_enqueue: u32_,
    pub(crate) tail_enqueue: u32_,
    pub(crate) pad_enqueue: u32_,
    pub(crate) misqueued: u32_,
    pub(crate) desc_alloc_fail: u32_,
    pub(crate) pad_alloc_fail: u32_,
    pub(crate) runt_receive_buff: u32_,
    pub(crate) runt_transmit_buff: u32_,
    pub(crate) empty_dequeue: u32_,
    pub(crate) busy_dequeue: u32_,
    pub(crate) good_dequeue: u32_,
    pub(crate) requeue: u32_,
    pub(crate) teardown_dequeue: u32_,
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct cpdma_ctlr {
    _unused: [u8; 0],
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct cpdma_chan {
    _unused: [u8; 0],
}
pub(crate) type cpdma_handler_fn = ::core::option::Option<
    unsafe extern "C" fn(
        token: *mut core::ffi::c_void,
        len: core::ffi::c_int,
        status: core::ffi::c_int,
    ),
>;
extern "C" {
    pub(crate) fn cpdma_ctlr_create(params: *mut cpdma_params) -> *mut cpdma_ctlr;
}
extern "C" {
    pub(crate) fn cpdma_ctlr_destroy(ctlr: *mut cpdma_ctlr) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpdma_ctlr_start(ctlr: *mut cpdma_ctlr) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpdma_ctlr_stop(ctlr: *mut cpdma_ctlr) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpdma_chan_create(
        ctlr: *mut cpdma_ctlr,
        chan_num: core::ffi::c_int,
        handler: cpdma_handler_fn,
        rx_type: core::ffi::c_int,
    ) -> *mut cpdma_chan;
}
extern "C" {
    pub(crate) fn cpdma_chan_get_rx_buf_num(chan: *mut cpdma_chan) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpdma_chan_destroy(chan: *mut cpdma_chan) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpdma_chan_start(chan: *mut cpdma_chan) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpdma_chan_stop(chan: *mut cpdma_chan) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpdma_chan_get_stats(
        chan: *mut cpdma_chan,
        stats: *mut cpdma_chan_stats,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpdma_chan_submit_mapped(
        chan: *mut cpdma_chan,
        token: *mut core::ffi::c_void,
        data: dma_addr_t,
        len: core::ffi::c_int,
        directed: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpdma_chan_submit(
        chan: *mut cpdma_chan,
        token: *mut core::ffi::c_void,
        data: *mut core::ffi::c_void,
        len: core::ffi::c_int,
        directed: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpdma_chan_idle_submit_mapped(
        chan: *mut cpdma_chan,
        token: *mut core::ffi::c_void,
        data: dma_addr_t,
        len: core::ffi::c_int,
        directed: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpdma_chan_idle_submit(
        chan: *mut cpdma_chan,
        token: *mut core::ffi::c_void,
        data: *mut core::ffi::c_void,
        len: core::ffi::c_int,
        directed: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpdma_chan_process(chan: *mut cpdma_chan, quota: core::ffi::c_int) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpdma_ctlr_int_ctrl(ctlr: *mut cpdma_ctlr, enable: bool_) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpdma_ctlr_eoi(ctlr: *mut cpdma_ctlr, value: u32_);
}
extern "C" {
    pub(crate) fn cpdma_chan_int_ctrl(chan: *mut cpdma_chan, enable: bool_) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpdma_ctrl_rxchs_state(ctlr: *mut cpdma_ctlr) -> u32_;
}
extern "C" {
    pub(crate) fn cpdma_ctrl_txchs_state(ctlr: *mut cpdma_ctlr) -> u32_;
}
extern "C" {
    pub(crate) fn cpdma_check_free_tx_desc(chan: *mut cpdma_chan) -> bool_;
}
extern "C" {
    pub(crate) fn cpdma_chan_set_weight(ch: *mut cpdma_chan, weight: core::ffi::c_int)
        -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpdma_chan_set_rate(ch: *mut cpdma_chan, rate: u32_) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpdma_chan_get_rate(ch: *mut cpdma_chan) -> u32_;
}
extern "C" {
    pub(crate) fn cpdma_chan_get_min_rate(ctlr: *mut cpdma_ctlr) -> u32_;
}
pub(crate) const cpdma_control_CPDMA_TX_RLIM: cpdma_control = 0;
pub(crate) const cpdma_control_CPDMA_CMD_IDLE: cpdma_control = 1;
pub(crate) const cpdma_control_CPDMA_COPY_ERROR_FRAMES: cpdma_control = 2;
pub(crate) const cpdma_control_CPDMA_RX_OFF_LEN_UPDATE: cpdma_control = 3;
pub(crate) const cpdma_control_CPDMA_RX_OWNERSHIP_FLIP: cpdma_control = 4;
pub(crate) const cpdma_control_CPDMA_TX_PRIO_FIXED: cpdma_control = 5;
pub(crate) const cpdma_control_CPDMA_STAT_IDLE: cpdma_control = 6;
pub(crate) const cpdma_control_CPDMA_STAT_TX_ERR_CHAN: cpdma_control = 7;
pub(crate) const cpdma_control_CPDMA_STAT_TX_ERR_CODE: cpdma_control = 8;
pub(crate) const cpdma_control_CPDMA_STAT_RX_ERR_CHAN: cpdma_control = 9;
pub(crate) const cpdma_control_CPDMA_STAT_RX_ERR_CODE: cpdma_control = 10;
pub(crate) const cpdma_control_CPDMA_RX_BUFFER_OFFSET: cpdma_control = 11;
pub(crate) type cpdma_control = core::ffi::c_uint;
extern "C" {
    pub(crate) fn cpdma_control_get(ctlr: *mut cpdma_ctlr, control: core::ffi::c_int) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpdma_control_set(
        ctlr: *mut cpdma_ctlr,
        control: core::ffi::c_int,
        value: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpdma_get_num_rx_descs(ctlr: *mut cpdma_ctlr) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpdma_set_num_rx_descs(
        ctlr: *mut cpdma_ctlr,
        num_rx_desc: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpdma_get_num_tx_descs(ctlr: *mut cpdma_ctlr) -> core::ffi::c_int;
}
pub(crate) const CPSW_RX_VLAN_ENCAP_HDR_PKT_VLAN_TAG: core::ffi::c_uint = 0;
pub(crate) const CPSW_RX_VLAN_ENCAP_HDR_PKT_RESERV: core::ffi::c_uint = 1;
pub(crate) const CPSW_RX_VLAN_ENCAP_HDR_PKT_PRIO_TAG: core::ffi::c_uint = 2;
pub(crate) const CPSW_RX_VLAN_ENCAP_HDR_PKT_UNTAG: core::ffi::c_uint = 3;
pub(crate) type _bindgen_ty_356 = core::ffi::c_uint;
#[repr(C)]
#[derive(Default, Copy, Clone)]
pub(crate) struct cpsw_wr_regs {
    pub(crate) id_ver: u32_,
    pub(crate) soft_reset: u32_,
    pub(crate) control: u32_,
    pub(crate) int_control: u32_,
    pub(crate) rx_thresh_en: u32_,
    pub(crate) rx_en: u32_,
    pub(crate) tx_en: u32_,
    pub(crate) misc_en: u32_,
    pub(crate) mem_allign1: [u32_; 8usize],
    pub(crate) rx_thresh_stat: u32_,
    pub(crate) rx_stat: u32_,
    pub(crate) tx_stat: u32_,
    pub(crate) misc_stat: u32_,
    pub(crate) mem_allign2: [u32_; 8usize],
    pub(crate) rx_imax: u32_,
    pub(crate) tx_imax: u32_,
}
#[repr(C)]
#[derive(Default, Copy, Clone)]
pub(crate) struct cpsw_ss_regs {
    pub(crate) id_ver: u32_,
    pub(crate) control: u32_,
    pub(crate) soft_reset: u32_,
    pub(crate) stat_port_en: u32_,
    pub(crate) ptype: u32_,
    pub(crate) soft_idle: u32_,
    pub(crate) thru_rate: u32_,
    pub(crate) gap_thresh: u32_,
    pub(crate) tx_start_wds: u32_,
    pub(crate) flow_control: u32_,
    pub(crate) vlan_ltype: u32_,
    pub(crate) ts_ltype: u32_,
    pub(crate) dlr_ltype: u32_,
}
#[repr(C)]
#[derive(Default, Copy, Clone)]
pub(crate) struct cpsw_host_regs {
    pub(crate) max_blks: u32_,
    pub(crate) blk_cnt: u32_,
    pub(crate) tx_in_ctl: u32_,
    pub(crate) port_vlan: u32_,
    pub(crate) tx_pri_map: u32_,
    pub(crate) cpdma_tx_pri_map: u32_,
    pub(crate) cpdma_rx_chan_map: u32_,
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct cpsw_slave_data {
    pub(crate) slave_node: *mut device_node,
    pub(crate) phy_node: *mut device_node,
    pub(crate) phy_id: [core::ffi::c_char; 61usize],
    pub(crate) phy_if: phy_interface_t,
    pub(crate) mac_addr: [u8_; 6usize],
    pub(crate) dual_emac_res_vlan: u16_,
    pub(crate) ifphy: *mut phy,
    pub(crate) disabled: bool_,
}
impl Default for cpsw_slave_data {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct cpsw_platform_data {
    pub(crate) slave_data: *mut cpsw_slave_data,
    pub(crate) ss_reg_ofs: u32_,
    pub(crate) channels: u32_,
    pub(crate) slaves: u32_,
    pub(crate) active_slave: u32_,
    pub(crate) bd_ram_size: u32_,
    pub(crate) mac_control: u32_,
    pub(crate) default_vlan: u16_,
    pub(crate) dual_emac: bool_,
}
impl Default for cpsw_platform_data {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct cpsw_slave {
    pub(crate) regs: *mut core::ffi::c_void,
    pub(crate) slave_num: core::ffi::c_int,
    pub(crate) mac_control: u32_,
    pub(crate) data: *mut cpsw_slave_data,
    pub(crate) phy: *mut phy_device,
    pub(crate) ndev: *mut net_device,
    pub(crate) port_vlan: u32_,
    pub(crate) mac_sl: *mut cpsw_sl,
}
impl Default for cpsw_slave {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct cpsw_vector {
    pub(crate) ch: *mut cpdma_chan,
    pub(crate) budget: core::ffi::c_int,
}
impl Default for cpsw_vector {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct cpsw_common {
    pub(crate) dev: *mut device,
    pub(crate) data: cpsw_platform_data,
    pub(crate) napi_rx: napi_struct,
    pub(crate) napi_tx: napi_struct,
    pub(crate) regs: *mut cpsw_ss_regs,
    pub(crate) wr_regs: *mut cpsw_wr_regs,
    pub(crate) hw_stats: *mut u8_,
    pub(crate) host_port_regs: *mut cpsw_host_regs,
    pub(crate) version: u32_,
    pub(crate) coal_intvl: u32_,
    pub(crate) bus_freq_mhz: u32_,
    pub(crate) rx_packet_max: core::ffi::c_int,
    pub(crate) descs_pool_size: core::ffi::c_int,
    pub(crate) slaves: *mut cpsw_slave,
    pub(crate) dma: *mut cpdma_ctlr,
    pub(crate) txv: [cpsw_vector; 8usize],
    pub(crate) rxv: [cpsw_vector; 8usize],
    pub(crate) ale: *mut cpsw_ale,
    pub(crate) quirk_irq: bool_,
    pub(crate) rx_irq_disabled: bool_,
    pub(crate) tx_irq_disabled: bool_,
    pub(crate) irqs_table: [u32_; 2usize],
    pub(crate) misc_irq: core::ffi::c_int,
    pub(crate) cpts: *mut cpts,
    pub(crate) devlink: *mut devlink,
    pub(crate) rx_ch_num: core::ffi::c_int,
    pub(crate) tx_ch_num: core::ffi::c_int,
    pub(crate) speed: core::ffi::c_int,
    pub(crate) usage_count: core::ffi::c_int,
    pub(crate) page_pool: [*mut page_pool; 8usize],
    pub(crate) br_members: u8_,
    pub(crate) hw_bridge_dev: *mut net_device,
    pub(crate) ale_bypass: bool_,
    pub(crate) base_mac: [u8_; 6usize],
}
impl Default for cpsw_common {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Default, Copy, Clone)]
pub(crate) struct cpsw_ale_ratelimit {
    pub(crate) cookie: core::ffi::c_ulong,
    pub(crate) rate_packet_ps: u64_,
}
#[repr(C)]
#[repr(align(64))]
#[derive(Copy, Clone)]
pub(crate) struct cpsw_priv {
    pub(crate) ndev: *mut net_device,
    pub(crate) dev: *mut device,
    pub(crate) msg_enable: u32_,
    pub(crate) mac_addr: [u8_; 6usize],
    pub(crate) rx_pause: bool_,
    pub(crate) tx_pause: bool_,
    pub(crate) mqprio_hw: bool_,
    pub(crate) fifo_bw: [core::ffi::c_int; 4usize],
    pub(crate) shp_cfg_speed: core::ffi::c_int,
    pub(crate) tx_ts_enabled: core::ffi::c_int,
    pub(crate) rx_ts_enabled: core::ffi::c_int,
    pub(crate) xdp_prog: *mut bpf_prog,
    pub(crate) __bindgen_padding_0: u64,
    pub(crate) xdp_rxq: [xdp_rxq_info; 8usize],
    pub(crate) xdpi: xdp_attachment_info,
    pub(crate) emac_port: u32_,
    pub(crate) cpsw: *mut cpsw_common,
    pub(crate) offload_fwd_mark: core::ffi::c_int,
    pub(crate) tx_packet_min: u32_,
    pub(crate) ale_bc_ratelimit: cpsw_ale_ratelimit,
    pub(crate) ale_mc_ratelimit: cpsw_ale_ratelimit,
}
impl Default for cpsw_priv {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
extern "C" {
    pub(crate) static mut cpsw_slave_index: ::core::option::Option<
        unsafe extern "C" fn(cpsw: *mut cpsw_common, priv_: *mut cpsw_priv) -> core::ffi::c_int,
    >;
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct addr_sync_ctx {
    pub(crate) ndev: *mut net_device,
    pub(crate) addr: *const u8_,
    pub(crate) consumed: core::ffi::c_int,
    pub(crate) flush: core::ffi::c_int,
}
impl Default for addr_sync_ctx {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct cpsw_meta_xdp {
    pub(crate) ndev: *mut net_device,
    pub(crate) ch: core::ffi::c_int,
}
impl Default for cpsw_meta_xdp {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
extern "C" {
    pub(crate) fn cpsw_init_common(
        cpsw: *mut cpsw_common,
        ss_regs: *mut core::ffi::c_void,
        ale_ageout: core::ffi::c_int,
        desc_mem_phys: phys_addr_t,
        descs_pool_size: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_split_res(cpsw: *mut cpsw_common);
}
extern "C" {
    pub(crate) fn cpsw_fill_rx_channels(priv_: *mut cpsw_priv) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_intr_enable(cpsw: *mut cpsw_common);
}
extern "C" {
    pub(crate) fn cpsw_intr_disable(cpsw: *mut cpsw_common);
}
extern "C" {
    pub(crate) fn cpsw_tx_handler(
        token: *mut core::ffi::c_void,
        len: core::ffi::c_int,
        status: core::ffi::c_int,
    );
}
extern "C" {
    pub(crate) fn cpsw_create_xdp_rxqs(cpsw: *mut cpsw_common) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_destroy_xdp_rxqs(cpsw: *mut cpsw_common);
}
extern "C" {
    pub(crate) fn cpsw_ndo_bpf(ndev: *mut net_device, bpf: *mut netdev_bpf) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_xdp_tx_frame(
        priv_: *mut cpsw_priv,
        xdpf: *mut xdp_frame,
        page: *mut page,
        port: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_run_xdp(
        priv_: *mut cpsw_priv,
        ch: core::ffi::c_int,
        xdp: *mut xdp_buff,
        page: *mut page,
        port: core::ffi::c_int,
        len: *mut core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_tx_interrupt(irq: core::ffi::c_int, dev_id: *mut core::ffi::c_void) -> irqreturn_t;
}
extern "C" {
    pub(crate) fn cpsw_rx_interrupt(irq: core::ffi::c_int, dev_id: *mut core::ffi::c_void) -> irqreturn_t;
}
extern "C" {
    pub(crate) fn cpsw_misc_interrupt(
        irq: core::ffi::c_int,
        dev_id: *mut core::ffi::c_void,
    ) -> irqreturn_t;
}
extern "C" {
    pub(crate) fn cpsw_tx_mq_poll(napi_tx: *mut napi_struct, budget: core::ffi::c_int)
        -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_tx_poll(napi_tx: *mut napi_struct, budget: core::ffi::c_int) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_rx_mq_poll(napi_rx: *mut napi_struct, budget: core::ffi::c_int)
        -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_rx_poll(napi_rx: *mut napi_struct, budget: core::ffi::c_int) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_rx_vlan_encap(skb: *mut sk_buff);
}
extern "C" {
    pub(crate) fn soft_reset(module: *const core::ffi::c_char, reg: *mut core::ffi::c_void);
}
extern "C" {
    pub(crate) fn cpsw_set_slave_mac(slave: *mut cpsw_slave, priv_: *mut cpsw_priv);
}
extern "C" {
    pub(crate) fn cpsw_ndo_tx_timeout(ndev: *mut net_device, txqueue: core::ffi::c_uint);
}
extern "C" {
    pub(crate) fn cpsw_need_resplit(cpsw: *mut cpsw_common) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_ndo_ioctl(
        dev: *mut net_device,
        req: *mut ifreq,
        cmd: core::ffi::c_int,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_ndo_set_tx_maxrate(
        ndev: *mut net_device,
        queue: core::ffi::c_int,
        rate: u32_,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_ndo_setup_tc(
        ndev: *mut net_device,
        type_: tc_setup_type,
        type_data: *mut core::ffi::c_void,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_shp_is_off(priv_: *mut cpsw_priv) -> bool_;
}
extern "C" {
    pub(crate) fn cpsw_cbs_resume(slave: *mut cpsw_slave, priv_: *mut cpsw_priv);
}
extern "C" {
    pub(crate) fn cpsw_mqprio_resume(slave: *mut cpsw_slave, priv_: *mut cpsw_priv);
}
extern "C" {
    pub(crate) fn cpsw_qos_clsflower_resume(priv_: *mut cpsw_priv);
}
extern "C" {
    pub(crate) fn cpsw_get_msglevel(ndev: *mut net_device) -> u32_;
}
extern "C" {
    pub(crate) fn cpsw_set_msglevel(ndev: *mut net_device, value: u32_);
}
extern "C" {
    pub(crate) fn cpsw_get_coalesce(
        ndev: *mut net_device,
        coal: *mut ethtool_coalesce,
        kernel_coal: *mut kernel_ethtool_coalesce,
        extack: *mut netlink_ext_ack,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_set_coalesce(
        ndev: *mut net_device,
        coal: *mut ethtool_coalesce,
        kernel_coal: *mut kernel_ethtool_coalesce,
        extack: *mut netlink_ext_ack,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_get_sset_count(ndev: *mut net_device, sset: core::ffi::c_int) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_get_strings(ndev: *mut net_device, stringset: u32_, data: *mut u8_);
}
extern "C" {
    pub(crate) fn cpsw_get_ethtool_stats(
        ndev: *mut net_device,
        stats: *mut ethtool_stats,
        data: *mut u64_,
    );
}
extern "C" {
    pub(crate) fn cpsw_get_pauseparam(ndev: *mut net_device, pause: *mut ethtool_pauseparam);
}
extern "C" {
    pub(crate) fn cpsw_get_wol(ndev: *mut net_device, wol: *mut ethtool_wolinfo);
}
extern "C" {
    pub(crate) fn cpsw_set_wol(ndev: *mut net_device, wol: *mut ethtool_wolinfo) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_get_regs_len(ndev: *mut net_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_get_regs(ndev: *mut net_device, regs: *mut ethtool_regs, p: *mut core::ffi::c_void);
}
extern "C" {
    pub(crate) fn cpsw_ethtool_op_begin(ndev: *mut net_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_ethtool_op_complete(ndev: *mut net_device);
}
extern "C" {
    pub(crate) fn cpsw_get_channels(ndev: *mut net_device, ch: *mut ethtool_channels);
}
extern "C" {
    pub(crate) fn cpsw_get_link_ksettings(
        ndev: *mut net_device,
        ecmd: *mut ethtool_link_ksettings,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_set_link_ksettings(
        ndev: *mut net_device,
        ecmd: *const ethtool_link_ksettings,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_get_eee(ndev: *mut net_device, edata: *mut ethtool_eee) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_set_eee(ndev: *mut net_device, edata: *mut ethtool_eee) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_nway_reset(ndev: *mut net_device) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_get_ringparam(
        ndev: *mut net_device,
        ering: *mut ethtool_ringparam,
        kernel_ering: *mut kernel_ethtool_ringparam,
        extack: *mut netlink_ext_ack,
    );
}
extern "C" {
    pub(crate) fn cpsw_set_ringparam(
        ndev: *mut net_device,
        ering: *mut ethtool_ringparam,
        kernel_ering: *mut kernel_ethtool_ringparam,
        extack: *mut netlink_ext_ack,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_set_channels_common(
        ndev: *mut net_device,
        chs: *mut ethtool_channels,
        rx_handler: cpdma_handler_fn,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_get_ts_info(ndev: *mut net_device, info: *mut ethtool_ts_info) -> core::ffi::c_int;
}
pub(crate) const CPSW_SL: core::ffi::c_int = 1;
pub(crate) const cpsw_sl_regs_CPSW_SL_IDVER: cpsw_sl_regs = 0;
pub(crate) const cpsw_sl_regs_CPSW_SL_MACCONTROL: cpsw_sl_regs = 1;
pub(crate) const cpsw_sl_regs_CPSW_SL_MACSTATUS: cpsw_sl_regs = 2;
pub(crate) const cpsw_sl_regs_CPSW_SL_SOFT_RESET: cpsw_sl_regs = 3;
pub(crate) const cpsw_sl_regs_CPSW_SL_RX_MAXLEN: cpsw_sl_regs = 4;
pub(crate) const cpsw_sl_regs_CPSW_SL_BOFFTEST: cpsw_sl_regs = 5;
pub(crate) const cpsw_sl_regs_CPSW_SL_RX_PAUSE: cpsw_sl_regs = 6;
pub(crate) const cpsw_sl_regs_CPSW_SL_TX_PAUSE: cpsw_sl_regs = 7;
pub(crate) const cpsw_sl_regs_CPSW_SL_EMCONTROL: cpsw_sl_regs = 8;
pub(crate) const cpsw_sl_regs_CPSW_SL_RX_PRI_MAP: cpsw_sl_regs = 9;
pub(crate) const cpsw_sl_regs_CPSW_SL_TX_GAP: cpsw_sl_regs = 10;
pub(crate) type cpsw_sl_regs = core::ffi::c_uint;
pub(crate) const CPSW_SL_CTL_FULLDUPLEX: core::ffi::c_uint = 1;
pub(crate) const CPSW_SL_CTL_LOOPBACK: core::ffi::c_uint = 2;
pub(crate) const CPSW_SL_CTL_MTEST: core::ffi::c_uint = 4;
pub(crate) const CPSW_SL_CTL_RX_FLOW_EN: core::ffi::c_uint = 8;
pub(crate) const CPSW_SL_CTL_TX_FLOW_EN: core::ffi::c_uint = 16;
pub(crate) const CPSW_SL_CTL_GMII_EN: core::ffi::c_uint = 32;
pub(crate) const CPSW_SL_CTL_TX_PACE: core::ffi::c_uint = 64;
pub(crate) const CPSW_SL_CTL_GIG: core::ffi::c_uint = 128;
pub(crate) const CPSW_SL_CTL_XGIG: core::ffi::c_uint = 256;
pub(crate) const CPSW_SL_CTL_TX_SHORT_GAP_EN: core::ffi::c_uint = 1024;
pub(crate) const CPSW_SL_CTL_CMD_IDLE: core::ffi::c_uint = 2048;
pub(crate) const CPSW_SL_CTL_CRC_TYPE: core::ffi::c_uint = 4096;
pub(crate) const CPSW_SL_CTL_XGMII_EN: core::ffi::c_uint = 8192;
pub(crate) const CPSW_SL_CTL_IFCTL_A: core::ffi::c_uint = 32768;
pub(crate) const CPSW_SL_CTL_IFCTL_B: core::ffi::c_uint = 65536;
pub(crate) const CPSW_SL_CTL_GIG_FORCE: core::ffi::c_uint = 131072;
pub(crate) const CPSW_SL_CTL_EXT_EN: core::ffi::c_uint = 262144;
pub(crate) const CPSW_SL_CTL_EXT_EN_RX_FLO: core::ffi::c_uint = 524288;
pub(crate) const CPSW_SL_CTL_EXT_EN_TX_FLO: core::ffi::c_uint = 1048576;
pub(crate) const CPSW_SL_CTL_TX_SG_LIM_EN: core::ffi::c_uint = 2097152;
pub(crate) const CPSW_SL_CTL_RX_CEF_EN: core::ffi::c_uint = 4194304;
pub(crate) const CPSW_SL_CTL_RX_CSF_EN: core::ffi::c_uint = 8388608;
pub(crate) const CPSW_SL_CTL_RX_CMF_EN: core::ffi::c_uint = 16777216;
pub(crate) const CPSW_SL_CTL_EXT_EN_XGIG: core::ffi::c_uint = 33554432;
pub(crate) const CPSW_SL_CTL_FUNCS_COUNT: core::ffi::c_uint = 33554433;
pub(crate) type _bindgen_ty_357 = core::ffi::c_uint;
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct cpsw_sl {
    _unused: [u8; 0],
}
extern "C" {
    pub(crate) fn cpsw_sl_get(
        device_id: *const core::ffi::c_char,
        dev: *mut device,
        sl_base: *mut core::ffi::c_void,
    ) -> *mut cpsw_sl;
}
extern "C" {
    pub(crate) fn cpsw_sl_reset(sl: *mut cpsw_sl, tmo: core::ffi::c_ulong);
}
extern "C" {
    pub(crate) fn cpsw_sl_ctl_set(sl: *mut cpsw_sl, ctl_funcs: u32_) -> u32_;
}
extern "C" {
    pub(crate) fn cpsw_sl_ctl_clr(sl: *mut cpsw_sl, ctl_funcs: u32_) -> u32_;
}
extern "C" {
    pub(crate) fn cpsw_sl_ctl_reset(sl: *mut cpsw_sl);
}
extern "C" {
    pub(crate) fn cpsw_sl_wait_for_idle(sl: *mut cpsw_sl, tmo: core::ffi::c_ulong) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_sl_reg_read(sl: *mut cpsw_sl, reg: cpsw_sl_regs) -> u32_;
}
extern "C" {
    pub(crate) fn cpsw_sl_reg_write(sl: *mut cpsw_sl, reg: cpsw_sl_regs, val: u32_);
}
pub(crate) const CPSW_SWITCHDEV: core::ffi::c_int = 1;
pub(crate) const switchdev_attr_id_SWITCHDEV_ATTR_ID_UNDEFINED: switchdev_attr_id = 0;
pub(crate) const switchdev_attr_id_SWITCHDEV_ATTR_ID_PORT_STP_STATE: switchdev_attr_id = 1;
pub(crate) const switchdev_attr_id_SWITCHDEV_ATTR_ID_PORT_MST_STATE: switchdev_attr_id = 2;
pub(crate) const switchdev_attr_id_SWITCHDEV_ATTR_ID_PORT_BRIDGE_FLAGS: switchdev_attr_id = 3;
pub(crate) const switchdev_attr_id_SWITCHDEV_ATTR_ID_PORT_PRE_BRIDGE_FLAGS: switchdev_attr_id = 4;
pub(crate) const switchdev_attr_id_SWITCHDEV_ATTR_ID_PORT_MROUTER: switchdev_attr_id = 5;
pub(crate) const switchdev_attr_id_SWITCHDEV_ATTR_ID_BRIDGE_AGEING_TIME: switchdev_attr_id = 6;
pub(crate) const switchdev_attr_id_SWITCHDEV_ATTR_ID_BRIDGE_VLAN_FILTERING: switchdev_attr_id = 7;
pub(crate) const switchdev_attr_id_SWITCHDEV_ATTR_ID_BRIDGE_VLAN_PROTOCOL: switchdev_attr_id = 8;
pub(crate) const switchdev_attr_id_SWITCHDEV_ATTR_ID_BRIDGE_MC_DISABLED: switchdev_attr_id = 9;
pub(crate) const switchdev_attr_id_SWITCHDEV_ATTR_ID_BRIDGE_MROUTER: switchdev_attr_id = 10;
pub(crate) const switchdev_attr_id_SWITCHDEV_ATTR_ID_BRIDGE_MST: switchdev_attr_id = 11;
pub(crate) const switchdev_attr_id_SWITCHDEV_ATTR_ID_MRP_PORT_ROLE: switchdev_attr_id = 12;
pub(crate) const switchdev_attr_id_SWITCHDEV_ATTR_ID_VLAN_MSTI: switchdev_attr_id = 13;
pub(crate) type switchdev_attr_id = core::ffi::c_uint;
#[repr(C)]
#[derive(Default, Copy, Clone)]
pub(crate) struct switchdev_mst_state {
    pub(crate) msti: u16_,
    pub(crate) state: u8_,
}
#[repr(C)]
#[derive(Default, Copy, Clone)]
pub(crate) struct switchdev_brport_flags {
    pub(crate) val: core::ffi::c_ulong,
    pub(crate) mask: core::ffi::c_ulong,
}
#[repr(C)]
#[derive(Default, Copy, Clone)]
pub(crate) struct switchdev_vlan_msti {
    pub(crate) vid: u16_,
    pub(crate) msti: u16_,
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct switchdev_attr {
    pub(crate) orig_dev: *mut net_device,
    pub(crate) id: switchdev_attr_id,
    pub(crate) flags: u32_,
    pub(crate) complete_priv: *mut core::ffi::c_void,
    pub(crate) complete: ::core::option::Option<
        unsafe extern "C" fn(
            dev: *mut net_device,
            err: core::ffi::c_int,
            priv_: *mut core::ffi::c_void,
        ),
    >,
    pub(crate) u: switchdev_attr__bindgen_ty_1,
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) union switchdev_attr__bindgen_ty_1 {
    pub(crate) stp_state: u8_,
    pub(crate) mst_state: switchdev_mst_state,
    pub(crate) brport_flags: switchdev_brport_flags,
    pub(crate) mrouter: bool_,
    pub(crate) ageing_time: clock_t,
    pub(crate) vlan_filtering: bool_,
    pub(crate) vlan_protocol: u16_,
    pub(crate) mst: bool_,
    pub(crate) mc_disabled: bool_,
    pub(crate) mrp_port_role: u8_,
    pub(crate) vlan_msti: switchdev_vlan_msti,
    _bindgen_union_align: [u32; 2usize],
}
impl Default for switchdev_attr__bindgen_ty_1 {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
impl Default for switchdev_attr {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
pub(crate) const switchdev_obj_id_SWITCHDEV_OBJ_ID_UNDEFINED: switchdev_obj_id = 0;
pub(crate) const switchdev_obj_id_SWITCHDEV_OBJ_ID_PORT_VLAN: switchdev_obj_id = 1;
pub(crate) const switchdev_obj_id_SWITCHDEV_OBJ_ID_PORT_MDB: switchdev_obj_id = 2;
pub(crate) const switchdev_obj_id_SWITCHDEV_OBJ_ID_HOST_MDB: switchdev_obj_id = 3;
pub(crate) const switchdev_obj_id_SWITCHDEV_OBJ_ID_MRP: switchdev_obj_id = 4;
pub(crate) const switchdev_obj_id_SWITCHDEV_OBJ_ID_RING_TEST_MRP: switchdev_obj_id = 5;
pub(crate) const switchdev_obj_id_SWITCHDEV_OBJ_ID_RING_ROLE_MRP: switchdev_obj_id = 6;
pub(crate) const switchdev_obj_id_SWITCHDEV_OBJ_ID_RING_STATE_MRP: switchdev_obj_id = 7;
pub(crate) const switchdev_obj_id_SWITCHDEV_OBJ_ID_IN_TEST_MRP: switchdev_obj_id = 8;
pub(crate) const switchdev_obj_id_SWITCHDEV_OBJ_ID_IN_ROLE_MRP: switchdev_obj_id = 9;
pub(crate) const switchdev_obj_id_SWITCHDEV_OBJ_ID_IN_STATE_MRP: switchdev_obj_id = 10;
pub(crate) type switchdev_obj_id = core::ffi::c_uint;
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct switchdev_obj {
    pub(crate) list: list_head,
    pub(crate) orig_dev: *mut net_device,
    pub(crate) id: switchdev_obj_id,
    pub(crate) flags: u32_,
    pub(crate) complete_priv: *mut core::ffi::c_void,
    pub(crate) complete: ::core::option::Option<
        unsafe extern "C" fn(
            dev: *mut net_device,
            err: core::ffi::c_int,
            priv_: *mut core::ffi::c_void,
        ),
    >,
}
impl Default for switchdev_obj {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct switchdev_obj_port_vlan {
    pub(crate) obj: switchdev_obj,
    pub(crate) flags: u16_,
    pub(crate) vid: u16_,
    pub(crate) changed: bool_,
}
impl Default for switchdev_obj_port_vlan {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct switchdev_obj_port_mdb {
    pub(crate) obj: switchdev_obj,
    pub(crate) addr: [core::ffi::c_uchar; 6usize],
    pub(crate) vid: u16_,
}
impl Default for switchdev_obj_port_mdb {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct switchdev_obj_mrp {
    pub(crate) obj: switchdev_obj,
    pub(crate) p_port: *mut net_device,
    pub(crate) s_port: *mut net_device,
    pub(crate) ring_id: u32_,
    pub(crate) prio: u16_,
}
impl Default for switchdev_obj_mrp {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct switchdev_obj_ring_test_mrp {
    pub(crate) obj: switchdev_obj,
    pub(crate) interval: u32_,
    pub(crate) max_miss: u8_,
    pub(crate) ring_id: u32_,
    pub(crate) period: u32_,
    pub(crate) monitor: bool_,
}
impl Default for switchdev_obj_ring_test_mrp {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct switchdev_obj_ring_role_mrp {
    pub(crate) obj: switchdev_obj,
    pub(crate) ring_role: u8_,
    pub(crate) ring_id: u32_,
    pub(crate) sw_backup: u8_,
}
impl Default for switchdev_obj_ring_role_mrp {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct switchdev_obj_ring_state_mrp {
    pub(crate) obj: switchdev_obj,
    pub(crate) ring_state: u8_,
    pub(crate) ring_id: u32_,
}
impl Default for switchdev_obj_ring_state_mrp {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct switchdev_obj_in_test_mrp {
    pub(crate) obj: switchdev_obj,
    pub(crate) interval: u32_,
    pub(crate) in_id: u32_,
    pub(crate) period: u32_,
    pub(crate) max_miss: u8_,
}
impl Default for switchdev_obj_in_test_mrp {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct switchdev_obj_in_role_mrp {
    pub(crate) obj: switchdev_obj,
    pub(crate) i_port: *mut net_device,
    pub(crate) ring_id: u32_,
    pub(crate) in_id: u16_,
    pub(crate) in_role: u8_,
    pub(crate) sw_backup: u8_,
}
impl Default for switchdev_obj_in_role_mrp {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct switchdev_obj_in_state_mrp {
    pub(crate) obj: switchdev_obj,
    pub(crate) in_id: u32_,
    pub(crate) in_state: u8_,
}
impl Default for switchdev_obj_in_state_mrp {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
pub(crate) type switchdev_obj_dump_cb_t =
    ::core::option::Option<unsafe extern "C" fn(obj: *mut switchdev_obj) -> core::ffi::c_int>;
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct switchdev_brport {
    pub(crate) dev: *mut net_device,
    pub(crate) ctx: *const core::ffi::c_void,
    pub(crate) atomic_nb: *mut notifier_block,
    pub(crate) blocking_nb: *mut notifier_block,
    pub(crate) tx_fwd_offload: bool_,
}
impl Default for switchdev_brport {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
pub(crate) const switchdev_notifier_type_SWITCHDEV_FDB_ADD_TO_BRIDGE: switchdev_notifier_type = 1;
pub(crate) const switchdev_notifier_type_SWITCHDEV_FDB_DEL_TO_BRIDGE: switchdev_notifier_type = 2;
pub(crate) const switchdev_notifier_type_SWITCHDEV_FDB_ADD_TO_DEVICE: switchdev_notifier_type = 3;
pub(crate) const switchdev_notifier_type_SWITCHDEV_FDB_DEL_TO_DEVICE: switchdev_notifier_type = 4;
pub(crate) const switchdev_notifier_type_SWITCHDEV_FDB_OFFLOADED: switchdev_notifier_type = 5;
pub(crate) const switchdev_notifier_type_SWITCHDEV_FDB_FLUSH_TO_BRIDGE: switchdev_notifier_type = 6;
pub(crate) const switchdev_notifier_type_SWITCHDEV_PORT_OBJ_ADD: switchdev_notifier_type = 7;
pub(crate) const switchdev_notifier_type_SWITCHDEV_PORT_OBJ_DEL: switchdev_notifier_type = 8;
pub(crate) const switchdev_notifier_type_SWITCHDEV_PORT_ATTR_SET: switchdev_notifier_type = 9;
pub(crate) const switchdev_notifier_type_SWITCHDEV_VXLAN_FDB_ADD_TO_BRIDGE: switchdev_notifier_type = 10;
pub(crate) const switchdev_notifier_type_SWITCHDEV_VXLAN_FDB_DEL_TO_BRIDGE: switchdev_notifier_type = 11;
pub(crate) const switchdev_notifier_type_SWITCHDEV_VXLAN_FDB_ADD_TO_DEVICE: switchdev_notifier_type = 12;
pub(crate) const switchdev_notifier_type_SWITCHDEV_VXLAN_FDB_DEL_TO_DEVICE: switchdev_notifier_type = 13;
pub(crate) const switchdev_notifier_type_SWITCHDEV_VXLAN_FDB_OFFLOADED: switchdev_notifier_type = 14;
pub(crate) const switchdev_notifier_type_SWITCHDEV_BRPORT_OFFLOADED: switchdev_notifier_type = 15;
pub(crate) const switchdev_notifier_type_SWITCHDEV_BRPORT_UNOFFLOADED: switchdev_notifier_type = 16;
pub(crate) type switchdev_notifier_type = core::ffi::c_uint;
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct switchdev_notifier_info {
    pub(crate) dev: *mut net_device,
    pub(crate) extack: *mut netlink_ext_ack,
    pub(crate) ctx: *const core::ffi::c_void,
}
impl Default for switchdev_notifier_info {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct switchdev_notifier_fdb_info {
    pub(crate) info: switchdev_notifier_info,
    pub(crate) addr: *const core::ffi::c_uchar,
    pub(crate) vid: u16_,
    pub(crate) _bitfield_1: __BindgenBitfieldUnit<[u8; 1usize], u8>,
    pub(crate) __bindgen_padding_0: u8,
}
impl Default for switchdev_notifier_fdb_info {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
impl switchdev_notifier_fdb_info {
    #[inline]
    pub(crate) fn added_by_user(&self) -> u8_ {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(0usize, 1u8) as u8) }
    }
    #[inline]
    pub(crate) fn set_added_by_user(&mut self, val: u8_) {
        unsafe {
            let val: u8 = ::core::mem::transmute(val);
            self._bitfield_1.set(0usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn is_local(&self) -> u8_ {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(1usize, 1u8) as u8) }
    }
    #[inline]
    pub(crate) fn set_is_local(&mut self, val: u8_) {
        unsafe {
            let val: u8 = ::core::mem::transmute(val);
            self._bitfield_1.set(1usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn offloaded(&self) -> u8_ {
        unsafe { ::core::mem::transmute(self._bitfield_1.get(2usize, 1u8) as u8) }
    }
    #[inline]
    pub(crate) fn set_offloaded(&mut self, val: u8_) {
        unsafe {
            let val: u8 = ::core::mem::transmute(val);
            self._bitfield_1.set(2usize, 1u8, val as u64)
        }
    }
    #[inline]
    pub(crate) fn new_bitfield_1(
        added_by_user: u8_,
        is_local: u8_,
        offloaded: u8_,
    ) -> __BindgenBitfieldUnit<[u8; 1usize], u8> {
        let mut __bindgen_bitfield_unit: __BindgenBitfieldUnit<[u8; 1usize], u8> =
            Default::default();
        __bindgen_bitfield_unit.set(0usize, 1u8, {
            let added_by_user: u8 = unsafe { ::core::mem::transmute(added_by_user) };
            added_by_user as u64
        });
        __bindgen_bitfield_unit.set(1usize, 1u8, {
            let is_local: u8 = unsafe { ::core::mem::transmute(is_local) };
            is_local as u64
        });
        __bindgen_bitfield_unit.set(2usize, 1u8, {
            let offloaded: u8 = unsafe { ::core::mem::transmute(offloaded) };
            offloaded as u64
        });
        __bindgen_bitfield_unit
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct switchdev_notifier_port_obj_info {
    pub(crate) info: switchdev_notifier_info,
    pub(crate) obj: *const switchdev_obj,
    pub(crate) handled: bool_,
}
impl Default for switchdev_notifier_port_obj_info {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct switchdev_notifier_port_attr_info {
    pub(crate) info: switchdev_notifier_info,
    pub(crate) attr: *const switchdev_attr,
    pub(crate) handled: bool_,
}
impl Default for switchdev_notifier_port_attr_info {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct switchdev_notifier_brport_info {
    pub(crate) info: switchdev_notifier_info,
    pub(crate) brport: switchdev_brport,
}
impl Default for switchdev_notifier_brport_info {
    fn default() -> Self {
        unsafe { ::core::mem::zeroed() }
    }
}
extern "C" {
    pub(crate) fn switchdev_bridge_port_offload(
        brport_dev: *mut net_device,
        dev: *mut net_device,
        ctx: *const core::ffi::c_void,
        atomic_nb: *mut notifier_block,
        blocking_nb: *mut notifier_block,
        tx_fwd_offload: bool_,
        extack: *mut netlink_ext_ack,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn switchdev_bridge_port_unoffload(
        brport_dev: *mut net_device,
        ctx: *const core::ffi::c_void,
        atomic_nb: *mut notifier_block,
        blocking_nb: *mut notifier_block,
    );
}
extern "C" {
    pub(crate) fn switchdev_deferred_process();
}
extern "C" {
    pub(crate) fn switchdev_port_attr_set(
        dev: *mut net_device,
        attr: *const switchdev_attr,
        extack: *mut netlink_ext_ack,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn switchdev_port_obj_add(
        dev: *mut net_device,
        obj: *const switchdev_obj,
        extack: *mut netlink_ext_ack,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn switchdev_port_obj_del(
        dev: *mut net_device,
        obj: *const switchdev_obj,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn register_switchdev_notifier(nb: *mut notifier_block) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn unregister_switchdev_notifier(nb: *mut notifier_block) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn call_switchdev_notifiers(
        val: core::ffi::c_ulong,
        dev: *mut net_device,
        info: *mut switchdev_notifier_info,
        extack: *mut netlink_ext_ack,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn register_switchdev_blocking_notifier(nb: *mut notifier_block) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn unregister_switchdev_blocking_notifier(nb: *mut notifier_block) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn call_switchdev_blocking_notifiers(
        val: core::ffi::c_ulong,
        dev: *mut net_device,
        info: *mut switchdev_notifier_info,
        extack: *mut netlink_ext_ack,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn switchdev_port_fwd_mark_set(
        dev: *mut net_device,
        group_dev: *mut net_device,
        joining: bool_,
    );
}
extern "C" {
    pub(crate) fn switchdev_handle_fdb_event_to_device(
        dev: *mut net_device,
        event: core::ffi::c_ulong,
        fdb_info: *const switchdev_notifier_fdb_info,
        check_cb: ::core::option::Option<unsafe extern "C" fn(dev: *const net_device) -> bool_>,
        foreign_dev_check_cb: ::core::option::Option<
            unsafe extern "C" fn(dev: *const net_device, foreign_dev: *const net_device) -> bool_,
        >,
        mod_cb: ::core::option::Option<
            unsafe extern "C" fn(
                dev: *mut net_device,
                orig_dev: *mut net_device,
                event: core::ffi::c_ulong,
                ctx: *const core::ffi::c_void,
                fdb_info: *const switchdev_notifier_fdb_info,
            ) -> core::ffi::c_int,
        >,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn switchdev_handle_port_obj_add(
        dev: *mut net_device,
        port_obj_info: *mut switchdev_notifier_port_obj_info,
        check_cb: ::core::option::Option<unsafe extern "C" fn(dev: *const net_device) -> bool_>,
        add_cb: ::core::option::Option<
            unsafe extern "C" fn(
                dev: *mut net_device,
                ctx: *const core::ffi::c_void,
                obj: *const switchdev_obj,
                extack: *mut netlink_ext_ack,
            ) -> core::ffi::c_int,
        >,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn switchdev_handle_port_obj_add_foreign(
        dev: *mut net_device,
        port_obj_info: *mut switchdev_notifier_port_obj_info,
        check_cb: ::core::option::Option<unsafe extern "C" fn(dev: *const net_device) -> bool_>,
        foreign_dev_check_cb: ::core::option::Option<
            unsafe extern "C" fn(dev: *const net_device, foreign_dev: *const net_device) -> bool_,
        >,
        add_cb: ::core::option::Option<
            unsafe extern "C" fn(
                dev: *mut net_device,
                ctx: *const core::ffi::c_void,
                obj: *const switchdev_obj,
                extack: *mut netlink_ext_ack,
            ) -> core::ffi::c_int,
        >,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn switchdev_handle_port_obj_del(
        dev: *mut net_device,
        port_obj_info: *mut switchdev_notifier_port_obj_info,
        check_cb: ::core::option::Option<unsafe extern "C" fn(dev: *const net_device) -> bool_>,
        del_cb: ::core::option::Option<
            unsafe extern "C" fn(
                dev: *mut net_device,
                ctx: *const core::ffi::c_void,
                obj: *const switchdev_obj,
            ) -> core::ffi::c_int,
        >,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn switchdev_handle_port_obj_del_foreign(
        dev: *mut net_device,
        port_obj_info: *mut switchdev_notifier_port_obj_info,
        check_cb: ::core::option::Option<unsafe extern "C" fn(dev: *const net_device) -> bool_>,
        foreign_dev_check_cb: ::core::option::Option<
            unsafe extern "C" fn(dev: *const net_device, foreign_dev: *const net_device) -> bool_,
        >,
        del_cb: ::core::option::Option<
            unsafe extern "C" fn(
                dev: *mut net_device,
                ctx: *const core::ffi::c_void,
                obj: *const switchdev_obj,
            ) -> core::ffi::c_int,
        >,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn switchdev_handle_port_attr_set(
        dev: *mut net_device,
        port_attr_info: *mut switchdev_notifier_port_attr_info,
        check_cb: ::core::option::Option<unsafe extern "C" fn(dev: *const net_device) -> bool_>,
        set_cb: ::core::option::Option<
            unsafe extern "C" fn(
                dev: *mut net_device,
                ctx: *const core::ffi::c_void,
                attr: *const switchdev_attr,
                extack: *mut netlink_ext_ack,
            ) -> core::ffi::c_int,
        >,
    ) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_port_dev_check(dev: *const net_device) -> bool_;
}
extern "C" {
    pub(crate) fn cpsw_switchdev_register_notifiers(cpsw: *mut cpsw_common) -> core::ffi::c_int;
}
extern "C" {
    pub(crate) fn cpsw_switchdev_unregister_notifiers(cpsw: *mut cpsw_common);
}
pub(crate) const CPSW_CPTS: core::ffi::c_int = 1;
#[repr(C)]
#[derive(Copy, Clone)]
pub(crate) struct cpts {
    _unused: [u8; 0],
}
pub(crate) const CPSW_CPDMA: core::ffi::c_int = 1;
