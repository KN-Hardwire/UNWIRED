#ifndef UNWIRED_PLATFORM_STM32_PINOUT_H
#define UNWIRED_PLATFORM_STM32_PINOUT_H

/* STM32F401CCU6 pin assignments.
 *
 * Source: hardware/microcontroller/stm32.kicad_sch (U3), with the routing of
 * the J2 matrix connector taken from hardware/connection_matrix. Net names
 * from the schematic are given in the comment of each signal. Keep this file
 * in sync with the schematic when the board changes; verify against
 * `kicad-cli sch export netlist`, not the PDF export.
 *
 * Definitions are independent of the STM32 HAL/CMSIS so this header can be
 * included anywhere in the stm32 platform; drivers map STM32_PORT_* to the
 * GPIOx instances. Alternate function numbers follow the STM32F401xB/C
 * datasheet alternate function mapping table.
 *
 * Pins are C23 constexpr objects: typed and usable in constant expressions
 * (static_assert, case labels), but not in #if. Each translation unit gets
 * its own read-only copy, which the compiler drops when unused.
 */

#include <stdint.h>

typedef enum : uint8_t {
    STM32_PORT_A = 0,
    STM32_PORT_B,
    STM32_PORT_C,
    STM32_PORT_H,
} Stm32Port;

typedef enum : uint8_t {
    STM32_AF_SYS = 0,  /* SWD / trace */
    STM32_AF_SPI = 5,  /* SPI1, SPI2 */
    STM32_AF_USB = 10, /* OTG_FS */
} Stm32Af;

/* Pin used as a plain GPIO. */
typedef struct {
    Stm32Port port;
    uint8_t pin; /* 0..15 */
} Stm32Pin;

/* Pin routed to a peripheral through its alternate function. */
typedef struct {
    Stm32Port port;
    uint8_t pin; /* 0..15 */
    Stm32Af af;
} Stm32AfPin;

/* --- Matrix board SPI bus (J2), SPI1 -------------------------------------
 * Shared by the MCP23S17 GPIO expander (CH446Q strobes) and the DAC8568.
 * Chip selects are driven as plain GPIO outputs, active low.
 */
constexpr Stm32AfPin STM32_MATRIX_SPI_SCK  = { STM32_PORT_A, 5, STM32_AF_SPI }; /* SPI.SCK,  J2 pin 11, U3 pin 15 */
constexpr Stm32AfPin STM32_MATRIX_SPI_MOSI = { STM32_PORT_A, 7, STM32_AF_SPI }; /* SPI.MOSI, J2 pin 13, U3 pin 17 */
/* Labelled in the schematic but not routed to J2: no device reads back. */
constexpr Stm32AfPin STM32_MATRIX_SPI_MISO = { STM32_PORT_A, 6, STM32_AF_SPI }; /* SPI.MISO, U3 pin 16 */

constexpr Stm32Pin STM32_EXPANDER_CS = { STM32_PORT_A, 4 };  /* MCP23S17 ~CS,   SPI.CS1, J2 pin 9,  U3 pin 14 */
constexpr Stm32Pin STM32_DAC_SYNC    = { STM32_PORT_B, 12 }; /* DAC8568 ~SYNC, SPI.CS2, J2 pin 10, U3 pin 25 */

/* --- CH446Q serial control (J2), SPI2 transmit only ----------------------
 * Buffered on the matrix board and shared by all CH446Q chips; each chip
 * latches with its own STROBE line driven by the MCP23S17.
 */
constexpr Stm32AfPin STM32_CH446_CLK = { STM32_PORT_B, 13, STM32_AF_SPI }; /* CH_CTR.CLK, J2 pin 15, U3 pin 26 */
constexpr Stm32AfPin STM32_CH446_DAT = { STM32_PORT_B, 15, STM32_AF_SPI }; /* CH_CTR.DAT, J2 pin 14, U3 pin 28 */

/* --- USB FS device (J1 via U2 USBLC6) ------------------------------------ */
constexpr Stm32AfPin STM32_USB_DM = { STM32_PORT_A, 11, STM32_AF_USB }; /* USB_D-, U3 pin 32 */
constexpr Stm32AfPin STM32_USB_DP = { STM32_PORT_A, 12, STM32_AF_USB }; /* USB_D+, U3 pin 33 */

/* --- Debug (J3, SWD) ----------------------------------------------------- */
constexpr Stm32AfPin STM32_SWDIO = { STM32_PORT_A, 13, STM32_AF_SYS }; /* SWDIO, U3 pin 34 */
constexpr Stm32AfPin STM32_SWCLK = { STM32_PORT_A, 14, STM32_AF_SYS }; /* SWCLK, U3 pin 37 */
constexpr Stm32AfPin STM32_SWO   = { STM32_PORT_B, 3,  STM32_AF_SYS }; /* SWO,   U3 pin 39 */

/* --- Clocks -------------------------------------------------------------- */
constexpr uint32_t STM32_HSE_HZ = 25'000'000; /* Y1 on PH0/PH1 */
constexpr uint32_t STM32_LSE_HZ = 32'768;     /* Y2 on PC14/PC15 */

/* --- Reserved / unassigned ------------------------------------------------
 * PB2 is BOOT1, pulled to GND through R5; do not drive it.
 * PA15 (U3 pin 38) carries a dangling "CS" label with no connection.
 * Unconnected: PA0-PA3, PA8-PA10, PB0, PB1, PB4-PB10, PB14, PC13.
 */

/* --- Compile-time checks ------------------------------------------------- */
static_assert(STM32_MATRIX_SPI_SCK.pin  < 16 && STM32_MATRIX_SPI_MOSI.pin < 16 &&
              STM32_MATRIX_SPI_MISO.pin < 16 && STM32_EXPANDER_CS.pin     < 16 &&
              STM32_DAC_SYNC.pin        < 16 && STM32_CH446_CLK.pin       < 16 &&
              STM32_CH446_DAT.pin       < 16 && STM32_USB_DM.pin          < 16 &&
              STM32_USB_DP.pin          < 16 && STM32_SWDIO.pin           < 16 &&
              STM32_SWCLK.pin           < 16 && STM32_SWO.pin             < 16,
              "GPIO pin index out of range");

#endif
