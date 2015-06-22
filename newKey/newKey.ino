/**
 * ----------------------------------------------------------------------------
 * This is a MFRC522 library example; see https://github.com/miguelbalboa/rfid
 * for further details and other examples.
 * 
 * NOTE: The library file MFRC522.h has a lot of useful info. Please read it.
 * 
 * Released into the public domain.
 * ----------------------------------------------------------------------------
 * This sample shows how to read and write data blocks on a MIFARE Classic PICC
 * (= card/tag).
 * 
 * BEWARE: Data will be written to the PICC, in sector #1 (blocks #4 to #7).
 * 
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno           Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 * 
 */

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;
MFRC522::MIFARE_Key newKey;



// BEGIN USER CONFIG

The following values must match in this sketch and in the Bike Alarm sketch for authentication to succeed!

// CHANGE THESE TO YOUR OWN UNIQUE VALUES!!!
//      You can get randomly generated bytes here- https://www.random.org/bytes/
// Note - only using 6 bytes of these buffers.. 

uint8_t newA[8] = { 0x3b, 0x32, 0xa8, 0xc3, 0x3f, 0xf5 };
uint8_t newB[8] = { 0xb5, 0x0e, 0xc9, 0x2d, 0x4a, 0xe6 }; //not yet used...


//this is the token block (i.e. THIS IS YOUR KEY!!!) Change it to something unique 
//      You can get randomly generated bytes here- https://www.random.org/bytes/
byte tokenBlockData[16]    = {
        0x13, 0x52, 0xf2, 0x5b,
	0xc8, 0x8c, 0x2d, 0xb0,
	0x7a, 0xe7, 0xcd, 0x86,
	0xbc, 0xe9, 0xf2, 0x0f
};

//key sector/blocks to use. (Don't use sector 0 as it holds the UID ) 
byte sector = 1;

//END USER CONFIG



byte blockAddr      = sector * 4; //calculate the first block in the sector  
byte trailerBlock   = blockAddr + 3; //calculate the last(trailer) block in the sector(this block has the keys)

byte keyUID[4] = { 0,0,0,0 }; //store the key UID
// Key Block pre-populated with access bytes (FF, 07, 80, 69) keys go before and after.
// Note - we only use  16 bytes... 
uint8_t newKeyBlock[18] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x07, 0x80, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/**
 * Initialize.
 */
void setup() {
    Serial.begin(9600); // Initialize serial communications with the PC
    while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
    SPI.begin();        // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522 card

    // Prepare the key (used both as key A and as key B)
    // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = 0xFF;
        newKey.keyByte[i] = newA[i];

        //Prepare the key block with our new key
        byte j = i + 10;
        newKeyBlock[i] = newA[i];
        newKeyBlock[j] = newB[i];
    }
    Serial.print(F("Finished Generating Key Block: "));
    dump_byte_array(newKeyBlock, 16);
    Serial.println();


    Serial.println(F("Scan a MIFARE Classic PICC to demonstrate read and write."));
    Serial.print(F("Using key (for A and B):"));
    dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);
    Serial.println();
    
    Serial.print(F("BEWARE: Data will be written to the PICC, in sector # "));
    Serial.println(sector);
}

/**
 * Main loop.
 */
void loop() {
    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent())
        return;

    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
        return;

    // Show some details of the PICC (that is: the tag/card)
    Serial.print(F("Card UID:"));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    for(byte k=0;k<4;k++)
    {
      keyUID[k] = mfrc522.uid.uidByte[k];
    }
    Serial.println();
    Serial.print(F("PICC type: "));
    byte piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    Serial.println(mfrc522.PICC_GetTypeName(piccType));

    // Check for compatibility
    if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("This sample only works with MIFARE Classic cards."));
        return;
    }

    byte status;
    byte buffer[18];
    byte size = sizeof(buffer);

    // Authenticate using key A
    Serial.println(F("Authenticating using key A..."));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        Serial.println(F("Key might already be provisioned..."));
        while (!Serial.available()); //wait for key press.
    }
    else
    {
      //success with factory key, write new key
      Serial.read();
      Serial.println(F("PICC detected... Press any key to provision"));
      while (!Serial.available()); //wait for key press.
      // Write data to the trsiler block
      Serial.print(F("Writing data into block ")); Serial.print(trailerBlock);
      Serial.println(F(" ..."));
      dump_byte_array(newKeyBlock, 16); Serial.println();
      status = mfrc522.MIFARE_Write(trailerBlock, newKeyBlock, 16);
      if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Write() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
      }
      Serial.println();
      // Write data to the block
      Serial.print(F("Writing data into block ")); Serial.print(blockAddr);
      Serial.println(F(" ..."));
      dump_byte_array(tokenBlockData, 16); Serial.println();
      status = mfrc522.MIFARE_Write(blockAddr, tokenBlockData, 16);
      if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Write() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
      }
      Serial.println();  
    }

    //Success!
    // Authenticate using new Key
    Serial.println(F("Authenticating using new Key..."));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &newKey, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        Serial.println(F("Key Change may be pooched!!!"));
        return;
    }    

    // Read data from the block
    Serial.print(F("Reading data from block ")); Serial.print(blockAddr);
    Serial.println(F(" ..."));
    status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    
    Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
    dump_byte_array(buffer, 16); Serial.println();
    Serial.println();

    // Read data from the block (again, should now be what we have written)
    Serial.print(F("Reading data from block ")); Serial.print(blockAddr);
    Serial.println(F(" ..."));
    status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
    dump_byte_array(buffer, 16); Serial.println();
        
    // Check that data in block is what we have written
    // by counting the number of bytes that are equal
    Serial.println(F("Checking result..."));
    byte count = 0;
    for (byte i = 0; i < 16; i++) {
        // Compare buffer (= what we've read) with dataBlock (= what we've written)
        if (buffer[i] == tokenBlockData[i])
            count++;
    }
    Serial.print(F("Number of bytes that match = ")); Serial.println(count);
    if (count == 16) {
        Serial.println();
        Serial.println(F("SUCCESS! Key provisioned. :-)"));
        Serial.println();
        Serial.println(F("You may now upload the Bike-Alarm Sketch."));
        Serial.println(F("Make sure to add your token block data and keys to that sketch..."));
        Serial.println();
        Serial.print(F("    Also you MUST add your card UID: '"));
        dump_byte_array(keyUID, 4);
        Serial.println(F("' to the keyUIDs array!!!"));
        
        Serial.read(); //clear input
        while (!Serial.available()); //wait for key press.
    } else {
        Serial.println(F("Failure, no match :-("));
        Serial.println(F("  perhaps the write didn't work properly..."));
    }
    Serial.println();
        
    // Dump the sector data
    Serial.println(F("Current data in sector:"));
    mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
    Serial.println();

    // Halt PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
    Serial.read();
    Serial.println(F("Finished! Press any key to exit..."));
    while (!Serial.available()); //wait for key press.
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}
