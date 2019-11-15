#include <dht11.h>
#define DHT11_PIN 2
#define RELAY_PIN 3

dht11 DHT;
 
void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
}
  
void loop(){
  
  if(Serial.available() > 0) {
    switch(Serial.read()) {
      case 'r':
        DHT.read(DHT11_PIN);
        Serial.write((uint8_t*)&DHT.temperature, sizeof(DHT.temperature));
        Serial.write((uint8_t*)&DHT.humidity, sizeof(DHT.humidity));
        break;

      case 'h':
        digitalWrite(RELAY_PIN, HIGH);
        break;

      case 'l':
        digitalWrite(RELAY_PIN, LOW);
        break;
    }  
  }
}
