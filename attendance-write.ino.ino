#include <SPI.h>
#include <MFRC522.h>

//--------------------------------------------------
// Update pin assignments for ESP32
const uint8_t RST_PIN = 22;  // Reset pin (can be any GPIO pin)
const uint8_t SS_PIN = 5;    // SDA pin (can be any GPIO pin)
//--------------------------------------------------
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

// Block number to write data into
int blockNum = 4;  // You can change this to any other block as needed
byte block_data[16];
byte bufferLen = 18;
byte readBlockData[18];

//--------------------------------------------------
MFRC522::StatusCode status;

void setup() 
{
  // Initialize serial communications with PC
  Serial.begin(115200);
  
  // Initialize SPI bus for ESP32
  SPI.begin();  // Default pins for VSPI: SCK=18, MISO=19, MOSI=23, SS=21
  
  // Initialize MFRC522 Module
  mfrc522.PCD_Init();
  Serial.println("Scan a MIFARE 1K Tag to write data...");
}

void loop() 
{
  // Prepare the key for authentication
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println("*Card Detected*");
  Serial.print(F("Card UID:"));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.print("\n");

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  // Write name to block (for example, "Kamrudhin N")
  toBlockDataArray("Aayishaa");
  WriteDataToBlock(blockNum, block_data);

  // Read the name from the block and print it to Serial
  ReadDataFromBlock(blockNum, readBlockData);
  dumpSerial(blockNum, readBlockData);

  // Halt PICC (Tag) for future scanning
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();  // Stop encryption on card
}

void WriteDataToBlock(int blockNum, byte blockData[]) 
{
  Serial.print("Writing data to block ");
  Serial.println(blockNum);

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed for Write: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Write(blockNum, blockData, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Writing to Block failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  Serial.println("Data written successfully to block");
}

void ReadDataFromBlock(int blockNum, byte readBlockData[]) 
{
  Serial.print("Reading data from block ");
  Serial.println(blockNum);

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed for Read: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  Serial.println("Data read successfully from block");
}

void dumpSerial(int blockNum, byte blockData[]) 
{
  Serial.print("Data in Block ");
  Serial.print(blockNum);
  Serial.print(": ");
  for (int j = 0; j < 16; j++) {
    Serial.write(blockData[j]);
  }
  Serial.println();
}

void toBlockDataArray(String str) 
{
  byte len = str.length();
  if (len > 16) len = 16;  // Trim the string if it's longer than 16 bytes
  for (byte i = 0; i < len; i++) block_data[i] = str[i];
  for (byte i = len; i < 16; i++) block_data[i] = ' ';  // Fill remaining with spaces
}
