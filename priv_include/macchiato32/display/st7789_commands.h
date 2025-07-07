#ifndef M32_DISPLAY_ST7789_COMMANDS_H
#define M32_DISPLAY_ST7789_COMMANDS_H

/* ST7789 instruction set. For an explanation on what these do and how they can be used, read the Sitronix ST7789 Datasheet */

#define ST7789_NOP      0x00
#define ST7789_SWRESET  0x01
// Display info 
#define ST7789_RDDID    0x04
#define ST7789_RDDST   0x09
#define ST7789_RDDPM   0x0A
#define ST7789_RDDMADCTL   0x0B
#define ST7789_RDDCOLMOD   0x0C
#define ST7789_COLMOD   0x3A    // Pixel Format interface
#define ST7789_RDDIM   0x0D
#define ST7789_RDDSM   0x0E
#define ST7789_RDDSDR   0x0F
// RDID
#define ST7789_RDID1   0xDA // ID1 (Manufacturer ID)
#define ST7789_RDID2   0xDB // ID2 (Driver version)
#define ST7789_RDID3   0xDC // ID3 (Driver ID)
// Sleep In/Out
#define ST7789_SLPIN   0x10
#define ST7789_SLPOUT   0x11
// Display modes
#define ST7789_PTLON   0x12 // Partial mode
#define ST7789_NORON   0x13 // Normal mode
#define ST7789_INVOFF    0x20
#define ST7789_INVON     0x21
#define ST7789_GAMSET    0x26
// Display On/Off
#define ST7789_DISPOFF     0x28
#define ST7789_DISPON    0x29
#define ST7789_CASET   0x2A
#define ST7789_RASET   0x2B
#define ST7789_RAMWR   0x2C
#define ST7789_RAMRD   0x2E
// Display modes
#define ST7789_PTLAR   0x30 // Partial Area
// Vertical Scrolling
#define ST7789_VSCRDEF   0x33
#define ST7789_VSCSAD   0x37    // Vertical SCreen ADdress of RAM
//Tearing
#define ST7789_TEOFF   0x34
#define ST7789_TEON   0x35
//Memory
#define ST7789_MADCTL   0x36    // Memory Data Access Control
// Idle Mode
#define ST7789_IDMOFF   0x38
#define ST7789_IDMON   0x39
// Write/Read Memory continue
#define ST7789_WRMEMC   0x3C
#define ST7789_RDMEMC   0x3E
// Scanlines
#define ST7789_STE   0x44   // Set Tear Scanline
#define ST7789_GSCAN   0x45   // Get Scanline
// Display Brightness
#define ST7789_WRDISBV   0x51
#define ST7789_RDDISBV   0x52
// CTRL Display
#define ST7789_WRCTRLD   0x53
#define ST7789_RDCTRLD   0x54
// Content Adaptive brightness control & Color Enhancement (CACE)
#define ST7789_WRCACE   0x55
#define ST7789_RDCABC   0x56
#define ST7789_WRCABCMB   0x5E  // Write CABC minimum brightness
#define ST7789_RDCABCMB   0x5F  // Read CABC minimum brightness
#define ST7789_RDABCSDR   0x68  // ReaD Automatic Brightness control Self-Diagnostic Result
/* [...] */

// ST7789 System Controls
#define ST7789_RAMCTRL 0xB0     // RAM control
#define ST7789_RGBCTRL 0xB1     // RGB interface control
#define ST7789_PORCTRL 0xB2     // Porch setting
#define ST7789_FRCTRL1   0xB3    // Frame Rate control in normal mode
#define ST7789_PARCTRL   0xB5   // Partial Control
#define ST7789_GCTRL   0xB7   // Gate Control
#define ST7789_GTADJ   0xB8   // Gate On Timing Adjustment
#define ST7789_DGMEN   0xBA   // Digital Gamma enable
#define ST7789_VCOMS   0xBB   // VCOM Setting
#define ST7789_POWSAVE   0xBC   // Power saving mode
#define ST7789_DLPOFFSAVE   0xBD   // Display off power save

// LCM Control (MY, RGB Endian, Color inversion, )
#define ST7789_LCMCTRL   0xC0
#define ST7789_IDSET   0xC1     // ID Code Setting
#define ST7789_VDVVRHEN   0xC2   // VDV/VRH Command enable
#define ST7789_VRHS   0xC3   // VRH Set
#define ST7789_VDVS   0xC4   // Frame Rate Control 1 (Partial mode/idle colors)
#define ST7789_VCMOFSET   0xC5   // VCM offset set
#define ST7789_FRCTRL2   0xC6    // Frame Rate control in normal mode
#define ST7789_CABCCTRL   0xC7  // CABC control
#define ST7789_REGSEL1   0xC8   // Register Value Selection 1
#define ST7789_REGSEL2   0xCA   // Register Value Selection 2
#define ST7789_PWMFRSEL   0xCC   // PWM frequency selection

//
#define ST7789_PWCTRL1   0xD0   // Power control 1
#define ST7789_VAPVANEN   0xD2   // Enable VAP/VAN signal output
#define ST7789_CMD2EN   0xDF   // Command 2 Enable

// 
#define ST7789_PVGAMCTRL   0xE0   // Positive voltage Gamma control
#define ST7789_NVGAMCTRL   0xE1   // Negative voltage Gamma Control
#define ST7789_DGMLUTR   0xE2   // Digital Gamma LUT for red
#define ST7789_DGMLUTB   0xE3   // Digital Gamma LUT for blue
#define ST7789_GATECTRL   0xE4   // Gate Control
#define ST7789_SPI2EN   0xE7   // SPI2 Enable
#define ST7789_PWCTRL2   0xE8   // Power Control 2
#define ST7789_EQCTRL   0xE9   // Equalize time control
#define ST7789_PROMCTRL   0xEC   // Program mode control

// 
#define ST7789_PROMEN   0xFA   // Program mode enable
#define ST7789_NVMSET   0xFC   // NVM setting
#define ST7789_PROMACT   0xFE   // Program action

#endif