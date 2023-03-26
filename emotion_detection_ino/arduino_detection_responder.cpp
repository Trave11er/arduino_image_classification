/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#if defined(ARDUINO) && !defined(ARDUINO_ARDUINO_NANO33BLE)
#define ARDUINO_EXCLUDE_CODE
#endif  // defined(ARDUINO) && !defined(ARDUINO_ARDUINO_NANO33BLE)

#ifndef ARDUINO_EXCLUDE_CODE

#include "detection_responder.h"

#include "Arduino.h"

// Flash the blue LED after each inference
void RespondToDetection(float frown_score, float happy_score, float neutral_score) {
  static bool is_initialized = false;
  if (!is_initialized) {
    // Pins for the built-in RGB LEDs on the Arduino Nano 33 BLE Sense
    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    pinMode(LEDB, OUTPUT);
    is_initialized = true;
  }

  // Note: The RGB LEDs on the Arduino Nano 33 BLE
  // Sense are on when the pin is LOW, off when HIGH.
  // Switch the person/not person LEDs off
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDR, HIGH);

  // Flash the blue LED after every inference.
  digitalWrite(LEDB, LOW);
  delay(100);
  digitalWrite(LEDB, HIGH);
  // Switch on the green LED when a person is detected,
  // the red when no person is detected
  if (frown_score > happy_score && frown_score > neutral_score) {
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDR, LOW);
  } else if (happy_score > frown_score && happy_score > neutral_score) {
    digitalWrite(LEDG, LOW);
    digitalWrite(LEDR, HIGH);
  } else {
    digitalWrite(LEDG, HIGH);
    digitalWrite(LEDR, HIGH);
  }
  /*
  Serial.print(frown_score);
  Serial.print(" ");
  Serial.print(happy_score);
  Serial.print(" ");
  Serial.print(neutral_score);
  Serial.print("\n");
  */
  byte scores[3] = {static_cast<byte>(255*frown_score), static_cast<byte>(255*happy_score), static_cast<byte>(255*neutral_score)};
  Serial.write(scores, 3);
}


#endif  // ARDUINO_EXCLUDE_CODE
