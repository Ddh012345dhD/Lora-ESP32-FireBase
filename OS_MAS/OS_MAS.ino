#include <FirebaseESP32.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <EEPROM.h>
#include <SPI.h>              
#include <LoRa.h>
#include <WiFiManager.h>
#include "BluetoothSerial.h"
//=======================================================
#define csPin 5         
#define resetPin  4      
#define irqPin  16
#define SD_CS 13
#define SD_SCK 14
#define SD_MOSI 15
#define SD_MISO 2
#define button_config  25
#define led_config 27 
//=======================================================

#define FIREBASE_HOST "lora123-5fffa-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "SKOrTgFG49Hs3qqAOXcyXP0lPLLfmESaht5gjlt6"
//lv123-a5177-default-rtdb.firebaseio.com
//nlwHxr8rmVz2FRP7YExSVMhaDD0HLhtSHfKeCbJ9 
//const char* ssid = "Tan Phat";//B 0703 //CR7//Tan Phat//Thu Hien//Kidkul2030
//const char* password = "02022021";//kidkul2021//xindelamgi//02022021//199619971998//kidkul@2018
#define gattway_addr 0x00
#define nodeOne_addr 0x01
#define nodeTwo_addr 0x02
#define nodeThree_addr 0x03
#define nodeFourth_addr 0x04
#define fre 433E6
byte destination_addr=0x00;
String cmdR = "RQ";
String cmdS = "S";
String cmd = "";
static String setting ="";
bool wm_nonblocking = false;
bool respons;

String temp,pH,oxy,TDS,level,bat1,bat2,bat3,bat4,Voltage,Cunrrent;
byte Node_addr[4]={nodeOne_addr,nodeTwo_addr,nodeThree_addr,nodeFourth_addr};
TaskHandle_t TaskHandle_1;
TaskHandle_t TaskHandle_2;
TaskHandle_t TaskHandle_3;
TaskHandle_t TaskHandle_4;
TaskHandle_t TaskHandle_5;
TaskHandle_t TaskHandle_6;
TaskHandle_t TaskHandle_7;
 WebServer server(80);
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
 const char* host = "esp32";
 WiFiManager wm; 
WiFiManagerParameter custom_field;
IPAddress local_IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8); 
IPAddress secondaryDNS(8, 8, 4, 4);
SPIClass sd_spi(HSPI);

//=======================================================
float read_eeprom(int addr);
void write_eeprom(int addr,float value);
void firebaseSetup();
uint8_t calc_crc(uint8_t data);
uint8_t crc8(byte data[], byte n);
void sendRequest(byte addr_destination, String message);
void onReceive(int packetSize);
void decode_data(byte addr,String sms);
void loraSetup();
void OTA_config();
String getParam(String name);
void wificonfig();
void saveParamCallback();
void SD_setup();
void appendFile(fs::FS &fs, const char * path, const char * message);
//========================================================
int addrepprom_oxy = 0;
int addrepprom_kp = addrepprom_oxy+4;
int addrepprom_ki = addrepprom_kp+4;
int addrepprom_kd = addrepprom_ki+4;
  union {
    float fval;
    byte bval[4];
  } floatAsBytes;
  float readfloat;
  union u_tag {
    byte b[4];
    float fval;
  } u;

String oxysetup ="";
String last_oxysetup ="";
String kp ="";
String ki="";
String kd="";
String last_kp ="";
String last_ki="";
String last_kd="";
FirebaseData fbdo;
FirebaseJson json;
FirebaseJson json1;
//========================================================
unsigned long t1=0;
int i=0;
int rd=0;
static int iloi1=0;
static int iloi2=0;
static int iloi3=0;
static int iloi4=0;
bool state_request=true;
static bool set_status=false;
static bool task_status=false;
static String node_status="";
static void task_lora_data(void *pvParameters)
{ 
  for(;;)
  { 
   if ((unsigned long)(millis() - t1) >(1000-rd))
   {
    t1 = millis();
    if(state_request && task_status ==false)//
     {
      state_request=false;
      destination_addr =Node_addr[i];
      cmd =cmdR;
      sendRequest(destination_addr,cmd);
      LoRa.receive();
      if(state_request==false) {state_request =true;i+=1;if(i>3) i =0;}
      }     
   }
   vTaskDelay(10);
  }
}

