#include <TimerOne.h>

#define CLOCKS_PER_BEAT 8
#define MINIMUM_BPM 10
#define MAXIMUM_BPM 340
#define BPM_INCREMENT 10

#define MIN_DIVISION 2
#define MAX_DIVISION 9 //one more than max

#define DEBUG false

const int resetMaxCapacitanceButton = 10;
const int BPMDownPin = 0;
const int BPMUpPin = 1;

const int syncInputPin = 21;
const byte ledPin = 13;
volatile byte state = LOW;

elapsedMillis syncIn1;

bool buttonPressed = false;

volatile int currentBPM = 120;

#define NUM_SENSORS 8
volatile int sensors[NUM_SENSORS];
int pins[NUM_SENSORS];
int mins[NUM_SENSORS];
int maxs[NUM_SENSORS];
int increments[NUM_SENSORS];
int sensorsCurrent[NUM_SENSORS];

void timerFunction() {
  // Loop over division array and pulse output pin if division can be made
  for(int i=0; i < NUM_SENSORS; i++) {
    
    
    if(sensors[i] > MAX_DIVISION-1 || sensorsCurrent[i] <= 1500) {
      digitalWrite(i+2, LOW);
      increments[i] = 0;
    } else {
      if((increments[i] % sensors[i]) == 0) {
        digitalWrite(i+2, HIGH);
      } else {
        digitalWrite(i+2, LOW);
      }
    }

    // Reset time on 16th beat
    if(increments[i] >= 16) {
      increments[i] = 1;
    } else {
      increments[i]++;
    }
    
  }
}

void resetMaxCapacitance() {
  for(int i=0; i < NUM_SENSORS; i++) {
    maxs[i] = 1500;
  }
}

void setup() {
  // Set up pin modes
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);

  pinMode(resetMaxCapacitanceButton, INPUT_PULLUP);

  // Initialise division array and increments array
  for(int i=0; i < NUM_SENSORS; i++) {
    sensors[i] = 9;
    increments[i] = 1;
  }

  // Initialise max array
  resetMaxCapacitance();

  // Add sensor pins to array
  pins[0] = A1;
  pins[1] = A2;
  pins[2] = A3;
  pins[3] = A4;
  pins[4] = A5;
  pins[5] = A8;
  pins[6] = A9;
  pins[7] = 25;

  // Start Serial
  Serial.begin(9600);

  // Attach interrupt to syncInputPin
  pinMode(ledPin, OUTPUT);
  pinMode(syncInputPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(syncInputPin), flash, RISING);


  // Begin Interrupt Timer
  Timer1.initialize(calculateIntervalMicroSecs(currentBPM));
  Timer1.attachInterrupt(timerFunction);
}

void loop() {
  // Read sensors, store minimum and maximum readings, map value to available divisions
  for(int i=0; i < NUM_SENSORS; i++) {
    int readValue = touchRead(pins[i]);
    if(mins[i] > readValue) mins[i] = readValue;
    if(maxs[i] < readValue) maxs[i] = readValue;
    sensors[i] = map(readValue, mins[i], maxs[i], MAX_DIVISION, MIN_DIVISION);
    sensorsCurrent[i] = readValue;
    
    if(i==0) {
      Serial.println(readValue);
      //Serial.print("MAX:");
      //Serial.println(maxs[i]);
      //Serial.print("MIN:");
      //Serial.println(mins[i]);
      Serial.print("DIV:");
      Serial.println(sensors[i]);
    }
    
  }
  
  // BPM control
  int BPMDownReading = touchRead(BPMDownPin);
  int BPMUpReading = touchRead(BPMUpPin);

  if(BPMDownReading < 2000 && BPMUpReading < 2000) {
    buttonPressed = false;
  }

  if(BPMDownReading > 2000 && !buttonPressed) {
    buttonPressed = true;
    //state = LOW;
    BPMDown();
  }

  if(BPMUpReading > 2000 && !buttonPressed) {
    buttonPressed = true;
    //state = LOW;
    BPMUp();
  }

  // Capacitance reset button
  if (digitalRead(resetMaxCapacitanceButton) == LOW) {
    resetMaxCapacitance();
  }

  digitalWrite(ledPin, state);
}

long calculateIntervalMicroSecs(int bpm) {
  return 60L * 1000 * 1000 / bpm / CLOCKS_PER_BEAT;
}

void BPMUp() {
  if((currentBPM + BPM_INCREMENT) > MAXIMUM_BPM) return;
  
  currentBPM += BPM_INCREMENT;
  
  Timer1.setPeriod(calculateIntervalMicroSecs(currentBPM));
  
  if(DEBUG) Serial.print("BPM UP\t\t");
  if(DEBUG)  Serial.println(currentBPM);
  delay(75);
}

void BPMDown() {
  if((currentBPM - BPM_INCREMENT) < MINIMUM_BPM) return;
  
  currentBPM -= BPM_INCREMENT;
  
  Timer1.setPeriod(calculateIntervalMicroSecs(currentBPM));
  
  if(DEBUG) Serial.print("BPM DOWN\t");
  if(DEBUG) Serial.println(currentBPM);
  delay(75);
}

void flash() {
  state = !state;

  if(!state) {
    syncIn1 = 0;
  }
  
  if(state) {
    Serial.println(syncIn1);
    calculateIntervalMicroSecs(syncIn1);
  }
}
