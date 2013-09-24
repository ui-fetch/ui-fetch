
//configuration settings


#ifndef OVERRIDE_CONFIG_BITS
/* ------------------------------------------------
------------ */
/* PIC32 Configuration Settings */
/* ------------------------------------------------
------------ */
/* Oscillator Settings
*/
#pragma config FNOSC = PRIPLL // Oscillator selection
#pragma config POSCMOD = XT // Primary oscillator mode
#pragma config FPLLIDIV = DIV_2 // PLL input divider
#pragma config FPLLMUL = MUL_20 // PLL multiplier
#pragma config FPLLODIV = DIV_1 // PLL output divider
#pragma config FPBDIV = DIV_8 // Peripheral bus clock divider
#pragma config FSOSCEN = OFF // Secondary oscillator enable
/* Clock control settings
*/
#pragma config IESO = OFF // Internal/external clock switchover
#pragma config FCKSM = CSDCMD // Clock switching (CSx)/Clock monitor (CMx)
#pragma config OSCIOFNC = OFF // Clock output on OSCO pin enable
/* Other Peripheral Device settings
*/
#pragma config FWDTEN = OFF // Watchdog timer enable
#pragma config WDTPS = PS1024 // Watchdog timer post-scaler
/* Code Protection settings
*/
#pragma config CP = OFF // Code protection
#pragma config BWP = OFF // Boot flash write protect
#pragma config PWP = OFF // Program flash write protect


#endif
