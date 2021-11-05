#include <WiFi.h>
#include <FirebaseESP32.h>
#include <SPI.h>              
#include <LoRa.h>
#include <EEPROM.h>
#include "time.h"
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
//=======================================================
#define csPin 5         
#define resetPin  14      
#define irqPin  15
#define button_config  22
#define led_config 21 
#define led_run 4
//=======================================================
#define gattway_addr 0x00
#define nodeOne_addr 0x01
#define nodeTwo_addr 0x02
#define nodeThree_addr 0x03
#define nodeFourth_addr 0x04
#define fre 433E6
#define FIREBASE_HOST "lora-20495-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "rp60h2bwIrEOUjofkM7r0osVjK4V8ViMTcEPcBUE" 
const char* ssid = "Kidkul2030";//B 0703 //CR7//Tan Phat//Thu Hien//Kidkul2030
const char* password = "kidkul@2018";//kidkul2021//xindelamgi//02022021//199619971998//kidkul@2018
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600*(-6);
const int   daylightOffset_sec = 3600;
//=======================================================
uint8_t calc_crc(uint8_t data);
uint8_t crc8(byte data[], byte n);
void sendRequest(byte addr_destination, String message);
void onReceive(int packetSize);
void decode_data(byte addr,String sms);
void lora_Config();
void firebaseSetup();
void firebaseSetup();
float read_eeprom(int addr);
void write_eeprom(int addr,float value);
void printLocalTime();
void OTA_config();
//========================================================
byte destination_addr=0x00;
String cmdR = "R";
String cmdS = "S";
String cmd = "";
bool state_request=true;
String oxysetup ="";
String last_oxysetup ="";
String temp,pH,oxy,TDS,level,bat1,bat2,bat3,bat4,Voltage,Cunrrent,setting;
byte Node_addr[4]={nodeOne_addr,nodeTwo_addr,nodeThree_addr,nodeFourth_addr};
FirebaseData fbdo;
FirebaseJson json;
FirebaseJson json1;
TaskHandle_t TaskHandle_1;
TaskHandle_t TaskHandle_2;
TaskHandle_t TaskHandle_3;
TaskHandle_t TaskHandle_4;
TaskHandle_t TaskHandle_5;
int addrepprom = 0;
  union {
    float fval;
    byte bval[4];
  } floatAsBytes;
  float readfloat;
  union u_tag {
    byte b[4];
    float fval;
  } u;
  WebServer server(80);
const char* host = "esp32";
 const char* loginIndex =
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>ESP32 Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
             "<td>Username:</td>"
             "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";

/*
 * Server Index Page
 */

