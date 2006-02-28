/*
 * pci.c - Generic Target Mode PCI Interface for FpgaC Applications
 * copyright 2006 by John Bass, DMS Design under FpgaC BSD License
 *
 *  NOTE: This is NOT complete yet .... still a work in progress
 *
 * TODO:
 *       * Finish state machine to include 64bit transactions.
 *
 *       * Add in burst mode support
 *
 *       * Do parity check and error assertion
 *
 *       * Finish application interface to my Dini demos
 *
 *       * Setup a multifunction PCI device with spoof of parallel port
 *         and xilinx config prom so the Dini eeprom can be programed
 *         directly from ISE, and not have to use the Dini utility.
 *
 *       * think more about master mode
 */

#include "pci.h"

/*
 * Our values for various configuration space fields
 * For my Dini DN2K board and driver
 */
#define PCI_VENDOR_ID       0xABCD
#define PCI_DEVICE_ID       0x1234
#define PCI_REVISION_ID     0x47
#define PCI_CLASS_CODE      0xFF0000
#define PCI_BIST            0x48
#define PCI_SUBSVID         0x5678
#define PCI_SUBSID          0x90AB

struct PCI_Interface pci_bus;
struct PCI_State pci_state;
struct PCI_Config pci_config;

/*
 * PCI State Machine Process
 */
