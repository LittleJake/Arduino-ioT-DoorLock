/*
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>

#define SS_PIN 53 //RFID SDA
#define RST_PIN 5 //RFID RESET PIN for mega
#define RELAY_PIN 22 //继电器PIN
#define SET_RFID_PIN A0 //设置RFID卡
#define BEEP_PIN 23 //蜂鸣器
//函数
char getKey();
void unlock();
void rfidRead();
void printHex(byte *buffer, byte bufferSize);
void printDec(byte *buffer, byte bufferSize);
bool isEqualUid(byte *hex1, byte *hex2);

MFRC522 rfid(SS_PIN, RST_PIN); // RFID CLASS
MFRC522::MIFARE_Key key; 
const String PASSWORD = "666666";
const int numRows = 4, numCols = 4, debounceTime = 5;
const char keymap[numRows][numCols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

const int rowPins[numRows] = {13,12,11,10};
const int colPins[numCols] = {9,8,7,6};
// 存放UID
byte nuidPICC[4];

String text = "";
LiquidCrystal_I2C lcd(0x27,16,2);

void setup() {
  Serial.begin(9600);
  //初始化NFC
  SPI.begin(); // Init SPI bus
  pinMode(SET_RFID_PIN,INPUT);
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) { key.keyByte[i] = 0xFF; }

  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  
  //初始化键盘
  pinMode(BEEP_PIN,OUTPUT);
  for(int row = 0; row< numRows; row++) {
    pinMode(rowPins[row], INPUT);
    digitalWrite(rowPins[row], HIGH);
  }
  
  for(int col = 0; col< numCols; col++) {
    pinMode(colPins[col], OUTPUT);
    digitalWrite(colPins[col], HIGH);
  }
  // 初始化LCD
  lcd.init();
  lcd.backlight(); //设置LCD背景等亮
  lcd.setCursor(0,0); //设置显示指针
  lcd.print("Initializing..."); //输出字符到LCD1602上
  delay(1000);
  lcd.clear();
}

//循环
void loop() {
  // put your main code here, to run repeatedly:
  //获取NFC
  rfidRead();
  //获取蓝牙
  char key = getKey();
  //TODO 获取指纹
  //TODO 获取NFC
  //TODO 获取蓝牙指令
  //TODO 获取TOTP

  if(key){
    Serial.print("get key:");
    Serial.println(key);
    text = text + key;
    
    if(text.equals(getTOTP())){
      lcd.setCursor(0,1);
      lcd.print("Good day");
      text = "";
      delay(2000);
      lcd.clear();
     }

    if(text.length() == 8){
      lcd.setCursor(0,1);
      lcd.print("Wrong Password");
      text = "";
      delay(2000);
      lcd.clear();
     }
  }

  lcd.setCursor(0,0);
  lcd.print("Enter Password");
  lcd.setCursor(0,1);
  lcd.print(text);
}

//获取按键
char getKey(){
  char key = 0;
  
  for(int col = 0; col < numCols; col++){
    digitalWrite(colPins[col], LOW);
    
    for(int row = 0; row<numRows; row++){
      if(digitalRead(rowPins[row]) == LOW){
        delay(debounceTime);
        tone(BEEP_PIN, ((row-1) * 4+col)* 200 + 2000);
        while(digitalRead(rowPins[row]) == LOW);
        noTone(BEEP_PIN);
        key = keymap[row][col];
      }
    }
    digitalWrite(colPins[col], HIGH);
  }
  return key;
}

char getFingerprint(){

  if(match){
    unlock();
  }
}

char getBluetooth(){
  //蓝牙混杂模式接收数据？
  if (1){
    unlock();
  }

}

String getTOTP(){}

//开锁
void unlock(){
  digitalWrite(RELAY_PIN, HIGH);
  delay(3000);
  digitalWrite(RELAY_PIN, LOW);
}

void rfidRead(){
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  // Verify if the NUID has been readed
  if ( !(rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()))
    return;

  tone(BEEP_PIN, 2000);
  //获取卡类型
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // 检查卡类型
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    // return;
  }

  if (!isEqualUid(rfid.uid.uidByte, nuidPICC)) {
    Serial.println(F("A new card has been detected."));

    // 保存UID
    //if (digitalRead(SET_RFID_PIN) == HIGH)
    if (analogRead(SET_RFID_PIN) > 1000)
      for (byte i = 0; i < 4; i++) 
        nuidPICC[i] = rfid.uid.uidByte[i];
    
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  } else {
    Serial.println(F("Card read previously."));
    Serial.println(F("Triger the relay."));
    unlock();
  } 
  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
  noTone(BEEP_PIN);
}

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

bool isEqualUid(byte *hex1, byte *hex2){
  return (hex1[0] == hex2[0] && hex1[1] == hex2[1] &&
    hex1[2] == hex2[2] && hex1[3] == hex2[3]);
}
