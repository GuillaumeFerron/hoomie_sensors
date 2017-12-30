//#include <ArduinoJson.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <TimeLib.h>
#include <Console.h>
#include <stdio.h>

#include <SoftwareSerial.h>
#include <RH_RF95.h>

#define ONE_WIRE_BUS 2
#define SENDING_LED 4
#define MEASURE_LED 3
#define LORA_TX 5 
#define LORA_RX 6

#define ROOM_NUMBER 204
#define NB_MEASURE_PER_SEND 5 //careful need to change size of json_to_send depending on nb measure
#define DELAY 20000 //1 min 
#define TIME_HEADER  'T'   // Header tag for serial time sync message
#define TIME_REQUEST "req"  


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
// Singleton instance of the radio driver
SoftwareSerial ss(5, 6);
RH_RF95 rf95(ss);

boolean dateSet;
int room;

void setup(void) {
 // init temp sensors, led
  sensors.begin();
  pinMode(MEASURE_LED, OUTPUT);
  pinMode(SENDING_LED,OUTPUT);
  

  Serial.begin(9600);
  //while (!Serial); // wait for a serial connection
   if (!rf95.init())
    {
        Serial.println("init failed");
        while(1);
    }
  Serial.println("You're Connected");
  setSyncProvider( requestSync);  //set function to call when sync required
  Serial.println("Waiting for sync message");
  rf95.setFrequency(868.0);
  room = ROOM_NUMBER;
  dateSet = false;
  
  
  while(!dateSet){
    if (rf95.available()) {
      processSyncMessage();
    }
    if (timeStatus() == timeSet) {
     for(int i =0;i<2;i++){ 
        digitalWrite(MEASURE_LED, HIGH); // LED on if synced
        digitalWrite(SENDING_LED, HIGH); // LED on if synced
        delay(500);
        digitalWrite(MEASURE_LED,LOW);
        digitalWrite(SENDING_LED,LOW);
        delay(500);
     }
      dateSet = true;
    } else {
      //requestSync();
      digitalWrite(MEASURE_LED, LOW);  // LED off if needs refresh
    }
    delay(1000);
  }
}

void loop(void) {
   int json_size = 3+NB_MEASURE_PER_SEND*(17);
   char json_to_send[json_size]; //size = 3+nb_measure*(16+1)+1 or 2 more
   sprintf(json_to_send,"%d",room);
   int nb_measure = 0;
  
  // Make a HTTP request:
  while( nb_measure < NB_MEASURE_PER_SEND ){
    digitalWrite(MEASURE_LED, HIGH);
    sprintf(json_to_send,"%s-%s",json_to_send,measureTemp());
    Serial.flush();
    digitalWrite(MEASURE_LED, LOW);
    if(nb_measure != NB_MEASURE_PER_SEND-1){
      delay(DELAY); //wait every 30 sec 
    }
    
    nb_measure +=1;
  }
  
 Serial.println(String(json_to_send));
 rf95.send((uint8_t*)json_to_send,sizeof(json_to_send));
 rf95.waitPacketSent();
 digitalWrite(SENDING_LED,HIGH);
    
    // Now wait for a reply
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    
  if(rf95.waitAvailableTimeout(5000))
    {
        
        // Should be a reply message for us now   
        if(rf95.recv(buf, &len))
        {
            Serial.print("got reply: ");
            Serial.println((char*)buf);
            digitalWrite(SENDING_LED,LOW);
        }
        else
        {
            Serial.println("recv failed");
            radio_error();
        }
    }
    else
    {
        Serial.println("No reply, is rf95_server running?");
        radio_error();
    }
    
    delay(DELAY);

}

void radio_error(){
   for(int i =0;i<5;i++){ 
        digitalWrite(SENDING_LED, HIGH); // LED on if synced
        delay(500);
        digitalWrite(SENDING_LED,LOW);
        delay(500);
     }
}

char* measureTemp(){
  char tempInfo[16]; 
  //date 
  time_t t = now(); 
  Serial.println(t);
  //temperature 
  sensors.requestTemperatures();
  char tempVal[5];
  dtostrf(sensors.getTempCByIndex(0),5,2,tempVal);
  int n = sprintf(tempInfo,"%ld-%s",(long)t,tempVal);
  Serial.println(n);
  return tempInfo;
  
}


String checkDigit(int d){
  String s;
  if(d < 10)
    s = "0";
  return s+String(d);
  
}


void processSyncMessage() {
  unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1513581421; // Dec 18 2017
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
    
  if(rf95.recv(buf, &len)){
    Serial.println((char*)buf);
    if((char)buf[0]==TIME_HEADER){
     String buffer = String((char*)buf).substring(1);
     Serial.println(buffer);
     
     pctime = buffer.toInt();
     if( pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Dec 18 2017)
       setTime(pctime); // Sync Arduino clock to the time received on the serial port
       Serial.println("setting time");
     }
     else{
      requestSync();
     }
    }
    else{
      Serial.println("error on time");
      requestSync();
    }
  }
  else{
    Serial.println("recv failed");
  }
}

time_t requestSync()
{
  Serial.println("request sent");
  uint8_t req[]= TIME_REQUEST;
  rf95.send(req,sizeof(req)); 
  rf95.waitPacketSent(); 
  return 0; // the time will be sent later in response to serial mesg
}

