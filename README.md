# RP2040-Pi-Zero-DVI-Output
Scaffolding to run DVI output on a Waveshare RP2040 Pi Zero

#### vi,vim,nano into Documents\Arduino\libraries\   
PicoDVI_-_Adafruit_Fork\software\include\common_dvi_pin_configs.h

add: 

```bash
// Waveshare RP2040-PiZero DVI configuration
static const struct dvi_serialiser_cfg rp2040pizero_cfg = {
  .pio = DVI_DEFAULT_PIO_INST,
  .sm_tmds = {0, 1, 2},
  .pins_tmds = {26, 24, 22},
  .pins_clk = 28,
  .invert_diffpairs = false
};
```
Before last end if.

Tools > USB Stack > Pico SDK or Adafruit TinyUSB
