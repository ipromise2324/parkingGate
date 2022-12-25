#include <Keypad.h>
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {23, 19, 18, 5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 26, 27, 14}; //connect to the column pinouts of the keypad
//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

//OLED顯示器///////////////////////////////////////
#include <Arduino.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display

#include <Adafruit_Fingerprint.h>
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);
uint8_t id = 1;//新增指紋順序
void OLED_SHOW(String string1 = "", String string2 = "", String string3 = "", String string4 = "");

String passcode = "4321";   // 預設密碼
String inputCode = "";      // 暫存用戶的按鍵字串
#define buzzer  15 //蜂鳴器腳
int Certification = 0;//用戶認證狀態，0為未認證，1為已經成功認證
unsigned long time_Relock = 0;

const char* ssid       = "kobe2324";
const char* password   = "asdfghjkl";

int door = 2;

void setup() {
  pinMode(buzzer, OUTPUT); //設定蜂鳴器控制腳為輸出
  digitalWrite(buzzer, !LOW);
  Serial.begin(9600);
  u8g2.begin();
  u8g2.enableUTF8Print();    // enable UTF8 support for the Arduino print() function
  u8g2.setFont(u8g2_font_unifont_t_chinese1);  // use chinese2 for all the glyphs of "你好世界"
  u8g2.setFontDirection(0);
  u8g2.clearBuffer();
  u8g2.setCursor(0, 15);
  u8g2.print("指紋識別系統");
  u8g2.print("");    // Chinese "Hello World"
  u8g2.sendBuffer();
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  }
  else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) {
      delay(1);
    }
  }
  //finger.emptyDatabase(); //清空指紋數據庫
  Serial.print("Sensor contains ");
  Serial.print(finger.templateCount);
  Serial.println(" templates");
  Serial.println("Waiting for valid finger...");
  time_Relock = millis();
  finger.emptyDatabase();

  pinMode(door, OUTPUT);
}

void loop() {
  if (millis() - time_Relock > 5000)//隔一段時間沒有操作自動鎖定
  {
    time_Relock = millis();
    locking();
  }

  getFingerprintIDez();
  char key = customKeypad.getKey();
  if (key) {
    time_Relock = millis();
    Serial.println(key);
    if (key == 'A') //鎖定
    {

    }
    else if (key == 'B')//新增指紋
    {
      if (Certification == 1)
      {
        OLED_SHOW("目前新增", String(id) + "號指紋", "請放置手指");
        getFingerprintEnroll();
      }
    }
    else if (key == '*' )//清除密碼輸入信息
    {
      inputCode = "";
      OLED_SHOW("您輸入的密碼為:");
    }
    else if (key == '#' )//確定鍵
    {
      if (inputCode == passcode) // 比對輸入密碼
      {
        OLED_SHOW("密碼正確!");
        //        Write_data("密碼解鎖");
        open_door();
        Certification = 1;
      }
      else
      {
        OLED_SHOW("密碼錯誤");
      }
      inputCode = "";
    }
    else if (key == 'C')//確定鍵
    {
      if (Certification == 1)
      {
        OLED_SHOW("按1清除指紋");
        delay(500);
        time_Relock  = millis();
        while (1)
        {
          char key = customKeypad.getKey();
          delay(50);
          if (key)// 若有按鍵被按下…
          {
            time_Relock  = millis();
            if (key == '1' )//確定鍵
            {
              finger.emptyDatabase();
              OLED_SHOW("已清空指紋信息");
              id = 1;
            }
            else if (key == 'D' )//確定鍵
            {
              locking();
              break;
            }
          }
          if (millis() - time_Relock > 5000)//隔一段時間沒有操作自動鎖定
          {
            break;
          }
        }
      }
    }
    else if (key == 'D' )//重設密碼
    {
      if (Certification == 1)
      {
        inputCode = "";
        OLED_SHOW("請輸入新密碼:");
        while (1)
        {
          char key = customKeypad.getKey();
          delay(50);
          if (key)// 若有按鍵被按下…
          {
            if (key == '*' )
            {
              inputCode = "";
            }
            else if (key == '#' )//確定鍵
            {
              passcode  = inputCode;   //更改密碼
              inputCode = "";
              OLED_SHOW("密碼更改成功!");
              break;
            }
            else if (key == 'A' )//確定鍵
            {
              OLED_SHOW("取消更改密碼");
              break;
            }
            else
            {
              inputCode += key;  // 儲存用戶的按鍵字元
            }
            String Show_str = "";
            if (inputCode.length() > 1)
            {
              for (int i = inputCode.length(); i > 1; i--)
              {
                Show_str = Show_str + "*";
              }
            }
            Show_str = Show_str + inputCode[inputCode.length() - 1];
            OLED_SHOW("新密碼為:", Show_str);
          }
        }
      }
      time_Relock = millis();
    }
    else
    {
      inputCode += key;  // 儲存用戶的按鍵字元
      String Show_str = "";
      if (inputCode.length() > 1)
      {
        for (int i = inputCode.length(); i > 1; i--)
        {
          Show_str = Show_str + "*";
        }
      }
      Show_str = Show_str + inputCode[inputCode.length() - 1];
      OLED_SHOW("您輸入的密碼為:", Show_str);
    }
  }
}

