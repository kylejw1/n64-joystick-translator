# Theory of Operation #

---


Most josticks today are made with potentiometers. The potentiometer will output a voltage proportional to the amount a joystick is pushed. [Potentiomers on Wikipedia](http://en.wikipedia.org/wiki/Potentiometer)

However, the N64 joystick works in a different way, using two rotary encoders. This is essentially the same as the way ball computer mice work (though they have all but died out). [Rotary Encoders on Wikipedia](http://en.wikipedia.org/wiki/Rotary_encoder)

## Code Flow ##

To successfully swap the rotary style joystick with the potentiometer style joysticks, the potentiometer voltages must be translated into rotary encoder-esque signals. This is accomplished with a simple microcontroller following the following flow: (only one axis is shown, in reality everything here happens twice for two dimensions)

![http://n64-joystick-translator.googlecode.com/files/how_it_works.jpg](http://n64-joystick-translator.googlecode.com/files/how_it_works.jpg)

## Translated Output State Machine ##

The upward, downward, left, and right movements follow a simple state machine:

![http://n64-joystick-translator.googlecode.com/files/state_machine.jpg](http://n64-joystick-translator.googlecode.com/files/state_machine.jpg)