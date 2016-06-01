    // Simple NeoPixel test.  Lights just a few pixels at a time so a
    // 1m strip can safely be powered from Arduino 5V pin.  Arduino
    // may nonetheless hiccup when LEDs are first connected and not
    // accept code.  So upload code first, unplug USB, connect pixels
    // to GND FIRST, then +5V and digital pin 6, then re-plug USB.
    // A working strip will show a few pixels moving down the line,
    // cycling between red, green and blue.  If you get no response,
    // might be connected to wrong end of strip (the end wires, if
    // any, are no indication -- look instead for the data direction
    // arrows printed on the strip).
     
    #include <Adafruit_NeoPixel.h>
     
    #define PIN      13
    #define N_LEDS   15
     
    Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, PIN, NEO_GRBW + NEO_KHZ800);
     
    void setup() {
      strip.begin();
      strip.show();
    }

    void glow(uint32_t c) {
      for(uint16_t i=0; i<strip.numPixels(); i++) {
          strip.setPixelColor(i  , c); // Draw new pixel
          strip.show();
      }
      delay(500);

    }
     
    void loop() {
     /* chase(strip.Color(0, 255, 0)); // Green
      glow(strip.Color(0, 255, 0));*/

      strip.setPixelColor(14  ,100, 0, 0, 0);
      strip.setPixelColor(13 , 0, 100, 0, 0);
      strip.setPixelColor(12 , 0, 0, 100, 0);
      strip.setPixelColor(11 , 0, 0, 0, 100);
      strip.show();
      delay(500);
    }
     
    static void chase(uint32_t c) {
      for(uint16_t i=0; i<strip.numPixels()+4; i++) {
          strip.setPixelColor(i  , c); // Draw new pixel
          strip.setPixelColor(i-4, 0); // Erase pixel a few steps back
          strip.show();
          delay(25);
      }
    }
