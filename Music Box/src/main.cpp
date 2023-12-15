#include <Arduino.h>
#include <LedControl.h>
#include <LiquidCrystal.h>

const int rs = 8, e = 7, d4 = 6, d5 = 5, d6 = 4, d7 = 3;
LiquidCrystal lcd(rs, e, d4, d5, d6, d7);
LedControl lc=LedControl(12,10,11,1);

#define NEXT_PIN A4
#define PREV_PIN A2
#define SAVE_PIN A3
#define TONE_PIN 9
#define PLAY_PIN 2

#define noOfButtons 3
#define bounceDelay 20
#define minButtonPress 3

#define TONE_USE_INT
#define TONE_PITCH 440

#include <TonePitch.h>

const int buttonPins[] = {NEXT_PIN, PREV_PIN, SAVE_PIN};
uint32_t previousMillis[noOfButtons];
uint8_t pressCount[noOfButtons];
uint8_t testCount[noOfButtons];


int count = 0;
int selected = 0;
int playing = 0;

int numMatrix[8][8];

int freqMatrix[8][8];

int noteMatrix[8][8];

int timeMatrix[8][8];

byte ledMatrix[8];

const int noteFrequencies[36] = {
  130, 138, 146, 155, 164, 174, 185, 196, 207, 220, 233, 246, 261, 277, 293, 311, 329, 349, 369, 392, 415, 440, 466, 493, 523, 554, 587, 622, 659, 698, 739, 783, 830, 880, 932, 987
};

const String noteNames[37] = {
  "   ", "C3 ", "C3#", "D3 ", "D3#", "E3 ", "F3 ", "F3#", "G3 ", "G3#", "A3 ", "A3#", "B3 ", "C4 ", "C4#", "D4 ", "D4#", "E4 ", "F4 ", "F4#", "G4 ", "G4#", "A4 ", "A4#", "B4 ", "C5 ", "C5#", "D5 ", "D5#", "E5 ", "F5 ", "F5#", "G5 ", "G5#", "A5 ", "A5#", "B5 "
};

int mapNotes(int freqIn) {
  int index = floor(map(freqIn, 0, 1023, 0, sizeof(noteFrequencies) / sizeof(noteFrequencies[0])));
  int noteFrequency = noteFrequencies[index];
  return noteFrequency;
}

int mapNotesName(int freqIn) {
  int index = floor(map(freqIn, 0, 1023, 0, sizeof(noteNames) / sizeof(noteNames[0])));
  return index;
}

void numToLed(int numMatrix[8][8], byte ledMatrix[8]) {
  for (int i = 0; i < 8; i++)
  {
    byte result = 0;
    for (int j = 0; j < 8; j++)
    {
      result |= (numMatrix[i][j] << j);
    }
    ledMatrix[i] = result;
  } 
}

void save(int row, int col) {
  numMatrix[row][7-col] = 1;
}

void select() {
  if (selected == 1) {
        selected = 0;
      }
  else {
        selected = 1;
      }
}

void playStart() {
  Serial.println("pressed play");
  if (playing == 1)
  {
    playing = 0;
  }
  else
  {
    playing =  1;
  }
}

void setup() {
  lc.shutdown(0, false);
  lc.setIntensity(0, 10);
  lc.clearDisplay(0);

  lcd.begin(16, 2);

  pinMode(TONE_PIN, OUTPUT);
  pinMode(PLAY_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PLAY_PIN), playStart, FALLING);

  uint8_t i;
  Serial.begin(115200);

  for (i=0; i<noOfButtons; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  numToLed(numMatrix, ledMatrix);

  TCCR0A = (1 << WGM01);
  OCR0A = 0xF9; //(16*10^6) / 1000*64
  TIMSK0 |= (1 << OCIE0A);

  sei();

  TCCR0B |= (1 << CS01);
  TCCR0B |= (1 << CS00);
}

uint32_t timer = 0;

ISR(TIMER0_COMPA_vect) {
  timer += 1;
}

void showDisplay(byte ledMatrix[8]) {
  for (int i = 0; i < 8; i++)
  {
    lc.setRow(0, i, ledMatrix[i]);
  }
}

void play() {
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      if (timeMatrix[i][j] != 0) {
        tone(TONE_PIN, freqMatrix[i][j]);
        delay(timeMatrix[i][j]);
      }
    }
  }
  noTone(TONE_PIN);
  Serial.println("NoTone");
}

void handleButtons(int b) {
  int col = int(floor(count/8));
  int row = 7-count%8;
  if (b == NEXT_PIN) {
    selected = 0;
    count += 1;
    showDisplay(ledMatrix);
  }
  if (b == PREV_PIN) {
    selected = 0;
    count -= 1;
    showDisplay(ledMatrix);
  }
  if (b == SAVE_PIN) {
    select();
    save(row, col);
    numToLed(numMatrix, ledMatrix);
    showDisplay(ledMatrix);
  }
}

float calTotalTime() {
  int totalTime = 0;
    for (int i=0; i<8; i++) {
      for (int j=0; j<8; j++) {
        totalTime += timeMatrix[i][j];
      }
    }
  return totalTime;
}

void display(int row, int col, int noteMatrix[8][8], int timeMatrix[8][8]) {
  lcd.setCursor(0,0);
  lcd.print(noteNames[noteMatrix[row][col]]);
  lcd.setCursor(13,0);
  lcd.print(timeMatrix[row][col]);
  
  int totalTime = calTotalTime();
  

 lcd.setCursor(0,1);
 lcd.print((timer/60/1000)%10);
 lcd.print(":");
 lcd.print(timer/1000/10%6);
 lcd.print(timer/1000%10);
 lcd.print(":");
 lcd.print(timer/10%100);
 lcd.setCursor(10,1);
 lcd.print((totalTime/60/1000)%10);
 lcd.print(":");
 lcd.print(totalTime/1000/10%6);
 lcd.print(totalTime/1000%10);
 lcd.print(":");
 lcd.print(totalTime/10%100);
}

void debounce() {
  uint8_t i;
  uint32_t currentMillis = timer;
  for (i = 0; i < noOfButtons; ++i) {
    if (digitalRead(buttonPins[i])) {             
        previousMillis[i] = currentMillis;        
        pressCount[i] = 0;                        
      } else {
      if (currentMillis - previousMillis[i] > bounceDelay) {
        previousMillis[i] = currentMillis;        
        ++pressCount[i];
        if (pressCount[i] == minButtonPress) {
          handleButtons(buttonPins[i]); 
          Serial.println(buttonPins[i]);                          
        }
      }
    }
  }
}

void loop() {
  lcd.setCursor(0, 1);
  cli();
  debounce();
  sei();
  int col = int(floor(count/8));
  int row = 7-count%8;
  lc.setLed(0, row, col, true);

  display(row, col, noteMatrix, timeMatrix);

  if (selected == 1) {
    freqMatrix[row][col] = mapNotes(analogRead(A0));
    noteMatrix[row][col] = mapNotesName(analogRead(A0));
    timeMatrix[row][col] = floor(map(analogRead(A1), 0, 1023, 19, 2))/20*1000;
  }
    
  if (freqMatrix[row][col] != 0 && playing == 0)
  {
    tone(TONE_PIN, freqMatrix[row][col], timeMatrix[row][col]);
    Serial.println(freqMatrix[row][col]);
    Serial.println(timeMatrix[row][col]);
  }

  if (playing == 1)
  {
    play();
  }
}

