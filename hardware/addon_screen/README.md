# Optional display #

This modification to FunBox allows you to add a small OLED screen to get visual feedback 
about the state of the pedal, which helps to make the pedal a lot more self-documenting.

## Demo ##

You can view a demo video [here](https://youtube.com/shorts/V6C6PNf6iQQ?feature=share), showing
the Neptune pedal in action.  

The screen is very flickery in the video due to clashes between the refresh rate and the camera frame rate, but to human eyes it looks nice and steady.

## Hardware ##

You need an SSD1306-based display, with I2C interface.  I used [this one](https://www.aliexpress.com/item/1005007614149117.html).  Note that various different interfaces are possible for these
displays, but I2C works well with FunBox because the hardware I2C pins on the DaisySeed are currently unused.

## Installation ##

You need to choose where to mount the screen.  I chose the front of the pedal, as seen in the video, although it's possible that would be problematic if you want to mount the pedal in a pedal board.

If you do mount to the front, it's convenient to drill a small hole halfway between the two footswitches to pass the wires through the pedal box, underneath the screen.

I've provided an STL file to 3D print an enclosure for the screen, which can be glued to the pedal case.

## Connection ##

There are no dedicated holes drilled for the screen connections on the standard FunBox PCB, but
you can easily connect wires from the screen to the existing soldered connections on the back of the PCB for the IC socket for the DaisySeed.

The correct pins are:
* Screen SCL -> Daisy D11
* Screen SDA -> Daisy D12
* Screen GND -> Daisy DGND
* Screen VCC -> Daisy 3V3 Digital

See [here](https://daisy.audio/hardware/Seed/#pinout) for pin locations.

## Software ##

This fork contains all the necessary code to drive the screen, built into each pedal.  

The screen-specific code is all in  funbox_display.h / funbox_display.cpp.  See the
individual planet code for how to integrate with new pedal code.