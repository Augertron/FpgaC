/*
 * pci.h - Generic Target Mode PCI Interface for FpgaC Applications
 * copyright 2006 by John Bass, DMS Design under FpgaC BSD License
 *
 *  NOTE: This is NOT complete yet .... still a work in progress
 */


/*
 * PCI Local Bus interfaces from PCI Specification Rev 2.2
 * Chapter 2 Signal Definitions
 *
 * Active low interfaces signals are designated with '_' name suffix
 */
struct PCI_Interface {
        fpgac_tristate	ad_b0:8;	// PCI Address/Data bus bits  7:0
        fpgac_tristate	ad_b1:8;	// PCI Address/Data bus bits 15:8
        fpgac_tristate	ad_b2:8;	// PCI Address/Data bus bits 23:16
        fpgac_tristate	ad_b3:8;	// PCI Address/Data bus bits 31:24
        fpgac_tristate	ad_b4:8;	// PCI Address/Data bus bits 39:32
        fpgac_tristate	ad_b5:8;	// PCI Address/Data bus bits 47:40
        fpgac_tristate	ad_b6:8;	// PCI Address/Data bus bits 55:48
        fpgac_tristate	ad_b7:8;	// PCI Address/Data bus bits 63:56
        fpgac_tristate	cbe_:8;		// PCI Bus Command and Byte Enables
        fpgac_tristate	par:1;		// PCI Parity for ad and cbe is EVEN low 32 bit bus
        fpgac_tristate	frame_:1;	// PCI Cycle Frame, active low for an access
        fpgac_tristate	trdy_:1;	// PCI Initiator Ready, active low
        fpgac_tristate	irdy_:1;	// PCI Target Ready, active low
        fpgac_tristate	stop_:1;	// PCI Stop, active low when target requests stop
        fpgac_tristate	devsel_:1;	// PCI Device Select, active low
        fpgac_tristate	idsel:1;	// PCI Initalization Device Select
        fpgac_tristate	perr_:1;	// PCI Parity Error, active low
        fpgac_tristate	serr_:1;	// PCI System Error, active low
        fpgac_tristate	req_:1;		// PCI Request for Bus Master, active low
        fpgac_input	gnt_:1;		// PCI Grant for Bus Master, active low
        fpgac_input	rst_:1;		// System Reset, active low
	fpgac_output	inta_:1;	// Interupt Request, active low
	fpgac_output	intb_:1;	// Interupt Request, active low
	fpgac_output	intc_:1;	// Interupt Request, active low
	fpgac_output	intd_:1;	// Interupt Request, active low
        fpgac_tristate	par64:1;	// PCI Parity for Upper 32 bit bus
        fpgac_tristate	req64_:1;	// PCI Request for 64 bit transfer
        fpgac_tristate	ack64_:1;	// PCI Acknowledge for 64 bit transfer
};

/*
 * Chapter 3 Bus Operation
 */
#define PCI_CMD_Int_Ack		0b0000		// Interrupt Acknowledge
#define PCI_CMD_Special_Cycle	0b0001		// Special Cycle
#define PCI_CMD_IO_Read		0b0010		// I/O Read
#define PCI_CMD_IO_Write	0b0011		// I/O Write
#define PCI_CMD_Rsvd4		0b0100		// Reserved
#define PCI_CMD_Rsvd5		0b0101		// Reserved
#define PCI_CMD_Memory_Read	0b0110		// Memory Read
#define PCI_CMD_Memory_Write	0b0111		// Memory Write
#define PCI_CMD_Rsvd8		0b1000		// Reserved
#define PCI_CMD_Rsvd9		0b1001		// Reserved
#define PCI_CMD_Config_Read	0b1010		// Configuration Space Read
#define PCI_CMD_Config_Write	0b1011		// Configuration Space Write
#define PCI_CMD_Mem_Read_Mult	0b1100		// Memory Read Multiple
#define PCI_CMD_Dual_Addr	0b1101		// Dual Address Cycle
#define PCI_CMD_Mem_Read_Line	0b1110		// Memory Read Line
#define PCI_CMD_Mem_Write_Inval	0b1111		// Memory Write and Invalidate

#define PCI_CMD_Memory		0b0100		// Mask for any memory access
/*
 * Chapter 6 Configuration Space
 */
struct PCI_Config {
    unsigned short Device_ID;		// PCI Configuration word 0x00
    unsigned short Vendor_ID;		// PCI Configuration word 0x00
    unsigned short Status;		// PCI Configuration word 0x04
    unsigned short Command;		// PCI Configuration word 0x04
    unsigned int   ClassCode:24;	// PCI Configuration word 0x08
    unsigned char  Revision_ID;		// PCI Configuration word 0x08
    unsigned char  BIST;		// PCI Configuration word 0x0c Built-In Self Test
    unsigned char  HeaderType;		// PCI Configuration word 0x0c single function device is 0x00
    unsigned char  LatencyTimer;	// PCI Configuration word 0x0c
    unsigned char  CacheLineSize;	// PCI Configuration word 0x0c
    unsigned long  Bar0;		// PCI Configuration word 0x10 Base Address Registers
    unsigned long  Bar1;		// PCI Configuration word 0x14 Base Address Registers
    unsigned long  Bar2;		// PCI Configuration word 0x18 Base Address Registers
    unsigned long  Bar3;		// PCI Configuration word 0x1c Base Address Registers
    unsigned long  Bar4;		// PCI Configuration word 0x20 Base Address Registers
    unsigned long  Bar5;		// PCI Configuration word 0x24 Base Address Registers
    unsigned long  CardbusCIS;		// PCI Configuration word 0x28
    unsigned short SubSystem_ID;	// PCI Configuration word 0x2C
    unsigned short SubVendor_ID;	// PCI Configuration word 0x2C
    unsigned long  ROM_Bar;		// PCI Configuration word 0x30 ROM Base Address Register
    unsigned char  Capabilities;	// PCI Configuration word 0x34
    unsigned char  Max_Lat;		// PCI Configuration word 0x3c
    unsigned char  Min_Gnt;		// PCI Configuration word 0x3c
    unsigned char  Interrupt_Pin;	// PCI Configuration word 0x3c
    unsigned char  Interrupt_Line;	// PCI Configuration word 0x3c
};