const char* serverIndex =
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')"
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";
void sendRequest(byte addr_destination, String message)
{
  uint8_t data_send[3]; 
  data_send[0] = addr_destination;
  data_send[1] = gattway_addr;
  data_send[2] = message.length();
  uint8_t crc=crc8(data_send, 3);
  LoRa.beginPacket();
  LoRa.write(data_send[0]);
  LoRa.write(data_send[1]);
  LoRa.write(data_send[2]);
  LoRa.write(crc);
  LoRa.print(message);
  LoRa.endPacket();
 // state_request=false;
}
void onReceive(int packetSize)
{
   byte data_rec[4];
   String LoRaData="";
   if (packetSize == 0) return; 
   data_rec[0]=LoRa.read(); //Serial.println(data_rec[0]);//addr gatt
   data_rec[1]=LoRa.read();//Serial.println(data_rec[1]);// addr node
   data_rec[2]=LoRa.read(); //Serial.println(data_rec[2]);
   data_rec[3]=LoRa.read();// Serial.println(data_rec[2]);
   while (LoRa.available()) LoRaData = LoRa.readString(); //Serial.println(LoRaData);        
   if ((data_rec[0] != gattway_addr)|| (data_rec[1]!= destination_addr )){ Serial.println("not for me."); return ;}                         
   if(data_rec[3] != crc8(data_rec, 3)){ Serial.println("crc faile"); return;}
   if(LoRaData.length() != data_rec[2]){ Serial.println("data length faile"); return;}
   decode_data(data_rec[1],LoRaData);
   
}
//========================================================
 uint8_t calc_crc(uint8_t data) {
    int index;
    uint8_t temp;
    for (index = 0; index < 8; index++) {
        temp = data;
        data <<= 1;
        if (temp & 0x80) {data ^= 0x07;}
    }
    return data;
}
uint8_t crc8(byte data[], byte n)
{
    int i;
    uint8_t crc = calc_crc((0x1D << 1) | 1);
    for (i = 0; i < n; i++) {
        crc = calc_crc(data[i] ^ crc);
    }
    return crc;
}
void decode_data(byte addr,String sms)
{
  int pos1,pos2,pos3,pos4,pos5,pos6,pos7,pos8,pos9,pos10,pos11; 
  switch(addr)
  {
   
    case nodeOne_addr:
    {
     pos1 = sms.indexOf('a');
     pos2 = sms.indexOf('b');
     pos3 = sms.indexOf('c');
     pos4 = sms.indexOf('/');
     temp =sms.substring(0, pos1);
     pH = sms.substring(pos1 +1, pos2);
     oxy =sms.substring(pos2 +1, pos3);
     bat1=sms.substring(pos3 +1, pos4);
     Serial.println("T:"+temp+  "pH:" +pH + "oxy:"+oxy  + "bat1:"+bat1);
      break;
    }
     case nodeTwo_addr:
    {
        pos7 = sms.indexOf('f');
        pos8 = sms.indexOf('g');
        level=sms.substring(0, pos7);
        bat2=sms.substring(pos7 +1, pos8);
        Serial.println("level:"+level+"bat2:"+bat2);
      
      break;
    }
     case nodeThree_addr:
    {
        pos5 = sms.indexOf('d');
        pos6 = sms.indexOf('e');
        TDS=sms.substring(0, pos5);
        bat3=sms.substring(pos5 +1, pos6);
        Serial.println("TDS:"+TDS+"bat3:"+bat3);
    
      break;
    }
    case nodeFourth_addr:
    {
        if(cmd==cmdR) 
        {
        pos9 = sms.indexOf('h');
        pos10 = sms.indexOf('j');
        pos11 = sms.indexOf('k');//Voltage,Cunrrent
        Voltage=sms.substring(0, pos9);
        Cunrrent=sms.substring(pos9 +1, pos10);
        bat4=sms.substring(pos10 +1, pos11);
        Serial.println("Voltage:"+Voltage+"Cunrrent:"+Cunrrent+"bat4:"+bat4);
        }
        else  //if(cmd==cmdS)
        {
          setting= sms;
          Serial.println(setting);
          
        }
       
      break;
    } 
    default : break;
  }
}
void lora_Config()
{
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin
  // override the default CS, reset, and IRQ pins (optional)
  if (!LoRa.begin(fre)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    delay(5000);
    ESP.restart();
  }
  LoRa.setTxPower(20);    
  LoRa.onReceive(onReceive);
  LoRa.receive();
  LoRa.enableCrc();
  Serial.println("LoRa init succeeded.");
}
void firebaseSetup()
{
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);
    Serial.println("Firebase OK!");
}
void wificonfig()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid,password); 
    
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
   WiFi.reconnect();
   Serial.println("wifi ok!");
}
void write_eeprom(int addr,float value)
{
  floatAsBytes.fval = value;
  EEPROM.write(addr, floatAsBytes.bval[0]);
  EEPROM.write(addr+1, floatAsBytes.bval[1]);
  EEPROM.write(addr+2, floatAsBytes.bval[2]);
  EEPROM.write(addr+3, floatAsBytes.bval[3]);
  EEPROM.commit();
  Serial.println("save succesfully!");
}
float read_eeprom(int addr)
{
  float readfloat;
  u.b[0] = EEPROM.read(addr);
  u.b[1] = EEPROM.read(addr+1);
  u.b[2] = EEPROM.read(addr+2);
  u.b[3] = EEPROM.read(addr+3);
  readfloat = u.fval; 
  return readfloat;
}
void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}
void OTA_config()
{
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
 if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
       vTaskSuspend(TaskHandle_1);
       vTaskSuspend(TaskHandle_2);
       vTaskSuspend(TaskHandle_3);
       vTaskSuspend(TaskHandle_4);
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
}