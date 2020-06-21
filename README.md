# 7-segment-clock

## 3D printing

The clock is modular. In order to create a 4 digit clock I printed:

I designed all of the following components in Fusion 360

* 1x clock-body-center-connector.stl
* 4x clock-body-digit.stl
* 2x clock-body-digit-connector.stl
* 2x clock-body-end.stl

Settings I used:

* 1.75mm PLA brass filament
* 0.2mm layer height
* 8 perimeter brim
* 5% gyroid infill
* 205 celcius nozzel
* 60 celcius bed

Print time was about 60-70 hours. 

After printing I heat set m3 threadded inserts into the plastic so it could be screwed to the base plate.

## Laser cutting

### Backplate

The baseplate was laser cut from 3mm ply wood. There are holes for zip ties for cable management and holes for m3 bolts to attach the base plate to the 3D printed clock body.

I cut one piece:

* baseplate.svg

### Diffusers

The diffusers are placed above the LEDs and make the light softer and more even. I used 3mm clear acrylic and etched lines in it to make it very textued. The red lines are cut lines and the blue lines are etch lines that are etched at twice the speed of the cut lines so they don't go all the way through. I think opal acrylic without the etch lines would have also looked very nice.

I cut these pices:

* 4x digit-diffuser.svg
* 2x circle-diffuser.svg

## Electronics

A WS2812B 144led/m strip was cut into lengths of 9 LEDs and stuck to the baseplate within the etch lines. Two lengths that are 2 LEDs long are also cut.

Stripped solid core wire is soldered to the strips to connect them in a snake pattern.

An ESP32 is placed in the center to control the LED strips.

## Programming

Code is uploaded to the ESP32 to control the LEDs.

## Todo

* Step up data signal to 5V with level shifter
* Laser cut diffusers
* 1000uF capacitor on each strip
* ESP32 web server to change colours and run functions