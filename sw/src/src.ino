/*
 * Typical pin layout used:
 * ----------------------------------------
 *             MFRC522      Arduino      
 *             Reader/PCD   Uno/101      
 * Signal      Pin          Pin           
 * ----------------------------------------
 * RST/Reset   RST          9          
 * 
 * SPI SS      SDA(SS)      10         
 * SPI MOSI    MOSI         11 / ICSP-4
 * SPI MISO    MISO         12 / ICSP-1   
 * SPI SCK     SCK          13 / ICSP-3
 */
  
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9
#define SS_PIN          10

MFRC522 mfrc522(SS_PIN, RST_PIN);

MFRC522::MIFARE_Key key;

void setup() {
  Serial.begin(9600); 
  while (!Serial);    
  SPI.begin();        
  mfrc522.PCD_Init(); 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}

void loop() {
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
    return;
  }
  //Serial.print(F("Data in the card: "));
  dump_data(buffer, 16); Serial.println();
  Serial.println();

  mfrc522.PICC_HaltA();

  mfrc522.PCD_StopCrypto1();
}

void dump_data(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " " : "");
    Serial.write(buffer[i]);
  }
}