unsigned long time_setting=0;
static void task_setting_lora(void *pvParameters)
{  
  for(;;)
  {    
    if ((unsigned long)(millis() - time_setting) >(1000-rd))
    {     
      time_setting =millis(); 
      if(task_status)
      {
      
      destination_addr =Node_addr[3];
      if(set_status)cmd= cmdS+"O"+ (String)read_eeprom(addrepprom_oxy);
      else
      {
        String dta = (String)read_eeprom(addrepprom_kp) + "/"+(String)read_eeprom(addrepprom_ki) +"*"+(String)read_eeprom(addrepprom_kd)+"+";
       // Serial.println(dta);
        cmd= cmdS+"P"+dta;
      }
      sendRequest(destination_addr,cmd);
      LoRa.receive();
      task_status ==false;
      }
    }
      vTaskDelay(10);
  }
}

static void task_realtime(void *pvParameters)
{
   
  for(;;)
  {
  vTaskDelay(1000/ portTICK_PERIOD_MS);
  }
}
 int countconfig=0;
 unsigned long timecheck_config=0;
static void task_button_config(void *pvParameters)
{

 digitalWrite(led_config, HIGH);
  for(;;)
  {
    if((unsigned long)(millis() - timecheck_config) >(500+rd))
    {
     timecheck_config = millis();
     if(wm_nonblocking) wm.process();
    if(digitalRead(button_config) == 0)
    {
      countconfig ++;
      Serial.println(countconfig);
      if(countconfig>10)
      {
       digitalWrite(led_config,LOW);
        wm.resetSettings();
        ESP.restart();
      }
    }
     else countconfig=0;
    }
   vTaskDelay(10/ portTICK_PERIOD_MS);
  }
}
static void task_OTA(void *pvParameters)
{
  for(;;)
  {
  server.handleClient();
  vTaskDelay(10/ portTICK_PERIOD_MS);
  }
}
static void task_blink_offline(void *pvParameters)
{
  for(;;)
  {
   digitalWrite(led_config,1);
   vTaskDelay(1000/ portTICK_PERIOD_MS);
   digitalWrite(led_config,0);
   vTaskDelay(1000/ portTICK_PERIOD_MS);
  }
}
void setup() {
 Serial.begin(115200);
 EEPROM.begin(20);
 
 wificonfig();
 OTA_config();
 loraSetup();
 firebaseSetup();
// SD_setup();
 xTaskCreate(task_lora_data,"lora",10000,NULL,1,&TaskHandle_1);
 xTaskCreate(task_setting_lora,"setting",5000,NULL,1,&TaskHandle_2);
 xTaskCreate(task_realtime,"realtime",1000,NULL,0,&TaskHandle_3);
 xTaskCreate(task_button_config,"button",1000,NULL,0,&TaskHandle_4);
 if(respons) xTaskCreate(task_OTA,"OTA",10000,NULL,3,&TaskHandle_5); 
 else xTaskCreate(task_blink_offline,"blink",1000,NULL,0,&TaskHandle_6);
 last_oxysetup = read_eeprom(addrepprom_oxy);
 Serial.println("last_oxysetup:" + (String)last_oxysetup);
 last_kp = read_eeprom(addrepprom_kp);
 Serial.println("last_kp:" + (String)last_kp);
 pinMode(button_config,INPUT);
 pinMode(led_config,OUTPUT); 
// appendFile(SD, "/hello1.txt", "World!\n");
}
unsigned long t2=0;
unsigned long t3=0;
void loop() { 
 
  rd = random(100);
  if(!respons){
    Serial.println(111);
    delay(100);
    }
    else
    {
    if ((unsigned long)(millis() - t2) >(1000-rd))
   {  
    t2 = millis();
    json.set("parent_001", "parent 001 text");
    Firebase.pushJSON(fbdo, "/test/append", json);  
   }
   if((unsigned long)(millis() - t3) >(2000+rd))
   {
    t3=millis() ;
    if (Firebase.getString(fbdo, "/OxySV")) {
      oxysetup= fbdo.stringData();
      Serial.println("OxySv:"+oxysetup);
      if(oxysetup !=last_oxysetup)
      {
        last_oxysetup= oxysetup;
        write_eeprom (addrepprom_oxy,last_oxysetup.toFloat());
        task_status =true;
        set_status=true;
      }
     } 
     if (Firebase.getString(fbdo, "/kp")) {
      kp= fbdo.stringData();
      //Serial.println("kp:"+kp);
      if(kp !=last_kp)
      {
        last_kp= kp;
        write_eeprom (addrepprom_kp,last_kp.toFloat());
        task_status =true;
        set_status=false;
      }
     } 
      if (Firebase.getString(fbdo, "/ki")) {
      ki= fbdo.stringData();
      Serial.println("ki:"+ki);
      if(ki !=last_ki)
      {
        last_ki= ki;
        write_eeprom (addrepprom_ki,last_ki.toFloat());
        task_status =true;
        set_status=false;
      }
     } 
      if (Firebase.getString(fbdo, "/kd")) {
      kd= fbdo.stringData();
      Serial.println("kd:"+kd);
      if(kd !=last_kd)
      {
        last_kd=kd;
        write_eeprom(addrepprom_kd,last_kd.toFloat());
        task_status =true;
        set_status=false;
      }
     }     
   }  
 }
}

