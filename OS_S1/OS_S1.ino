#include <Arduino_FreeRTOS.h>
#include <SPI.h> 
#include <LoRa.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DFRobot_MAX17043.h"
#include <Wire.h>

//==================================================
#define ALR_PIN  3
#define ds18b20_pin 7
#define ph_pin A0
#define oxy_pin A1
#define csPin 10         
#define resetPin  9      
#define irqPin  2 
#define gattway_addr 0x00
#define nodeOne_addr 0x01
#define nodeFourth_addr 0x04
#define fre 433E6
//====================================================
float pH,temp,battery_per,oxy;
void task_read_ph(void *pvParameters );
void task_read_oxy(void *pvParameters);
void task_read_temp(void *pvParameters);
void task_read_battery_per(void *pvParameters);
void task_lora_data(void *pvParameters);
uint8_t calc_crc(uint8_t data);
uint8_t crc8(byte data[], byte n);
void sendMessage(byte addr_destination, String message);
void onReceive(int packetSize);
 
//=======================================================
OneWire oneWire(ds18b20_pin);
DallasTemperature ds18b20(&oneWire);
DFRobot_MAX17043 gauge;
String cmd ="R";
String msgsend = "";
uint8_t intFlag = 0;
unsigned long t1=0;

//====================================
void task_lora_data(void *pvParameters)
{
  // Serial.begin(9600);
   
  for(;;)
  {
   
   vTaskDelay(10);
  }
}

//========================================
void task_read_ph(void *pvParameters)
{
   pinMode(ph_pin,INPUT);
  for(;;)
  {
     
      vTaskDelay(500);
  }
}
//============================================
void task_read_temp(void *pvParameters)
{
  ds18b20.begin();
  for(;;)
  {
      ds18b20.requestTemperatures(); 
      temp= ds18b20.getTempCByIndex(0);
      vTaskDelay(500);
  }
}
//========================================
void task_read_oxy(void *pvParameters)
{
 pinMode(oxy_pin,INPUT);
  for(;;)
  {
     
      vTaskDelay(500);
  }
}
//=============================================

void interruptCallBack()
{
  intFlag = 1;
}
void task_read_battery_per(void *pvParameters)
{
  pinMode(ALR_PIN, INPUT_PULLUP);
  attachInterrupt(ALR_PIN, interruptCallBack, FALLING); 
  while(gauge.begin() != 0) {
    Serial.println("gauge begin faild!");
    delay(2000);
  }
  delay(2);
  Serial.println("gauge begin successful!");
  gauge.setInterrupt(10); 
  for(;;)
  {
    battery_per= gauge.readPercentage();
    if(intFlag == 1) {
    intFlag = 0;
    gauge.clearInterrupt();
    Serial.println("Low power alert interrupt!");
    }
      vTaskDelay(500);
  }
}
//=================================================
void setup() {
 Serial.begin(9600);
 LoRa.setPins(csPin, resetPin, irqPin);
   if (!LoRa.begin(fre)) {   
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       
  }
  Serial.println("LoRa init succeeded.");
  LoRa.onReceive(onReceive);
  LoRa.receive();
  LoRa.setTxPower(18);
  LoRa.enableCrc();
 //xTaskCreate(task_lora_data,"lora",1000,NULL,1,NULL);
// xTaskCreate(task_read_temp,"temp",1000,NULL,0,NULL);
// xTaskCreate(task_read_ph,"ph",1000,NULL,0,NULL);
// xTaskCreate(task_read_oxy,"oxy",1000,NULL,0,NULL);  
// xTaskCreate(task_read_battery_per,"bat",1000,NULL,0,NULL); 
}

void loop() {
if ((unsigned long)(millis() - t1) >1000)
   {
    t1 = millis();
   pH=1;
   temp=1;
   battery_per=1;
   oxy=1;  
    LoRa.receive();
   }
}


void sendMessage(byte destination_addr, String message)
 {
  byte data_send[3];
  data_send[0] = destination_addr;
  data_send[1] = nodeOne_addr;
  data_send[2] = message.length();
  byte crc=crc8(data_send, 3);
  LoRa.beginPacket();
  LoRa.write(data_send[0]); //
  LoRa.write(data_send[1]); //
  LoRa.write(data_send[2]); //
  LoRa.write(crc); 
  LoRa.print(message);
  LoRa.endPacket();
 }
 
void onReceive(int packetSize)
 {
   uint8_t data_rec[4];
   String LoRaData="";
   if (packetSize == 0) return; 
   data_rec[0]=LoRa.read(); Serial.println(data_rec[0]);//addr node 
   data_rec[1]=LoRa.read(); Serial.println(data_rec[1]);// addr gatt
   data_rec[2]=LoRa.read(); Serial.println(data_rec[2]); // length cmd
   data_rec[3]=LoRa.read(); Serial.println(data_rec[3]);// crc
   while (LoRa.available()) LoRaData = LoRa.readString(); Serial.println(LoRaData);       
   if ((data_rec[0] != nodeOne_addr)|| (data_rec[1]!= gattway_addr )){ Serial.println("not for me."); return ;}                         
   if(data_rec[3] != crc8(data_rec, 3)){ Serial.println("crc faile"); return;}
   if(LoRaData.length() != data_rec[2]){ Serial.println("data length faile"); return;}
   msgsend=(String)temp+"a"+(String)pH+"b" +(String)oxy+"c"+(String)battery_per+"/";
    Serial.println(msgsend);
   if(LoRaData == cmd) 
   { 
    sendMessage(data_rec[1],msgsend);   
   }
 }
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
