#include <Arduino.h>
#include "define.h"
unsigned long t1=0;
int i=0;
static bool task_status=false;
static void task_lora_data(void *pvParameters)
{
  
  for(;;)
  {
   
   if ((unsigned long)(millis() - t1) >500)
   {
    t1 = millis();
      if(state_request &&task_status ==false)
     {
      state_request=false;
      destination_addr =Node_addr[3];
      cmd =cmdR;
      sendRequest(destination_addr,cmd);
      LoRa.receive();
      if(state_request==false) {state_request =true;i+=1;if(i>3) i =0;}
      }     
   }
   vTaskDelay(1);
  }
}
unsigned long t2=0;
unsigned long t3=0;
int rd=0;
unsigned long time_setting=0;
unsigned long timeout_setting=0;
static void task_setting_lora(void *pvParameters)
{  
  for(;;)
  {      
    if ((unsigned long)(millis() - time_setting) >1000-rd)
    {
      
      time_setting =millis(); 
      if(task_status )
      {
      destination_addr =Node_addr[3];
      cmd= cmdS+"/"+ (String)read_eeprom(addrepprom);
      sendRequest(destination_addr,cmd);
      LoRa.receive(); 
     if(setting !="OK" && ((unsigned long)(millis() - timeout_setting) >= 5000)) 
     {
      timeout_setting =millis(); 
      Serial.println("setting faile"); task_status =false;
      }  
      else { task_status =false;
    // json1.set("succesful", );
    // Firebase.pushJSON(fbdo, "/status/", json1); 
      }
      }
    }
      vTaskDelay(10);
  }
}
char timeWeekDay[10];
struct tm timeinfo;
static void task_manager(void *pvParameters)
{
   
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
   if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  for(;;)
  {
 
 
  strftime(timeWeekDay,10, "%A", &timeinfo);
  //Serial.println(timeWeekDay);
  vTaskDelay(1000/ portTICK_PERIOD_MS);
  }
}
 int countconfig=0;
static void task_button_config(void *pvParameters)
{
 pinMode(button_config,INPUT);
 pinMode(led_config,OUTPUT); 
 digitalWrite(led_config, LOW);
  for(;;)
  {
   if(digitalRead(button_config)==0)
   {
    countconfig++;
      if(countconfig >50)
      {      
         digitalWrite(led_config, HIGH);
       //  wm.resetSettings();  
        // ESP.restart();
       }
    Serial.println(countconfig);
   }
    else countconfig =0;  
  vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
static void task_OTA(void *pvParameters)
{
  for(;;)
  {
  server.handleClient();
  vTaskDelay(1/ portTICK_PERIOD_MS);
  }
}
void setup() {
 Serial.begin(115200);
 EEPROM.begin(4);
 wificonfig();
 OTA_config();
 lora_Config();
 firebaseSetup();
  xTaskCreate(task_lora_data,"lora",5000,NULL,0,&TaskHandle_1);
 xTaskCreate(task_setting_lora,"setting",1000,NULL,0,&TaskHandle_2);
 xTaskCreate(task_manager,"manager",10000,NULL,1,&TaskHandle_3);
 xTaskCreate(task_button_config,"button",1000,NULL,2,&TaskHandle_4);
 xTaskCreate(task_OTA,"OTA",10000,NULL,3,&TaskHandle_5);
 last_oxysetup = read_eeprom(addrepprom);
}

void loop() {
rd = random(100);
   if ((unsigned long)(millis() - t2) >1000-rd)
   {  
    t2 = millis();
    json.set("parent_001", "parent 001 text");
    Firebase.pushJSON(fbdo, "/test/append", json);  
   }
   if((unsigned long)(millis() - t3) >2000+rd)
 {
    t3=millis() ;
    if (Firebase.getString(fbdo, "/OxySV")) {
      oxysetup= fbdo.stringData();
      Serial.println("OxySv:"+oxysetup);
      if(oxysetup !=last_oxysetup)
      {
        last_oxysetup= oxysetup;
        write_eeprom (addrepprom,last_oxysetup.toFloat());
        task_status =true;
      }
     }     
     //Serial.println(read_eeprom(addrepprom));
  }
}

