
/*
      Sumber Kode : https://www.youtube.com/redirect?event=video_description&redir_token=QUFFLUhqbXg3UlF0WldTei1RMnJfRUh1RkpxWG96R19PQXxBQ3Jtc0tudnQyd05XbWR1My1ZczR0Tk1VdHZOM1hlTy1oc2otWU42VjVzOWgweXcxdTB1V3FVdE1HeXctVjZYX0ZFZ19vdzNPeEZoQlFJbWJzNWI4YnU1MkFkaGZxdUNHa2FiV1l5UXdDX3hkeUJCWmZJQy1GWQ&q=https%3A%2F%2Fgithub.com%2FTheDIYGuy999%2FRc_Engine_Sound&v=61Ogc1pHduk
      Diedit Oleh : Edrick Hoki
      Subscribe : https://www.youtube.com/ngilmuminatiayoindonesiakreatif
*/

#include "settings.h"
#include "curves.h" 
#define SPEAKER 3         // Output Audio di pin 3
#define THROTTLE_INPUT 2  // Throttle Signal Terhubung di pin 2
#define FREQ 16000000L                          

// Define global variables
volatile uint16_t currentSmpleRate = BASE_RATE; 
volatile uint16_t fixedSmpleRate = FREQ / BASE_RATE; 
volatile uint8_t engineState = 0;     // 0 = off, 1 = starting, 2 = running, 3 = stopping
volatile boolean engineOn = false;
volatile uint16_t curEngineSample;              
volatile uint16_t curStartSample;               

uint16_t  currentThrottle = 0;                  
volatile int16_t pulseWidth = 0;                
volatile boolean pulseAvailable;                

int16_t pulseMaxNeutral; 
int16_t pulseMinNeutral;
int16_t pulseMax;
int16_t pulseMin;
int16_t pulseMaxLimit;
int16_t pulseMinLimit;


/* =======================================================================================================
   SETUP
   =======================================================================================================*/

void setup() {

  attachInterrupt(0, getPulsewidth, CHANGE);

  if (!engineManualOnOff) pulseZero = pulseWidth;

  // Calculate throttle range
  pulseMaxNeutral = pulseZero + pulseNeutral;
  pulseMinNeutral = pulseZero - pulseNeutral;
  pulseMax = pulseZero + pulseSpan;
  pulseMin = pulseZero - pulseSpan;
  pulseMaxLimit = pulseZero + pulseLimit;
  pulseMinLimit = pulseZero - pulseLimit;

  setupPcm();
}


/* =======================================================================================================
   SETUP PCM (Script konfigurasi Suara)
   =======================================================================================================*/

void setupPcm() {

  pinMode(SPEAKER, OUTPUT);

  ASSR &= ~(_BV(EXCLK) | _BV(AS2));                         

  TCCR2A |= _BV(WGM21) | _BV(WGM20);                        
  TCCR2B &= ~_BV(WGM22);

  TCCR2A = (TCCR2A | _BV(COM2B1)) & ~_BV(COM2B0);           
  TCCR2A &= ~(_BV(COM2A1) | _BV(COM2A0));                   
  TCCR2B = (TCCR2B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10); 

  OCR2B = pgm_read_byte(&idle_data[0]);                     

  // Set up Timer 1 to send a sample every interrupt.
  cli();

  TCCR1B = (TCCR1B & ~_BV(WGM13)) | _BV(WGM12);             
  TCCR1A = TCCR1A & ~(_BV(WGM11) | _BV(WGM10));             

  TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10); 

  OCR1A = FREQ / BASE_RATE;                                
  // OCR1A is a 16-bit register, so we have to do this with
  // interrupts disabled to be safe.

  TIMSK1 |= _BV(OCIE1A);                                   


  curEngineSample = 0;
  curStartSample = 0;
  //curHornSample = 0;

  sei();
}


/* =======================================================================================================
   MAP PULSEWIDTH TO THROTTLE
   =======================================================================================================*/


void mapThrottle() {

  if (pulseWidth > pulseMinLimit && pulseWidth < pulseMaxLimit) {
    if (pulseWidth < pulseMin) pulseWidth = pulseMin; 
    if (pulseWidth > pulseMax) pulseWidth = pulseMax;
    if (pulseWidth > pulseMaxNeutral) currentThrottle = (pulseWidth - pulseZero) * 2;
    else if (pulseWidth < pulseMinNeutral) currentThrottle = abs( (pulseWidth - pulseZero) * 2);
    else currentThrottle = 0;
  }
}


