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