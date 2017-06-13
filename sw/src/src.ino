/*
 * Typical pin layout used:
 * --------------------------------------------------- 
 *             MFRC522      Arduino       NodeMCU
 *             Reader/PCD   Uno/101       ESP8266 12E
 * Signal      Pin          Pin           Pin
 * ---------------------------------------------------
 * RST/Reset   RST          9             D2
 * SPI SS      SDA(SS)      10            D1
 * SPI MOSI    MOSI         11 / ICSP-4   D6
 * SPI MISO    MISO         12 / ICSP-1   D7
 * SPI SCK     SCK          13 / ICSP-3   D5
 */

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         5
#define SS_PIN          4
String SERVER ="192.168.0.122";				//Change IP

MFRC522 mfrc522(SS_PIN, RST_PIN);

MFRC522::MIFARE_Key key;

ESP8266WiFiMulti WiFiMulti;

void setup() {
  WiFiMulti.addAP("tenet_sales", "TeNeT2007Dlink"); 		//Change WiFi Creds

  Serial.begin(115200); 
  while (!Serial);
  SPI.begin();
  mfrc522.PCD_Init(); 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}

void loop() {
  if((WiFiMulti.run() != WL_CONNECTED))
    return;
  
  if ( ! mfrc522.PICC_IsNewCardPresent())
    return;
    
  if ( ! mfrc522.PICC_ReadCardSerial())
    return;
    

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);

  if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
          &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
          &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    return;
  }
  
  byte sector         = 1;
  byte blockAddr      = 4;
  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);

  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAddr, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    return;
  }

  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
  }
  
  HTTPClient http;

  http.begin("http://" + SERVER + "/rfid/add.php?id=" + buf2Str(buffer, 16)); 
  
  int httpCode = http.GET();

  if(httpCode > 0) {
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      if(httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          Serial.println(payload);
      }
  } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
  
  mfrc522.PICC_HaltA();

  mfrc522.PCD_StopCrypto1();
}

String buf2Str(byte *buffer, byte bufferSize) {
  String s;
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print((char)buffer[i]);
    s += String((char)buffer[i]);
  }
  return s;
}

