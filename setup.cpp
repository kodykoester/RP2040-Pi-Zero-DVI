/*
 * ============================================================
 * Waveshare RP2040-PiZero DVI Scaffold
 * ============================================================
 *
 * Board:
 *   Waveshare RP2040-PiZero
 *
 * Display:
 *   DVI / HDMI output
 *   PicoDVI
 *   320x240 @ 60 Hz
 *
 * Arduino IDE setup:
 *
 *   Board:
 *     Waveshare RP2040-PiZero
 *
 *   Required libraries:
 *     PicoDVI
 *
 * IMPORTANT:
 *   The Waveshare PiZero requires the correct PicoDVI
 *   board configuration.
 *
 *   The configuration used by the display constructor is:
 *
 *       rp2040pizero_cfg
 *
 *   This comes from the PicoDVI board configuration and
 *   must be available to the compiler.
 *
 * ============================================================
 *
 * Typical PicoDVI setup:
 *
 *     #include <PicoDVI.h>
 *
 *     DVIGFX16 display(
 *         DVI_RES_320x240p60,
 *         rp2040pizero_cfg
 *     );
 *
 * ============================================================
 */

// ------------------------------------------------------------
// DVI CONFIGURATION
// ------------------------------------------------------------

#include <PicoDVI.h>

// Waveshare RP2040-PiZero DVI configuration
DVIGFX16 display(
    DVI_RES_320x240p60,
    rp2040pizero_cfg
);

// ------------------------------------------------------------
// ------------------------------------------------------------

void setup()
{

    display.begin();


    // --------------------------------------------------------
    // TEST SCREEN
    // --------------------------------------------------------

    display.fillScreen(0x0000);

    display.setTextSize(2);
    display.setTextColor(0xFFFF);
    display.setCursor(40, 40);

    display.print("RP2040-PiZero");

    display.setCursor(40, 70);
    display.print("DVI WORKING");

    display.setTextSize(1);
    display.setCursor(40, 110);
    display.print("320x240 @ 60Hz");
}


// ------------------------------------------------------------
// -----------------------------------------------------------

void loop()
{
 
}
