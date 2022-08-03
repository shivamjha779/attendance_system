#include<ESP8266WiFi.h>
#include<ESP8266HTTPClient.h>
#include<WiFiClient.h>
#include <SPI.h>
#include <MFRC522.h>
#define ssid "Galaxy"
#define password "shivamjha"
#define RST_PIN         D3        // Configurable, see typical pin layout above
#define SS_PIN          D4         // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
const char*host="https://maker.ifttt.com/";

//*****************************************************************************************//

WiFiClient client;
void setup(){
  Serial.begin(9600);
  Serial.println("Connecting to");
  Serial.print(ssid); 
  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.print(". ");
  }
  Serial.println();
  Serial.println("Connected to:");
  Serial.println(ssid);
  pinMode(D2,OUTPUT);
  digitalWrite(D2,HIGH);
  delay(2000);
  digitalWrite(D2,LOW);
  
  //connecting to the RFID module using SPI communication...........
  SPI.begin();                                                  // Init SPI bus
  mfrc522.PCD_Init();                                           // Init MFRC522 card
  Serial.println(F("Read personal data on a MIFARE PICC:"));    //shows in serial that it is ready to read

  }
void loop(){
  
  
  // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

  //-------------------------------------------

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println(F("**Card Detected:**"));
  tone(D2,350);
  //-------------------------------------------

  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); //dump some details about the card

  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));      //uncomment this to see all blocks in hex

  //-------------------------------------------

  Serial.print(F("Name: "));

  byte buffer1[18];

  block = 4;
  len = 18;

  //------------------------------------------- GET FIRST NAME
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
String first_name="",last_name="",full_name="";
  
  //PRINT FIRST NAME
  for (uint8_t i = 1; i < 16; i++)
  {
    if(char(buffer1[i])==' '){
      break;
    }
      first_name+=char(buffer1[i]);
  }

  //---------------------------------------- GET LAST NAME

  byte buffer2[18];
  block = 1;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  int start=0;

  //PRINT LAST NAME
  if(char(buffer2[0])=='\n'){
    start=1;
  }
  for (uint8_t i = start; i < 16; i++) {
    if(char(buffer2[i])==' '){
      break;
    }
    last_name+=char(buffer2[i]);
  }
  //fullname=firstname+lastname
  full_name=String(first_name)+String(last_name);
  Serial.println(full_name);
  Serial.println(F("\n**End Reading**\n"));
  delay(1000); //change value if you want to read cards faster
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  noTone(D2);

    //sending data to ifttt;
  
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();
  String Link,getData;
  getData="trigger/student_entered/with/key/b-AXeVs3f38Q5Q5mlbpcCsm2JqHRz8zLcgJes9WYfWW/?value1="+String(first_name)+"&value2="+String(last_name);
  Link=host+getData;
  http.begin(client,Link);
  http.GET();
  String payload=http.getString();
  Serial.println(payload);
  http.end();
  Serial.print(Link);
  }
