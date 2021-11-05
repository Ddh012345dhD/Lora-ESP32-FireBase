#include <SPI.h> 
#include <LoRa.h>
#include "DFRobot_MAX17043.h"
#include <Wire.h>
#include <EEPROM.h>
//==================================================
#define csPin 10         
#define resetPin  9      
#define irqPin  2 
#define gattway_addr 0x00
#define nodeOne_addr 0x02
#define nodeFourth_addr 0x04
#define fre 433E6
//====================================================
float volt,current;
int battery_per;
uint8_t calc_crc(uint8_t data);
uint8_t crc8(byte data[], byte n);
void sendMessage(byte addr_destination, String message);
void onReceive(int packetSize);
float read_eeprom(int addr);
void write_eeprom(int addr,float value);
void lora_setup();
void decode_data(byte addr,String dta);
int read_power();
float read_curent();
float read_voltage();
//=======================================================
DFRobot_MAX17043 gauge;
//String cmd ="";
String cmdSO ="SO";
String cmdR ="RQ";
String cmdSPID ="SP";
String msgsend = "";
unsigned long t1=0;
unsigned long t2=0;
int addrepprom_Oxy = 0;
int addrepprom_kp = addrepprom_Oxy+4;
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
  float oxy_set=0.0;
  float last_oxy_set=0.0;
  float Votaltage_max = 4250;
  float Votaltage_min = 3600;
  int delta_voltage = Votaltage_max - Votaltage_min;
  int offset_voltage=13;
//====================================
void setup() {
 Serial.begin(115200);
 lora_setup();
 /*
 while(gauge.begin() != 0) {
    Serial.println("gauge begin faild!");
    delay(2000);
  }
  delay(2);
  Serial.println("gauge begin successful!");
  gauge.setInterrupt(10); 
  */
}
int rd=0;
void loop() {
  rd=random(0,100);
if ((unsigned long)(millis() - t1) >1000-rd)
   {
   t1 = millis();
   Serial.println("oxy:" + (String)read_eeprom(addrepprom_Oxy));
   Serial.println("kp:" + (String)read_eeprom(addrepprom_kp));
   Serial.println("ki:" + (String)read_eeprom(addrepprom_ki));
   Serial.println("kd:" + (String)read_eeprom(addrepprom_kd));
   
   LoRa.receive();
   }
   if ((unsigned long)(millis() - t2) >1000+rd)
   {
    t2 = millis();
    volt=3.5;
    current =4.5;
    battery_per= read_power();
    // battery_per= gauge.readVoltage() +offset_voltage;
   // Serial.println(battery_per);
    }
   } 
int read_power()
{
   float a=0.0;
   int b;
   int c;
  for(int i=0;i<50;i++)
  {
   a+= gauge.readVoltage();//readVoltage()//readPercentage() 
   delay(10);
  }
  b =(int)(a/50);
  //Serial.println(a);
  b =b+offset_voltage;
  c= (b -Votaltage_min)/delta_voltage *100 ;
 // Serial.println(c);

  return c;
}
void sendMessage(byte destination_addr, String message)
 {
  byte data_send[3];
  data_send[0] = destination_addr;
  data_send[1] = nodeFourth_addr;
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
   data_rec[0]=LoRa.read();// Serial.println(data_rec[0]);//addr node 
   data_rec[1]=LoRa.read(); //Serial.println(data_rec[1]);// addr gatt
   data_rec[2]=LoRa.read(); //Serial.println(data_rec[2]); // length cmd
   data_rec[3]=LoRa.read(); //Serial.println(data_rec[3]);// crc
   while (LoRa.available()) LoRaData = LoRa.readString(); Serial.println(LoRaData);       
   if ((data_rec[0] != nodeFourth_addr)|| (data_rec[1]!= gattway_addr )){ Serial.println("not for me."); return ;}                         
   if(data_rec[3] != crc8(data_rec, 3)){ Serial.println("crc faile"); return;}
   if(LoRaData.length() != data_rec[2]){ Serial.println("data length faile"); return;}
    decode_data(data_rec[1],LoRaData);

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
void write_eeprom(int addr,float value)
{
  floatAsBytes.fval = value;
  EEPROM.write(addr, floatAsBytes.bval[0]);
  EEPROM.write(addr+1, floatAsBytes.bval[1]);
  EEPROM.write(addr+2, floatAsBytes.bval[2]);
  EEPROM.write(addr+3, floatAsBytes.bval[3]);
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
void lora_setup()
{
  LoRa.setPins(csPin, resetPin, irqPin);
   if (!LoRa.begin(fre)) {   
    Serial.println("LoRa init failed. Check your connections.");
    while (true);                       
  }
  Serial.println("LoRa init succeeded.");
  LoRa.onReceive(onReceive);
  LoRa.receive();
  LoRa.setTxPower(20);
  LoRa.enableCrc();
}
void decode_data(byte addr,String dta)
{
   String temp = dta.substring(0, 2);
   float dt;
   int pos1,pos2,pos3;
   float kp,ki,kd;
  if(temp == cmdR) 
   {  
    msgsend=(String)volt+"h"+(String)battery_per+"j"+(String)current+"/OK4";
    sendMessage(addr,msgsend); 
   // Serial.println(msgsend);  
   }
   else if(temp == cmdSO) 
   {
   dt = dta.substring(2, dta.length()).toFloat();
   write_eeprom(addrepprom_Oxy,dt);
   msgsend = "OKO";
   sendMessage(addr,msgsend);
   Serial.println(msgsend);
   }
    else if(temp == cmdSPID) 
   {  
    pos1 = dta.indexOf('/');
    pos2 = dta.indexOf('*');
    pos3 = dta.indexOf('+');
    kp = dta.substring(2, pos1).toFloat();
    ki = dta.substring(pos1+1, pos2).toFloat();
    kd = dta.substring(pos2+1, pos3).toFloat();
   // Serial.println("kp:"+(String)kp+"ki:"+(String)ki+"kd:"+(String)kd);
    write_eeprom(addrepprom_kp,kp);
    write_eeprom(addrepprom_ki,ki);
    write_eeprom(addrepprom_kd,kd);
   msgsend = "OKP";
   sendMessage(addr,msgsend);
   Serial.println(msgsend);
   }
}
