#include <ArduinoJson.h>

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Bridge.h>
#include <SoftwareSerial.h>
#include <RH_RF95.h>
#include <HttpClient.h>
#include <Process.h>
#include <Console.h>
#include <stdio.h>

#define ONE_WIRE_BUS 2
#define SENDING_LED 4
#define MEASURE_LED 3
#define ROOM_NUMBER 205
#define NB_MEASURE_PER_SEND 4
#define DELAY 30000
#define TIME_HEADER  'T'   // Header tag for serial time sync message
#define TIME_REQUEST "req"

Process date;
Process sendData;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


// Singleton instance of the radio driver
SoftwareSerial ss(10, 11);
RH_RF95 rf95(ss);
int room;

void setup(void) {

  sensors.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MEASURE_LED,OUTPUT);
  pinMode(SENDING_LED,OUTPUT);
  Serial.begin(9600);
  Bridge.begin();
  if(!rf95.init())
    {
        Serial.println("init failed");
        while(1);
    } 
 // while (!SerialUSB); // wait for a serial connection
  Serial.print("connected");
  rf95.setFrequency(868.0);
  room = ROOM_NUMBER;
 
  
}

void loop()
{
  //Serial.println("waiting");
  if(rf95.available())
  {
    // Should be a message for us now   
    uint8_t buf[90];
    uint8_t len = sizeof(buf);
    memset(buf,char(0),len);
    //Serial.print(len);
    if(rf95.recv(buf, &len))
    {
        Serial.println((char*)buf);
        
        
        digitalWrite(LED_BUILTIN, HIGH);
        if(buf[0]==(uint8_t)TIME_REQUEST[0]){ 
          uint8_t data[15];
          getTimeNow().toCharArray(data,15);
          Serial.println("Sending time");
          rf95.send(data, sizeof(data));
          rf95.waitPacketSent();
        }else{          
          
          uint8_t data[]="ok" ;
          rf95.send(data, sizeof(data));
          rf95.waitPacketSent();
          Serial.println("Sending a reply");
          receive((char*)buf);
       }   
        digitalWrite(LED_BUILTIN, LOW);
    }
    else
    {
        Serial.println("recv failed");
    }
  }
}

void receive(char b[]) {

  StaticJsonBuffer<250> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  
  JsonArray& json = root["data"];
  
  char* ws[11];
  int index = 0 ;
  char* split = strtok(b,"-");
  while(split != NULL){
    ws[index]=split;
    index += 1;
    split = strtok(NULL,"-");
  }
  
  
  digitalWrite(MEASURE_LED, HIGH);
  json.add(measureTemp());
  Serial.flush();
  digitalWrite(MEASURE_LED, LOW);
  
 
  
  // Make a HTTP request:
    
  char json_to_send[250];
  root.printTo(json_to_send);
  Serial.println(String(json_to_send));
  /*
    if(!sendData.running()){
      digitalWrite(SENDING_LED,HIGH);
      sendData.begin("curl");
      sendData.addParameter("-X");
      sendData.addParameter("POST");
      sendData.addParameter("http://hoomieserver.herokuapp.com/temperature/addDoc");
      sendData.addParameter("-H");
      sendData.addParameter("Content-Type:application/json");
      sendData.addParameter("--data-binary");
      sendData.addParameter(json_to_send);
      Serial.println("sent");
      sendData.run();
      delay(500);
      digitalWrite(SENDING_LED, LOW);
    }

  */
  
  

}

String formJson(int room,long epoch,float value){
  
 
}


JsonObject& measureTemp(){
  StaticJsonBuffer<60> jsonBuffer;
  JsonObject& tempInfo = jsonBuffer.createObject();
  String dateVal;

  date.runShellCommand("echo `date +%Y-%m-%d-%H-%M-%S`");
  // do nothing until the process finishes, so you get the whole output:
  while (date.running());
  //if there's a result from the date process, get it.
  while (date.available()) {
    // print the results we got.
    char c = date.read();
    if(c!='\n') dateVal += c;
  }
  
  tempInfo["date"] = dateVal;
  sensors.requestTemperatures();
  char tempVal[5];
  dtostrf(sensors.getTempCByIndex(0),5,2,tempVal);
  String tempValString ;
  for (int i=0;i<5;i++){
    tempValString += tempVal[i];
  }
  tempInfo["value"] = tempValString;
  tempInfo["room"] = room;
  tempInfo.printTo(Serial);
  return tempInfo;
  
}


String  getTimeNow(){
  Serial.println("getting date");
  String dateVal ;
  dateVal += TIME_HEADER;
  // java time is in ms, we want secs    
  date.runShellCommand("echo `date +%s`");
  //Serial.println(date);
 // do nothing until the process finishes, so you get the whole output:
  while (date.running());
  //if there's a result from the date process, get it.
  //Serial.println(date.readString());
  while (date.available()) {
    // print the results we got.
    char c = date.read();
    if(c!='\n') dateVal += c;
  }
  date.runShellCommand("echo `date +%Y-%m-%d-%H-%M-%S`");
  Serial.println(date.readString());
  Serial.println(dateVal);
  return dateVal; 
}

