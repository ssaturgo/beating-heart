#include <NeoPixelBus.h>        // NeoPixel Library
#include <NeoPixelAnimator.h>   // LED Animator
#include <Wire.h>               // i2C communication
#include "MAX30105.h"           // Heart rate sensor
#include "heartRate.h"          //Heart rate calculating algorithm

// define colors
RgbColor BLACK(0,0,0);
RgbColor RED(255,0,0);
RgbColor GOLD(255,140,0);


// define objects
MAX30105 pulseSensor;
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(16, 12); // strip(ledCount, ledPin)
NeoPixelAnimator animations(2); // NeoPixel animation management

// variables
const byte RATE_SIZE = 4; // Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; // Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; // time since last heart beat was read
float beatsPerMinute;
int beatAvg;

void setup() {
  Serial.begin(115200); // initialize serial monitor
  // initRandom(); // initialize random number generator

  // initialize LEDs
  strip.Begin();
  // strip.ClearTo(GOLD);
  // strip.Show();
  delay(1000);
  Serial.println("LEDs initialized");

  // Initialize sensor
  pulseSensor.begin(Wire, I2C_SPEED_FAST); // using default i2C port / 400khz speed
  pulseSensor.setup(); // setup sensor with default settings
  pulseSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  delay(1000);
  Serial.println("Sensor initialized");
}

void loop() {
  long irValue = pulseSensor.getIR(); // get IR value to check if finger is on the sensor or not

  // detected finger
  if (irValue > 7000) { 
    if (lastBeat == 0 && !animations.IsAnimating()) {
      animations.StartAnimation(1, 2000, fadeAnimation);
    }

    // check if we sense a heart beat
    if (checkForBeat(irValue) == true) {
      long delta = millis() - lastBeat; // measure duration between two beats
      lastBeat = millis();
      beatsPerMinute = 60 / (delta / 1000.0); // calculate bpm
  
      if (beatsPerMinute < 255 && beatsPerMinute > 20) { // To calculate the average we strore some values (4) then do some math to calculate the average
        rates[rateSpot++] = (byte) beatsPerMinute; // store result to array
        rateSpot %= RATE_SIZE;
  
        // average the result
        beatAvg = 0;
        for (byte x = 0 ; x < RATE_SIZE ; x++) {
          beatAvg += rates[x];
        }
        beatAvg /= RATE_SIZE;
        Serial.print("Heart rate : ");
        Serial.println(beatAvg);
      }

      // display heartbeat on led
      if (beatsPerMinute < 255 && beatsPerMinute > 20) {
        animations.StopAnimation(1);
        animations.StartAnimation(0, 500, beatAnimation);
      }
    }
  }

  // if finger NOT detected
  // set average bpm to 0
  else {
    beatAvg = 0;
    lastBeat = 0;
    animations.StopAnimation(1);
    strip.ClearTo(BLACK);
  }

  animations.UpdateAnimations();
  strip.Show();
  // delay(15);
}


// animations

// heart pulse animation
void beatAnimation(const AnimationParam& param) {
  strip.ClearTo(RgbColor::LinearBlend(RED, BLACK, param.progress));
}

// fading animation
void fadeAnimation(const AnimationParam& param) {
  float index = param.progress * 2;

  // fade in
  if (index < 1) {
    strip.ClearTo(RgbColor::LinearBlend(BLACK, GOLD, index));
  }
  // fade out
  else {
    strip.ClearTo(RgbColor::LinearBlend(GOLD, BLACK, index - 1));
  }

  if (param.state == AnimationState_Completed) {
    animations.RestartAnimation(1);
  }
}









