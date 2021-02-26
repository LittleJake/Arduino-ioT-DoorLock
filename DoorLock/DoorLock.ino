#include <LiquidCrystal_I2C.h>
#include <Wire.h>

const String PASSWORD = "666666";
const String NFC_UID = "";
const int numRows = 4, numCols = 4, debounceTime = 5;

const char keymap[numRows][numCols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

const int rowPins[numRows] = {9,8,7,6};
const int colPins[numCols] = {5,4,3,2};
const int lockPin = 10;
String pass = "";

LiquidCrystal_I2C lcd(0x27,16,2);

void setup() {
  //初始化
  Serial.begin(9600);
  pinMode(10,OUTPUT);
  for(int row = 0; row< numRows; row++)
  {
    pinMode(rowPins[row], INPUT);
    digitalWrite(rowPins[row], HIGH);
  }
  
    for(int col = 0; col< numCols; col++)
  {
    pinMode(colPins[col], OUTPUT);
    digitalWrite(colPins[col], HIGH);
  }

  lcd.init(); // 初始化LCD
  lcd.backlight(); //设置LCD背景等亮
  lcd.setCursor(0,0); //设置显示指针
  lcd.print("Initializing..."); //输出字符到LCD1602上
  delay(1000);
  lcd.clear();
}

//循环
void loop() {
  // put your main code here, to run repeatedly:
  char key = getKey();
  //TODO 获取指纹
  //TODO 获取NFC
  //TODO 获取蓝牙指令
  //TODO 获取TOTP


  
  if(key){
    Serial.print("get key:");
    Serial.println(key);
    pass = pass + key;
    
    if(pass == PASSWORD){
      lcd.setCursor(0,1);
      lcd.print("Good day");
      pass = "";
      delay(2000);
      lcd.clear();
     }

    if(pass.length() == 4){
      lcd.setCursor(0,1);
      lcd.print("Wrong Password");
      pass = "";
      delay(2000);
      lcd.clear();
     }
  }

  lcd.setCursor(0,0);
  lcd.print("Enter Password");
  lcd.setCursor(0,1);
  lcd.print(pass);
}

//获取按键
char getKey(){
  char key = 0;
  
  for(int col = 0; col < numCols; col++){
    digitalWrite(colPins[col], LOW);
    
    for(int row = 0; row<numRows; row++){
      if(digitalRead(rowPins[row]) == LOW){
        delay(debounceTime);
        tone(10, ((row-1) * 4+col)* 200 + 2000);
        while(digitalRead(rowPins[row]) == LOW);
        noTone(10);
        key = keymap[row][col];
      }
    }
    digitalWrite(colPins[col], HIGH);
  }
  return key;
}

char getFingerprint(){}

char getNFC(){}

char getBluetooth(){}

char getTOTP(){}

//开锁
void unlock(){
  digitalWrite(lockPin, HIGH);
  delay(3000);
  digitalWrite(lockPin, LOW);
}
