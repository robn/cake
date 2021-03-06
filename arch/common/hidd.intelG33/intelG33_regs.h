
#include <inttypes.h>

/*
Address Range Description
MMADR+
00000h 00FFFh VGA and Extended VGA Control Registers. These registers are located in both
              I/O space and memory space. The VGA and Extended VGA registers contain the
              following register sets: General Control/Status, Sequencer (SRxx), Graphics
              Controller (GRxx), Attribute Controller (Arxx), VGA Color Palette, and CRT Controller
              (CRxx) registers. Detailed bit descriptions are provided in the VGA and Extended VGA
              Register Chapter. The registers within a set are accessed using an indirect addressing
              mechanism as described at the beginning of each section. Note that some of the
              register description sections have additional operational information at the beginning
              of the section
01000h 01FFFh Reserved
02000h 02FFFh Instruction, Memory, and Interrupt Control Registers:
              Instruction Control Registers Ring Buffer registers and page table control
              registers are located in this address range. Various instruction status, error, and
              operating registers are located in this group of registers.
              Graphics Memory Fence Registers. The Graphics Memory Fence registers are
              used for memory tiling capabilities.
              Interrupt Control/Status Registers. This register set provides interrupt
              control/status for various GC functions.
              Display Interface Control Register. This register controls the FIFO watermark and
              provides burst length control.
              Logical Context Registers
              Software Visible Counters
03000h 031FFh FENCE & Per Process GTT Control registers
03200h 03FFFh Frame Buffer Compression Registers
04000h 043FFh Reserved.
04400h 04FFFh Reserved.
05000h 05FFFh I/O Control Registers
06000h 06FFFh Clock Control Registers. This memory address space is the location of the GC clock
              control and power management registers
07000h 073FFh 3D Internal Debug Registers
07400h 088FFh GPE Debug Registers (3D/Media Fixed Functions)
08900h 08FFFh Reserved for Subsystem Debug Registers
09000h 09FFFh Reserved
0A000h 0AFFFh Display Palette Registers
0B000h 0FFFFh Reserved
10000h 13FFFh MMIO MCHBAR. Alias through which the graphics driver can access registers in the
              MCHBAR accessed through device 0.
14000h 2FFFFh Reserved
30000h 3FFFFh Overlay Registers. These registers provide control of the overlay engine. The
              overlay registers are double-buffered with one register buffer located in graphics
              memory and the other on the device. On-chip registers are not directly writeable. To
              update the on-chip registers software writes to the register buffer area in graphics
              memory and instructs the device to update the on-chip registers.
40000h 5FFFFh Reserved
60000h 6FFFFh Display Engine Pipeline Registers
70000h 72FFFh Display and Cursor Registers
73000h 73FFFh Performance Counters
74000h 7FFFFh Reserved
*/

#define INTEL_VENDOR_ID     0x8086

#define IS_G33(id) (id == 0x29b2 || \
                    id == 0x29c2 || \
                    id == 0x29d2)

#define readl(addr) ( *(volatile uint32_t *) (addr) )
#define readw(addr) ( *(volatile uint16_t *) (addr) )
#define readb(addr) ( *(volatile uint8_t *)  (addr) )

#define writeb(b,addr) ( (*(volatile uint8_t *)  (addr)) = (b) )
#define writew(b,addr) ( (*(volatile uint16_t *) (addr)) = (b) )
#define writel(b,addr) ( (*(volatile uint32_t *) (addr)) = (b) )

#define G33_RD_REGL(a, reg) ( readl(sd->Chipset.a + reg) )
#define G33_RD_REGW(a, reg) ( readw(sd->Chipset.a + reg) )
#define G33_RD_REGB(a, reg) ( readb(sd->Chipset.a + reg) )

#define G33_WR_REGL(a, reg, value) ( writel((value), (sd->Chipset.a + reg)) )
#define G33_WR_REGW(a, reg, value) ( writew((value), (sd->Chipset.a + reg)) )
#define G33_WR_REGB(a, reg, value) ( writeb((value), (sd->Chipset.a + reg)) )