void firebaseSetup()
{
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);
    Serial.println("Firebase OK!");
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
   //if(data_rec[1] == nodeOne_addr && LoRaData ==NULL )iloi1 +=1;
   //Serial.println("iloi1:" +(String)iloi1);
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
unsigned long timeout_setting=0;
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
     node_status = sms.substring(pos4 +1, sms.length());
     Serial.println("T:"+temp+  "pH:" +pH + "oxy:"+oxy  + "bat1:"+bat1);
     if(node_status !="OK1" ||sms ==NULL) iloi1 +=1;  
     Serial.println("iloi1:" +(String)iloi1);
      break;
    }
     case nodeTwo_addr:
    {
        pos7 = sms.indexOf('f');
        pos8 = sms.indexOf('/');
        level=sms.substring(0, pos7);
        bat2=sms.substring(pos7 +1, pos8);
        node_status = sms.substring(pos8 +1, sms.length());
        Serial.println("level:"+level+"bat2:"+bat2);
        if(node_status !="OK2" ||sms ==NULL ) iloi2 +=2;  
       Serial.println("iloi2:" +(String)iloi2);
      break;
    }
     case nodeThree_addr:
    {
        pos5 = sms.indexOf('d');
        pos6 = sms.indexOf('/');
        TDS=sms.substring(0, pos5);
        bat3=sms.substring(pos5 +1, pos6);
        node_status = sms.substring(pos6 +1, sms.length());
        Serial.println("TDS:"+TDS+"bat3:"+bat3);
        if(node_status !="OK3" ||sms ==NULL ) iloi3 +=1;  
       Serial.println("iloi3:" +(String)iloi3);
      break;
    }
    case nodeFourth_addr:
    {
        if(cmd==cmdR) 
        {
        pos9 = sms.indexOf('h');
        pos10 = sms.indexOf('j');
        pos11 = sms.indexOf('/');//Voltage,Cunrrent
        Voltage=sms.substring(0, pos9);
        Cunrrent=sms.substring(pos9 +1, pos10);
        bat4=sms.substring(pos10 +1, pos11);
        node_status = sms.substring(pos11 +1, sms.length());
        Serial.println("Voltage:"+Voltage+"Cunrrent:"+Cunrrent+"bat4:"+bat4 + "node_status"+node_status);
        if(node_status !="OK4" ||sms ==NULL ) iloi4 +=1;  
        Serial.println("iloi4:" +(String)iloi4);
        }
        else  //if(cmd==cmdS)
        {
          setting= sms;
          Serial.println("setting:" + setting);
       if((unsigned long)(millis() -timeout_setting) >=5000 )
      {
        timeout_setting= millis();
                if(setting =="OKO" && set_status )
         {
        Serial.println("setting oxy succesfully!"); 
        task_status =false;
        setting="";
        }
         else if(setting !="OKO" && set_status )
        {
          Serial.println("setting  oxy failed");
          task_status =false;
          setting="";
         } 
        else if(setting =="OKP" && set_status ==false )
        {
        Serial.println("setting kpid succesfully!"); 
        task_status =false;
        setting="";
        }
         else if(setting !="OKP" && set_status ==false )
        {
          Serial.println("setting kpid failed!");
          task_status =false;
          setting="";
         }  
      } 
      }
        
      break;
    } 
    default : break;
  }
}
void loraSetup()
{
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin
  // override the default CS, reset, and IRQ pins (optional)
  if (!LoRa.begin(fre)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    delay(5000);
    //ESP.restart();
  }
  LoRa.setTxPower(20);    
  LoRa.onReceive(onReceive);
  LoRa.receive();
  LoRa.enableCrc();
  Serial.println("LoRa init succeeded.");
}
void OTA_config()
{

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
void wificonfig()
{
     WiFi.mode(WIFI_STA);
     if(!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
   Serial.println("STA Failed to configure");
   }
  Serial.println("");
  Serial.print("Connected to ");
  //Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  if(wm_nonblocking) wm.setConfigPortalBlocking(false);

  // add a custom input field
  int customFieldLength = 40;


  // new (&custom_field) WiFiManagerParameter("customfieldid", "Custom Field Label", "Custom Field Value", customFieldLength,"placeholder=\"Custom Field Placeholder\"");
  
  // test custom html input type(checkbox)
  // new (&custom_field) WiFiManagerParameter("customfieldid", "Custom Field Label", "Custom Field Value", customFieldLength,"placeholder=\"Custom Field Placeholder\" type=\"checkbox\""); // custom html type
  
  // test custom html(radio)
  const char* custom_radio_str = "<br/><label for='customfieldid'>Custom Field Label</label><input type='radio' name='customfieldid' value='1' checked> One<br><input type='radio' name='customfieldid' value='2'> Two<br><input type='radio' name='customfieldid' value='3'> Three";
  new (&custom_field) WiFiManagerParameter(custom_radio_str); // custom html input
  
  wm.addParameter(&custom_field);
  wm.setSaveParamsCallback(saveParamCallback);

  // custom menu via array or vector
  // 
  // menu tokens, "wifi","wifinoscan","info","param","close","sep","erase","restart","exit" (sep is seperator) (if param is in menu, params will not show up in wifi page!)
  // const char* menu[] = {"wifi","info","param","sep","restart","exit"}; 
  // wm.setMenu(menu,6);
  std::vector<const char *> menu = {"wifi","info","param","sep","restart","exit"};
  wm.setMenu(menu);

  // set dark theme
  wm.setClass("invert");


  //set static ip
  // wm.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); // set static ip,gw,sn
  // wm.setShowStaticFields(true); // force show static ip fields
  // wm.setShowDnsFields(true);    // force show dns field always

  // wm.setConnectTimeout(20); // how long to try to connect for before continuing
  wm.setConfigPortalTimeout(30); // auto close configportal after n seconds
  // wm.setCaptivePortalEnable(false); // disable captive portal redirection
  // wm.setAPClientCheck(true); // avoid timeout if client connected to softap

  // wifi scan settings
  // wm.setRemoveDuplicateAPs(false); // do not remove duplicate ap names (true)
  // wm.setMinimumSignalQuality(20);  // set min RSSI (percentage) to show in scans, null = 8%
  // wm.setShowInfoErase(false);      // do not show erase button on info page
  // wm.setScanDispPerc(true);       // show RSSI as percentage not graph icons
  
  // wm.setBreakAfterConfig(true);   // always exit configportal even if wifi save fails

 
  // res = wm.autoConnect(); // auto generated AP name from chipid
  // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
  respons = wm.autoConnect("ESP_LORA","1234567890"); // password protected ap

  if(!respons) {
    Serial.println("Failed to connect or hit timeout");
    // ESP.restart();
  } 
  else {
    //if you get here you have connected to the WiFi    
    Serial.println("connected...yeey :)");
  }
  WiFi.reconnect();
}
String getParam(String name){
  //read parameter from server, for customhmtl input
  String value;
  if(wm.server->hasArg(name)) {
    value = wm.server->arg(name);
  }
  return value;
}

void saveParamCallback(){
  Serial.println("[CALLBACK] saveParamCallback fired");
  Serial.println("PARAM customfieldid = " + getParam("customfieldid"));
}

void SD_setup()
{
   // sd_spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
   // MailClient.sdBegin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    if (!SD.begin(SD_CS, sd_spi)) Serial.println("SD Card: mounting failed.");  
    else Serial.println("SD Card: mounted.");      
}
void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
      Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}