void OLED_SHOW(String string1, String string2, String string3, String string4)
{
  u8g2.clearBuffer();
  u8g2.setCursor(0, 16);
  u8g2.print(string1);
  u8g2.setCursor(0, 32);
  u8g2.print(string2);
  u8g2.setCursor(0, 48);
  u8g2.print(string3);
  u8g2.setCursor(0, 64);
  u8g2.print(string4);
  u8g2.sendBuffer();
}


void locking()
{
  SG90_close_door();
  inputCode = "";
  Certification = 0;
  OLED_SHOW("請輸入密碼開鎖", "或使用指紋開鎖");
}

//模擬開門
void open_door()
{
  SG90_open_door();
  Certification = 1;
  digitalWrite(buzzer, !HIGH);
  delay(200);
  digitalWrite(buzzer, !LOW);
  time_Relock = millis();
}

void SG90_close_door()
{
  for (int i = 200; i > 1; i--)
  {
    //關門
    digitalWrite(door, HIGH);
    delayMicroseconds(2350);
    digitalWrite(door, LOW);
    delayMicroseconds(10);
  }
}
void SG90_open_door()
{
  for (int i = 200; i > 1; i--)
  {
    //開門
    digitalWrite(door, HIGH);
    delayMicroseconds(500);
    digitalWrite(door, LOW);
    delayMicroseconds(10);
  }
}


//////////////////////////以下程式碼修改自比對指紋信息範例/////////////
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  // OK success!
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  //如果有比對到配對的指紋會返回大約0的數值
  if (finger.fingerID >= 0)
  {
    OLED_SHOW("指紋正確", "已經解鎖", "指紋編號為" + String(finger.fingerID));
    open_door();
  }
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}

//////////////////////////以下程式碼修改自新增指紋範例/////////////////////////////////////////////////////////////
uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while ((p != FINGERPRINT_OK)) {
    p = finger.getImage();
    //    if (p == FINGERPRINT_NOFINGER)
    //      break;
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");//圖片拍攝
        break;
      case FINGERPRINT_NOFINGER://手指指紋
        Serial.println(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");//通信故障
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");//成像误差
        break;
      default:
        Serial.println("Unknown error");//未知錯誤
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");//圖片轉換
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");//圖片太亂
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");//通訊錯誤
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");//找不到指紋
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");//找不到指紋
      return p;
    default:
      Serial.println("Unknown error");//未知指紋
      return p;
  }
  OLED_SHOW("請重新放置手指");
  Serial.println("Remove finger");//移開手指
  //  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");//再次放置同一根手指
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  /////////////////////////////////////
  if (p == FINGERPRINT_OK)
  {
    OLED_SHOW("指紋新增成功!");
    id = id + 1;
  }
  else
  {
    OLED_SHOW("指紋新增失敗!", "請重新嘗試");
  }
  delay(1000);
  /////////////////////////////////////
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
}