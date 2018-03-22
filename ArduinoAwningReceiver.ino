#include "RFControl.h"

const int RELAY_PIN_1 = 10;
const int RELAY_PIN_2 = 11;
const int RECEIVER_PIN = 3;
const int RECEIVER_INTERRUPT_PIN = digitalPinToInterrupt(RECEIVER_PIN);


void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);
  pinMode(RECEIVER_PIN, INPUT);
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  Serial.print("Hello 433\n");

  RFControl::startReceiving(RECEIVER_INTERRUPT_PIN);
}

bool in_raw_mode = false;

#if 0
const char s00[] PROGMEM = "001011001010101010110010101010101011010011001011001010101010101102";  // remote 0, 1
const char s01[] PROGMEM = "001010101011010101001100110100101011001100110100101010101010101102";  // remote 2
const char s02[] PROGMEM = "001010101011010101001101001010110010110010110101001010101010101102";  // remote 3

const char *const signatures [] PROGMEM = {
  s00, s01, s02
};
#endif 

const char s00[] PROGMEM = "01100110010101100110010101100101010101100110011002";

const char s10[] PROGMEM = "01100110010101100110010101100101010101100110010102";

const char *const signatures [] PROGMEM = {
  s00, s10
};

const int relays [] = {
  RELAY_PIN_1, RELAY_PIN_2
};

int num_sigs = sizeof(signatures) / sizeof(const char *);

void toggle_relay(int pin) {
  Serial.print(pin);
  Serial.println(" on");
  digitalWrite(pin, HIGH);
  delay(500);                     // press relay for 100ms
  Serial.print(pin);
  Serial.println(" off");
  digitalWrite(pin, LOW);
  // delay(500);                    // cooldown of 1 seconds  
}


void loop() {
  if(RFControl::hasData()) {
    unsigned int *timings;
    unsigned int timings_size;
    RFControl::getRaw(&timings, &timings_size);
    unsigned int buckets[8];
    buckets[0] = 200;
    buckets[1] = 400;
    buckets[2] = 6000;
    RFControl::compressTimings(buckets, timings, timings_size);
    int found = -1;
    String signature = "";
    for(unsigned int i=0; i < timings_size; i++) {
      char c = '0' + timings[i];
      signature += c;
    }
    for (int s = 0; s < num_sigs; s++) {
      static char buffer[128];
      strcpy_P(buffer, (char*)pgm_read_word(&(signatures[s])));
      String check(buffer);
      if (signature == check) {
        found = s;
      }
    }
    for (int t = 0; t < 8; t++) {
      Serial.print(buckets[t]);
      Serial.print(" ");
    }
    if (found != -1) {
      Serial.print(signature);
      Serial.print(" ");
      Serial.println(found);
      toggle_relay(relays[found]);
    }
    else {
      Serial.print(signature);
      Serial.println(" not found");
    }
    RFControl::continueReceiving();
  }
}
