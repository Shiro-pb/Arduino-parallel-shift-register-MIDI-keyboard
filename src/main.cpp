#include <Arduino.h>
//#include <SPI.h>

#define LOAD_PIN PA8
#define DATA_IN PB14
#define CLK_PIN PB13

#define rangeKnob A0

uint32_t keyboardWordRaw = 0;
uint32_t keyboardWordLower3 = 0;
uint32_t keyboardWordUpper1 = 0;
uint32_t keyboardWordLower3Old = 0;
uint32_t keyboardWordLower3XorMask = 0;

int rangeValue = 0;

void sendMIDI(uint8_t statusByte, uint8_t dataByte1, uint8_t dataByte2){
  Serial.write(statusByte);
  Serial.write(dataByte1);
  Serial.write(dataByte2);
} 

void sendMIDInoteON(uint8_t note){
   sendMIDI(0x90, note, 0x00);
}

void sendMIDInoteOff(uint8_t note){
  sendMIDI(0x80, note, 0x00);
}

uint32_t readKeyboard(){// 4 bytes (4 * FF)
  uint32_t inputWord = 0;

  digitalWrite(LOAD_PIN, LOW);
  //delay??
  digitalWrite(LOAD_PIN, HIGH);

  //shift seems to happen on a positive edge
  for(int i = 0 ; i < 31 ; i++){// maybe 32
    digitalWrite(CLK_PIN, HIGH);
    inputWord += (digitalRead(DATA_IN) << i);//LSB comes first
    digitalWrite(CLK_PIN, LOW);
  }

  return inputWord;
}

void mainKeyboardRoutine(){
  keyboardWordRaw = readKeyboard();
  keyboardWordLower3 = keyboardWordRaw and 0x00FFFFFF;//first 3 bytes
  keyboardWordLower3XorMask = keyboardWordLower3Old xor keyboardWordRaw;

  for(int j = 0 ; j < 23 ; j++){
    if((keyboardWordLower3XorMask >> j) & 0b01){
      if((keyboardWordRaw >> j) & 0b01){
        sendMIDInoteON(j + (24 * rangeValue));
      } else {
        sendMIDInoteOff(j + (24 * rangeValue));
      }
    }
  }

  keyboardWordLower3Old = keyboardWordRaw;
}

void setup() {
  Serial.begin(31250);
  pinMode(LOAD_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(DATA_IN, INPUT);
  pinMode(rangeKnob, INPUT_ANALOG);
  digitalWrite(LOAD_PIN, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);//may be low
}

void loop() {
  rangeValue = map(analogRead(rangeKnob), 0, 1023, 0, 5);
  mainKeyboardRoutine();
}