/* =======================================================================================================
   ENGINE SIMULATION
   =======================================================================================================*/

void engineMassSimulation() {

  static int16_t  mappedThrottle = 0;
  static int16_t currentRpm = 0;
  static unsigned long throtMillis;

  if (millis() - throtMillis > 5) { 
    throtMillis = millis();

    // compute unlinear throttle curve
    mappedThrottle = reMap(curveShifting, currentThrottle);

    // Accelerate engine
    if (mappedThrottle + acc > currentRpm && engineState == 2) {
      currentRpm += acc;
      if (currentRpm > maxRpm) currentRpm = maxRpm;
    }

    // Decelerate engine
    else if (mappedThrottle - dec < currentRpm) {
      currentRpm -= dec;
      if (currentRpm < minRpm) currentRpm = minRpm;
    }

    // Speed (sample rate) output
    currentSmpleRate = FREQ / (BASE_RATE + long(currentRpm * TOP_SPEED_MULTIPLIER) );
  }
}


/* =======================================================================================================
   SWITCH ENGINE ON OR OFF
   =======================================================================================================*/

void engineOnOff() {

  static unsigned long pulseDelayMillis;
  static unsigned long idleDelayMillis;

  if (engineManualOnOff) { // Mesin dihidupkan atau dimatikan secara manual tergantung pada keberadaan dari servo
    if (pulseAvailable) pulseDelayMillis = millis(); 

    if (millis() - pulseDelayMillis > 100) {
      engineOn = false; // Setelah delay 0,1 sekon, Matikan suara
    }
    else engineOn = true;
  }
  else { // Suara akan otomatis mati ketika Throttle tidak memberikan Sinyal ke board
    if (currentThrottle > 80) idleDelayMillis = millis(); // reset delay timer, Jika Throttle memberikan sinyal
    
    if (millis() - idleDelayMillis > 60000) {  // satuan milisekon, 1 sekon = 1000
      engineOn = false; // Setelah delay 60 sekon, Matikan suara
    }
    else {
      if (currentThrottle > 100) engineOn = true;
    }
  }
}


/* =======================================================================================================
   MAIN LOOP
   =======================================================================================================*/


void loop() {

  // Map pulsewidth ke throttle
  mapThrottle();

  // Simulate engine mass, Memuat sinyal RPM
  engineMassSimulation();

  // Otomatis suara nyala mati
  engineOnOff();
}


/* =======================================================================================================
   ERRORS
   =======================================================================================================*/

void getPulsewidth() {

  unsigned long currentMicros = micros();
  boolean currentState = PIND & B00000100; 
  static unsigned long prevMicros = 0;
  static boolean lastState = LOW;

  if (lastState == LOW && currentState == HIGH) {    
    prevMicros = currentMicros;
    pulseAvailable = true;
    lastState = currentState;
  }
  else if (lastState == HIGH && currentState == LOW) { 
    pulseWidth = currentMicros - prevMicros;
    pulseAvailable = false;
    lastState = currentState;
  }
}
ISR(TIMER1_COMPA_vect) 
{

  static float attenuator;

  switch (engineState) {

    case 0: // off ----
      OCR1A = fixedSmpleRate; 
      OCR2B = 0;
      if (engineOn) engineState = 1;
      break;

    case 1: // starting ----
      if (curStartSample >= start_length) { 
        curStartSample = 0;
        engineState = 2;
      }
      OCR1A = fixedSmpleRate; 
      OCR2B = pgm_read_byte(&start_data[curStartSample]);
      curStartSample++;
      break;

    case 2: // running ----
      if (curEngineSample >= idle_length) { 
        curEngineSample = 0;
        attenuator = 1;
      }
      OCR1A = currentSmpleRate; 
      OCR2B = pgm_read_byte(&idle_data[curEngineSample]);
      curEngineSample++;
      if (!engineOn) {
        engineState = 3;
      }
      break;

    case 3: // stopping ----
      if (curEngineSample >= idle_length) { 
        curEngineSample = 0;
      }

      OCR1A = fixedSmpleRate;
      //OCR1A = fixedSmpleRate * attenuator; // engine slowing down
      OCR2B = pgm_read_byte(&idle_data[curEngineSample]) / attenuator;
      curEngineSample++;
      attenuator += 0.002; // Efek fade Out suara Utama ketika dimatikan 0.002
      if (attenuator >= 20) {  // 3 - 20
        engineOn = false;
        if (!engineOn) engineState = 0; 
      }
      break;

  } 
}
