#include <EEPROM.h>
const char* NODE_ID = "node2"; //MAX 8

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("boot");

  eepromBurn(NODE_ID, 0); 

  Serial.println();
  Serial.println(eepromReed(0, 8));

  //WiFi.begin(EEPROM_ESP8266_LEER(0,32).c_str(), EEPROM_ESP8266_LEER(32,64).c_str());

}
//
void loop() {}
//
void eepromBurn(String buffer, int n) {
  EEPROM.begin(8); 
  delay(10);
  for (int i = 0; i < 8; ++i) {
    EEPROM.write(n + i, buffer[i]);
  }
  EEPROM.commit();
}
//
String eepromReed(int min, int max) {
  EEPROM.begin(8); 
  delay(10); 
  String buffer;
  for (int i = min; i < max; ++i)
    if (isAlphaNumeric(EEPROM.read(i)))
      buffer += char(EEPROM.read(i));
  return buffer;
}
