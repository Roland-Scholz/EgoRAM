# EgoRAM

The Atari 8-bit RAM cartridge with an own ego and opinion what should be in the RAM.


$D500-$D5FF
===========


| Address | Name | Descriptio Read | Write |
|---------+------+------------|
| $D500   |      | 'E'        |
| $D501   |      | 'R'        |
| $D502   |      | 'A'        |
| $D503   |      | 'M'        |
| $D504   |      | $YY        |
| $D505   |      | $YY        |
| $D506   |      | $MM        |
| $D507   |      | $DD        |


| $D508   |  ECTL  | $00=ready, $01=busy, $8x=error  | $00=reset,$01=begin command |
| $D509   |  EDAT  | Data, increments address        | Data to write, increments address |
| $dD0a   |  ERET  | n/a                             | Strobe, run command |


| Command | Name     | Arguments | Description 
| G       | Graphics | mode: byte (8,9,15), address: word ($8000...$bffff), width: byte (bytes 32,40,48), height: byte (lines 1..255) | width*height<$2000
| SD      | Shape Define | shapeNumber: word ($0001-$ffff), bpp: ($01, $02, $04), width: word (pixels 1..256), height: word (pixels 1..256), format: byte ($01=raw), paletteSize: word, paletteMap: array[paletteSize], maskIndex: byte (0..paletteSize), data: array[0...dataSize-1] | dataSize = 

[Getting started with Raspberry Pi
Pico-series](https://pip-assets.raspberrypi.com/categories/610-raspberry-pi-pico/documents/RP-008276-DS-1-getting-started-with-pico.pdf)
If you get an error message "No accessible RP2040/RP2350 devices in BOOTSEL mode were found." accompanied by a note similar to Device at bus 1, address 7 appears to beana RP2040 device in BOOTSEL mode, but picotool was unable to connect, indicating that there was a Pico-series device connected, then you can run picotool using sudo, e.g.
$ sudo picotool info -a
If you get this message on Windows, you will need to install a driver. Download and run [Zadig](https://zadig.akeo.ie/), select RP2 Boot (Interface 1) from the dropdown box and select WinUSB as the driver, and click on the "Install Driver" button. Wait for the installation to complete - this may take a few minutes.