/*
 * Configuration Space Addresses
 */
#define PCI_CONFIG_Dev_Vendor	0x00	// Device Id, Vendor ID
#define PCI_CONFIG_status_cmd	0x04	// Status, Command
#define PCI_CONFIG_class_revid	0x08	// Class Code, Revision ID
#define PCI_CONFIG_bhlc		0x0c	// BIST, Header Type, Latency Timer, Cache Line Size
#define PCI_CONFIG_bar0		0x10	// Base Address Registers, word 0
#define PCI_CONFIG_bar1		0x14	// Base Address Registers, word 1
#define PCI_CONFIG_bar2		0x18	// Base Address Registers, word 2
#define PCI_CONFIG_bar3		0x1c	// Base Address Registers, word 3
#define PCI_CONFIG_bar4		0x20	// Base Address Registers, word 4
#define PCI_CONFIG_bar5		0x24	// Base Address Registers, word 5
#define PCI_CONFIG_Cardbus_CIS	0x28	// Cardbus CIS Pointer
#define PCI_CONFIG_ssid_svid	0x2c	// Subsystem ID, Subsystem Vendor ID
#define PCI_CONFIG_ROM_Bar	0x30	// Expansion ROM Base Address
#define PCI_CONFIG_Capabilities	0x34	// Capabilities Pointer
#define PCI_CONFIG_lgpl		0x3c	// Max_lat, Min_Gnt, Interrupt Pin, Interrupt Line

/*
 * Appendix B State Machines
 *
 * Target Mode States
 */
#define PCI_SM_Target_Idle	0b0000001	// Valid next states are idle, B_Busy
#define PCI_SM_Target_B_Busy	0b0000010	// Valid next states are idle, B_Busy, Backoff, S_Data
#define PCI_SM_Target_S_Data	0b0000100	// Valid next states are S_Data, Backoff, Turn_Ar
#define PCI_SM_Target_Turn_Ar	0b0001000	// Valid next states are idle, B_Busy
#define PCI_SM_Target_BackOff	0b0010000	// Valid next states are Backoff, Turn_Ar
#define PCI_SM_Target_Comp_Addr	0b0100000	// Valid next states are
#define PCI_SM_Target_Config	0b1000000	// Valid next states are

/*
 * Master Mode States
 */
#define PCI_SM_Master_Idle	0b000001	// Valid next states are
#define PCI_SM_Master_Addr	0b000010	// Valid next states are
#define PCI_SM_Master_M_Data	0b000100	// Valid next states are
#define PCI_SM_Master_Turn_Ar	0b001000	// Valid next states are
#define PCI_SM_Master_S_Tar	0b010000	// Valid next states are
#define PCI_SM_Master_DR_Bus	0b100000	// Valid next states are

/*
 * Latched copy of key PCI Bus values, inverted versions, etc
 */
struct PCI_State {
	unsigned	addr_b0:8;	// PCI address latched with frame, bits  7:0
	unsigned	addr_b1:8;	// PCI address latched with frame, bits 15:8
	unsigned	addr_b2:8;	// PCI address latched with frame, bits 23:16
	unsigned	addr_b3:8;	// PCI address latched with frame, bits 31:24
	unsigned	addr_b4:8;	// PCI address latched with frame, bits  7:0
	unsigned	addr_b5:8;	// PCI address latched with frame, bits 15:8
	unsigned	addr_b6:8;	// PCI address latched with frame, bits 23:16
	unsigned	addr_b7:8;	// PCI address latched with frame, bits 31:24
        unsigned	cmd:8;		// PCI Bus Command latched with frame
        unsigned	frame:1;	// PCI Cycle Frame, active high for an access
        unsigned	trdy:1;		// PCI Initiator Ready, active high
        unsigned	irdy:1;		// PCI Target Ready, active high
        unsigned	stop:1;		// PCI Stop, active high when target requests stop
        unsigned	devsel:1;	// PCI Device Select, active high
        unsigned	idsel:1;	// PCI Initalization Device Select
        unsigned	gnt:1;		// PCI Grant for Bus Master, active high
        unsigned	rst:1;		// System Reset, active high
        unsigned	req64:1;	// PCI Request for 64 bit transfer
        unsigned	ack64:1;	// PCI Acknowledge for 64 bit transfer
	unsigned	lastframe:1;	// PCI Frame's previous state
	unsigned	rw:1;		// PCI Read/Write state latched from command
	unsigned	bar0:1;		// PCI Read/Write to bar0
	unsigned	bar1:1;		// PCI Read/Write to bar1
};