/* Should get away with this... or not... */
#define G33_SETBMASK_REGL(a, reg, value) ( writel((readl(sd->Chipset.a + reg )|value), (sd->Chipset.a + reg)) )
#define G33_SETBMASK_REGW(a, reg, value) ( writew((readw(sd->Chipset.a + reg )|value), (sd->Chipset.a + reg)) )
#define G33_SETBMASK_REGB(a, reg, value) ( writeb((readb(sd->Chipset.a + reg )|value), (sd->Chipset.a + reg)) )

#define G33_CLRBMASK_REGL(a, reg, value) ( writel((readl(sd->Chipset.a + reg )&(~value)), (sd->Chipset.a + reg)) )
#define G33_CLRBMASK_REGW(a, reg, value) ( writew((readw(sd->Chipset.a + reg )&(~value)), (sd->Chipset.a + reg)) )
#define G33_CLRBMASK_REGB(a, reg, value) ( writeb((readb(sd->Chipset.a + reg )&(~value)), (sd->Chipset.a + reg)) )

#define G33_WRM_REGL(a, reg, value, mask) ( writel( (((readl(sd->Chipset.a + reg )&(~mask)))&value), (sd->Chipset.a + reg)) )
#define G33_WRM_REGW(a, reg, value, mask) ( writew( (((readw(sd->Chipset.a + reg )&(~mask)))&value), (sd->Chipset.a + reg)) )
#define G33_WRM_REGB(a, reg, value, mask) ( writeb( (((readb(sd->Chipset.a + reg )&(~mask)))&value), (sd->Chipset.a + reg)) )

#define G33_RD_REG_ARRAY(a, reg, offset) ( readl((sd->Chipset.a + reg) + ((offset) << 2)) )
#define G33_WR_REG_ARRAY(a, reg, offset, value) ( writel((value), ((sd->Chipset.a + reg) + ((offset) << 2))) )

#define MEMORY_CONTROL_ENABLED      0x0004
#define MEMORY_MASK                 0x0001

#define STOLEN_MEMORY_MASK          0x00f0
#define STOLEN_MEMORY_1M            0x0010
#define STOLEN_MEMORY_4M            0x0020
#define STOLEN_MEMORY_8M            0x0030
#define STOLEN_MEMORY_16M           0x0040
#define STOLEN_MEMORY_32M           0x0050
#define STOLEN_MEMORY_48M           0x0060
#define STOLEN_MEMORY_64M           0x0070
#define STOLEN_MEMORY_128M          0x0080
#define STOLEN_MEMORY_256M          0x0090
#define STOLEN_MEMORY_96M           0x00a0
#define STOLEN_MEMORY_160M          0x00b0
#define STOLEN_MEMORY_224M          0x00c0
#define STOLEN_MEMORY_352M          0x00d0

#define GTT_MASK                    0x0300
#define GTT_1M                      0x0100
#define GTT_2M                      0x0200

/* I/O Control Registers (05000h−05FFFh) */
#define GMBUSRate1MHz               3<<8
#define GMBUSRate400KHz             2<<8
#define GMBUSRate100KHz             0
#define GMBUSRate50KHz              1<<8
#define GMBUSHTimeExt               1<<7

#define GMBUS0      0x5100
#define GMBUS1      0x5104
#define GMBUS2      0x5108
#define GMBUS3      0x510c
#define GMBUS4      0x5110
#define GMBUS5      0x5120

#define ADPA        0x61100     /* Analog display port A*/
#define DDPBC       0x61140     /* Digital display port B control */
#define DDPCC       0x61160     /* Digital display port C control */
#define LVDS        0x61180     /* Digital display port control*/

#define DPMSMASK    0x0c00

#define VGADisable  1<<31
#define VGACNTRL    0x71400


