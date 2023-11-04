#include <Arduino.h>
#include <LedControl.h>

LedControl lc=LedControl(12,10,11,1);

#define NEXT_PIN 4
#define PREV_PIN 5
#define SAVE_PIN 2
#define TONE_PIN 9
#define PLAY_PIN 3

byte last_nextBtn, last_prevBtn = HIGH;

unsigned long debounce = 50;
unsigned long lastTimeNext = 0;
unsigned long lastTimePrev = 0;
int count = 0;
int selected = 0;
int playing = 0;

int numMatrix[8][8] = {
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
};

float freqMatrix[8][8] = {
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
};

float timeMatrix[8][8] = {
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
};

byte ledMatrix[8];

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
  Serial.println("selected"); 
}

void playStart() {
  playing =  1;
  Serial.println("started");
}

void setup() {
  // put your setup code here, to run once:1
  lc.shutdown(0, false);
  lc.setIntensity(0, 10);
  lc.clearDisplay(0);

  pinMode(PREV_PIN, INPUT_PULLUP);
  pinMode(NEXT_PIN, INPUT_PULLUP);
  pinMode(SAVE_PIN, INPUT_PULLUP);
  pinMode(PLAY_PIN, INPUT_PULLUP);
  pinMode(TONE_PIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(SAVE_PIN), select, FALLING);
  attachInterrupt(digitalPinToInterrupt(PLAY_PIN), playStart, FALLING);

  Serial.begin(115200);
  numToLed(numMatrix, ledMatrix);
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
  playing = 0;
  noTone(TONE_PIN);
}

void loop() {
  // put your main code here, to run repeatedly:
    int col = int(floor(count/8));
    int row = 7-count%8;
    lc.setLed(0, row, col, true);
    if (millis() - lastTimeNext > debounce) {
      byte nextBtn = digitalRead(NEXT_PIN);
      if (nextBtn != last_nextBtn) {
        lastTimeNext = millis();
        last_nextBtn = nextBtn;
        if (nextBtn == LOW) {
          selected = 0;
          count += 1;
          showDisplay(ledMatrix);
        }
      }
    }
    
    if (millis() - lastTimePrev > debounce) {
      byte prevBtn = digitalRead(PREV_PIN);
      if (prevBtn != last_prevBtn) {
        lastTimePrev = millis();
        last_prevBtn = prevBtn;
        if (prevBtn == LOW) {
          selected = 0;
          count -= 1;
          showDisplay(ledMatrix);
        }
      }
    }

    if (digitalRead(SAVE_PIN) == LOW) {
      save(row,col);
      numToLed(numMatrix, ledMatrix);
      showDisplay(ledMatrix);
      Serial.println(selected); 
    }

    if (selected == 1) {
      freqMatrix[row][col] = map(analogRead(A0), 0, 1023, 2000, 50);
      timeMatrix[row][col] = map(analogRead(A1), 0, 1023, 1000, 10);
    }
    
    if (freqMatrix[row][col] != 0 && playing == 0)
    {
      tone(TONE_PIN, freqMatrix[row][col], timeMatrix[row][col]);
    }

    if (playing == 1)
    {
      play();
    }
}

