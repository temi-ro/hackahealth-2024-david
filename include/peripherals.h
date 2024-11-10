#pragma once

#include <Arduino.h>

#include <Adafruit_NeoPixel.h>


// #define BTN_DAVID D1    
#define BTN_SNOOZE D0    
#define LED_PIN D2   
#define BUZZER_PIN D5

constexpr unsigned long N_LEDS = 10;
bool snoozePressed = false;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);


void handleSnoozePress() {
    Serial.println("Snooze button pressed");
    detachInterrupt(digitalPinToInterrupt(BTN_SNOOZE));
    digitalWrite(BUZZER_PIN, LOW);
    for(int i=0; i<10; i++) {
        strip.setPixelColor(i, 0, 0, 0);
    }
    // strip.show();
    snoozePressed = true;
}

void handleCancelPress() {
    Serial.println("Cancel button pressed");
    detachInterrupt(digitalPinToInterrupt(BTN_SNOOZE));
    digitalWrite(BUZZER_PIN, LOW);
    for(int i=0; i<10; i++) {
        strip.setPixelColor(i, 0, 0, 0);
    }
}

 
void handleServiceRequestPress() {
    pinMode(LED_PIN, OUTPUT);
    #ifdef BTN_DAVID
        pinMode(BTN_SNOOZE, INPUT_PULLUP);
    #endif
    attachInterrupt(digitalPinToInterrupt(BTN_SNOOZE), handleSnoozePress, RISING);
    Serial.println("Request triggered");
    digitalWrite(BUZZER_PIN, HIGH);
    for(int i=0; i<10; i++) {
     strip.setPixelColor(i, 255, 0, 0);
    }
    // strip.show();
}


void setupPeripherals() {
    // TODO: INPUT_PULLDOWN
    pinMode(BTN_SNOOZE, INPUT_PULLUP);    
    pinMode(BUZZER_PIN, OUTPUT); 
    #ifdef BTN_DAVID
        pinMode(BTN_DAVID, INPUT_PULLUP);  
        attachInterrupt(digitalPinToInterrupt(BTN_DAVID), handleServiceRequestPress, RISING);
    #endif
    // Attach interrupt for BTN_SNOOZE but disable it initially
    attachInterrupt(digitalPinToInterrupt(BTN_SNOOZE), handleSnoozePress, RISING);
    detachInterrupt(digitalPinToInterrupt(BTN_SNOOZE));
    strip.begin();

    for(int i=0; i<10; i++) {
        strip.setPixelColor(i, 0, 0, 0);
        // strip.show();
    }
}


void loopPheripherals(){
    strip.show();
    delayMicroseconds(100);
}
 