fpgac_process pci() {
    int target_sm, next_target_sm;

    next_target_sm = PCI_SM_Target_Idle;	// Default next state is idle

    if(pci_state.rst == 0) {
#pragma fpgac_bus_idle(pci_bus.ad_b0)
#pragma fpgac_bus_idle(pci_bus.ad_b1)
#pragma fpgac_bus_idle(pci_bus.ad_b2)
#pragma fpgac_bus_idle(pci_bus.ad_b3)
#pragma fpgac_bus_idle(pci_bus.ad_b4)
#pragma fpgac_bus_idle(pci_bus.ad_b5)
#pragma fpgac_bus_idle(pci_bus.ad_b6)
#pragma fpgac_bus_idle(pci_bus.ad_b7)
#pragma fpgac_bus_idle(pci_bus.cbe_)
#pragma fpgac_bus_idle(pci_bus.par)
#pragma fpgac_bus_idle(pci_bus.frame_)
#pragma fpgac_bus_idle(pci_bus.trdy_)
#pragma fpgac_bus_idle(pci_bus.irdy_)
#pragma fpgac_bus_idle(pci_bus.stop_)
#pragma fpgac_bus_idle(pci_bus.devsel_)
#pragma fpgac_bus_idle(pci_bus.idsel)
#pragma fpgac_bus_idle(pci_bus.perr_)
#pragma fpgac_bus_idle(pci_bus.serr_)
#pragma fpgac_bus_idle(pci_bus.req_)
#pragma fpgac_bus_idle(pci_bus.par64_)
#pragma fpgac_bus_idle(pci_bus.req64_)
#pragma fpgac_bus_idle(pci_bus.ack64_)
        target_sm = 0;
        pci_config.Device_ID = PCI_VENDOR_ID;
        pci_config.Vendor_ID = PCI_DEVICE_ID;
        pci_config.Status = 0;
        pci_config.Command = 0;
        pci_config.ClassCode = PCI_CLASS_CODE;
        pci_config.Revision_ID = PCI_REVISION_ID;
        pci_config.BIST = PCI_BIST;
        pci_config.HeaderType = 0;
        pci_config.LatencyTimer = 0;
        pci_config.CacheLineSize = 0;
        pci_config.Bar0 = 0;
        pci_config.Bar1 = 0;
        pci_config.Bar2 = 0;
        pci_config.Bar3 = 0;
        pci_config.Bar4 = 0;
        pci_config.Bar5 = 0;

    } else {
        pci_state.frame  = ~pci_bus.frame_;	// Make active high copies (get optimized away)
        pci_state.trdy   = ~pci_bus.trdy_;
        pci_state.irdy   = ~pci_bus.irdy_;
        pci_state.stop   = ~pci_bus.stop_;
        pci_state.devsel = ~pci_bus.devsel_;
        pci_state.idsel  = ~pci_bus.idsel_;
        pci_state.gnt    = ~pci_bus.gnt_;
        pci_state.rst    = ~pci_bus.rst_;
        pci_state.req64  = ~pci_bus.req64_;
        pci_state.ack64  = ~pci_bus.ack64_;
    }

    if(target_sm & PCI_SM_Target_Idle) {
#pragma fpgac_bus_idle(pci_bus.ad_b0)
#pragma fpgac_bus_idle(pci_bus.ad_b1)
#pragma fpgac_bus_idle(pci_bus.ad_b2)
#pragma fpgac_bus_idle(pci_bus.ad_b3)
#pragma fpgac_bus_idle(pci_bus.ad_b4)
#pragma fpgac_bus_idle(pci_bus.ad_b5)
#pragma fpgac_bus_idle(pci_bus.ad_b6)
#pragma fpgac_bus_idle(pci_bus.ad_b7)
#pragma fpgac_bus_idle(pci_bus.trdy_)
#pragma fpgac_bus_idle(pci_bus.stop_)
#pragma fpgac_bus_idle(pci_bus.devsel_)
#pragma fpgac_bus_idle(pci_bus.par_)
#pragma fpgac_bus_idle(pci_bus.par64_)
#pragma fpgac_bus_idle(pci_bus.perr_)
        if(pci_state.frame & !pci_state.lastframe) {
            pci_state.cmd   = ~pci_bus.cbe_;	// Latch cmd and address when frame asserted
            pci_state.ad_b0 = pci_bus.ad_b0;
            pci_state.ad_b1 = pci_bus.ad_b1;
            pci_state.ad_b2 = pci_bus.ad_b2;
            pci_state.ad_b3 = pci_bus.ad_b3;
            pci_state.rw = pci_state.cmd & 1;

            if(pci_state.cmd & PCI_CMD_Memory) { // Memory RW
                pci_state.bar0 = pci_bus.ad_b3 == ((pci_config.Bar0>>23) & 0xff);
                pci_state.bar1 = pci_bus.ad_b3 == ((pci_config.Bar1>>23) & 0xff);
                if(pci_state.bar0 | pci_state.bar1)
                    next_target_sm = PCI_SM_Target_B_Busy;
            } else {
                pci_state.bar0 = pci_state.bar1 = 0;
                if(idsel && ((pci_state.cmd & 0xfe) == PCI_CMD_Config_Read))  // Config RW
                    next_target_sm = PCI_SM_Target_Config;
            }
        } else ;// next_target_sm = PCI_SM_Target_Idle;
    }

    if(target_sm & PCI_SM_Target_B_Busy) {
        next_target_sm = PCI_SM_Target_Comp_Addr;
    }

    if(target_sm & PCI_SM_Target_Comp_Addr) {
        if(pci_state.bar0 | pci_state.bar1)
             next_target_sm = PCI_SM_Target_S_Data;
        else ;// next_target_sm = PCI_SM_Target_Idle;
    }

    if(target_sm & PCI_SM_Target_S_Data) {

        /*
         * Place read/write interfaces to backend here
         */

        if(!(trdy_reg_int & irdy)) next_target_sm = PCI_SM_Target_S_Data;
        else if(irdy & trdy_reg_int & frame) next_target_sm = PCI_SM_Target_BackOff;
        else ;// next_target_sm = PCI_SM_Target_Idle;
    }

    if(target_sm & PCI_SM_Target_BackOff) {
        if(pci_state.frame) next_target_sm = PCI_SM_Target_BackOff;
        else next_target_sm = PCI_SM_Target_Turn_Ar;
    }

    if(target_sm & PCI_SM_Target_Config) {
        unsigned char config_addr;
        unsigned long data;

        config_addr = pci_state.ad_b0 & 0xff;
        if(pci_state.rw) {			// Configuration Space Write
            if(config_addr == PCI_CONFIG_status_cmd)
                pci_config.Command = ((pci_bus.ad_b1<<8) | pci_bus.ad_b0) & 0x2ff;
            if(config_addr == PCI_CONFIG_bar0)
                pci_config.Bar0 = (pci_bus.ad_b3<<24) & 0xff000000;
            if(config_addr == PCI_CONFIG_bar1)
                pci_config.Bar1 = (pci_bus.ad_b3<<24) & 0xff000000;
        } else {				// Configuration Space Read
            data = 0;
            if(config_addr == PCI_CONFIG_Dev_Vendor)
                data = ((pci_config.Device_ID & 0xffff) << 16) | (pci_config.Vendor_ID & 0xffff);
            if(config_addr == PCI_CONFIG_status_cmd)
                data = ((pci_config.Status & 0xffff) << 16) | (pci_config.Command & 0xffff);
            if(config_addr == PCI_CONFIG_class_revid)
                data = ((pci_config.ClassCode & 0xffffff) << 8) | (pci_config.Revision_ID & 0xff);
            if(config_addr == PCI_CONFIG_bar0)
                data = pci_config.Bar0 & 0xffffffe0;
            if(config_addr == PCI_CONFIG_bar1)
                data = pci_config.Bar1 & 0xffffffe0;
            if(config_addr == PCI_CONFIG_ssid_svid)
                data = ((pci_config.SubSystem_ID & 0xffff) << 16) | (pci_config.SubVendor_ID & 0xffff);

            // Now driving data bus, must be tristated at end of cycle
            if(pci_bus.cbe & 0x01) pci_bus.ad_0 =  data      & 0xff;
            if(pci_bus.cbe & 0x02) pci_bus.ad_1 = (data>>8)  & 0xff;
            if(pci_bus.cbe & 0x04) pci_bus.ad_2 = (data>>16) & 0xff;
            if(pci_bus.cbe & 0x08) pci_bus.ad_3 = (data>>16) & 0xff;
        }
        pci_bus.trdy_ = pci_bus.devsel_ = 0; // Now driving trdy, must be tristated at end of cycle

        if(!pci_state.irdy)
            next_target_sm = PCI_SM_Target_Config;   // Loop in Config waiting for host
        else if(pci_state.irdy & pci_state.frame)
            next_target_sm = PCI_SM_Target_BackOff;  // Done, finish cycle
        else ;// next_target_sm = PCI_SM_Target_Idle;
    }

    if(target_sm & PCI_SM_Target_Turn_Ar) {
#pragma fpgac_bus_idle(pci_bus.ad_b0)
#pragma fpgac_bus_idle(pci_bus.ad_b1)
#pragma fpgac_bus_idle(pci_bus.ad_b2)
#pragma fpgac_bus_idle(pci_bus.ad_b3)
#pragma fpgac_bus_idle(pci_bus.ad_b4)
#pragma fpgac_bus_idle(pci_bus.ad_b5)
#pragma fpgac_bus_idle(pci_bus.ad_b6)
#pragma fpgac_bus_idle(pci_bus.ad_b7)
        ;// next_target_sm = PCI_SM_Target_Idle;
    }

    target_sm = next_target_sm;
    pci_state.lastframe = pci_state.frame;
}
