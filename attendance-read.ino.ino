#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Define constants and variables
const char* serverName = "https://script.google.com/macros/s/AKfycbzNunL6_374wWjvLtPFxcX_y5obo85sdWZ_tp1IVtavp4HZcS4oKwaaUcsmLvc5tnsI/exec"; // Replace with your deployed Google Apps Script URL
const char* ssid = "vivo1234";  // Replace with your network SSID
const char* password = "12345678"; // Replace with your network password

// Define GPIO pins for ESP32
#define RST_PIN  22  // RST pin for ESP32
#define SS_PIN   5   // SS pin for ESP32
#define BUZZER_PIN 2  // Buzzer pin for ESP32

// Create instances for RFID
MFRC522 rfid(SS_PIN, RST_PIN); 
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;

// Define block data variables
byte bufferLen = 18;
byte readBlockData[18];

void setup() {
    Serial.begin(115200);
    SPI.begin(); // Init SPI bus
    rfid.PCD_Init(); // Init MFRC522 module

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Initialize Buzzer
    pinMode(BUZZER_PIN, OUTPUT); // Set buzzer pin as output
}

void loop() {
    // Check for a new card
    if (!rfid.PICC_IsNewCardPresent()) return;
    if (!rfid.PICC_ReadCardSerial()) return;

    Serial.println(F("Reading RFID data block 4..."));
    
    // Read data from block 4
    ReadDataFromBlock(4, readBlockData);
    
    // Convert read data to String
    String blockData = String((char*)readBlockData);
    blockData.trim(); // Trim any whitespace
    Serial.println("Read Block Data: " + blockData); // Debug print

    // Construct the URL with block data, ensuring proper URL encoding
    String fullURL = String(serverName) + "?name=" + urlencode(blockData);
    Serial.println(fullURL); // Print URL for debugging

    // Send data to Google Sheets
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(fullURL); // Make GET request to Google Sheets

        int httpResponseCode = http.GET();
        if (httpResponseCode > 0) {
            Serial.printf("[HTTPS] GET... code: %d\n", httpResponseCode);
            // Activate buzzer for successful read
            activateBuzzer();
        } else {
            Serial.printf("[HTTPS] GET... failed, error: %s\n", http.errorToString(httpResponseCode).c_str());
        }

        http.end();
        delay(1000);
    }

    rfid.PICC_HaltA(); // Halt RFID card
    rfid.PCD_StopCrypto1(); // Stop encryption on card
}

/****************************************************************************************************
 * ReadDataFromBlock() function
 * Reads data from a specific block of the RFID tag.
 ****************************************************************************************************/
void ReadDataFromBlock(int blockNum, byte readBlockData[]) {
    // Use key A for authentication (default key for most cards)
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
    }

    // Authenticate using key A
    status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(rfid.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print("Authentication failed for Read: ");
        Serial.println(rfid.GetStatusCodeName(status));
        return;
    }

    // Read block data
    status = rfid.MIFARE_Read(blockNum, readBlockData, &bufferLen);
    if (status != MFRC522::STATUS_OK) {
        Serial.print("Reading failed: ");
        Serial.println(rfid.GetStatusCodeName(status));
        return;
    }

    Serial.println("Data read successfully from block");
}

// Helper function to URL encode the block data
String urlencode(String str) {
    String encoded = "";
    char c;
    for (int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
        } else {
            encoded += '%' + String(c, HEX);
        }
    }
    return encoded;
}

// Function to activate the buzzer
void activateBuzzer() {
    digitalWrite(BUZZER_PIN, HIGH); // Turn on the buzzer
    delay(500); // Buzzer ON for 500ms
    digitalWrite(BUZZER_PIN, LOW); // Turn off the buzzer
}