//#include <MutichannelGasSensor.h>

#include <Time.h>
#include <TimeLib.h>

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
#define NB_MEASURE_PER_SEND 2
#define NB_FIELD_PER_MEASURE 4
#define LOCAL_TIME_DIFFERENCE 3600
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

  int buffsize = 1+NB_MEASURE_PER_SEND*NB_FIELD_PER_MEASURE ; //room + NB_MEASURE_PER_SEND*(time+temp+co+no2)
  char* ws[buffsize];
  int index = 0 ;
  char* split = strtok(b,"-");
  while(split != NULL){
    ws[index]=split;
    index += 1;
    split = strtok(NULL,"-");
  }

  
  int roomRcv = atoi(ws[0]);
  boolean missingInfo = false;
  long etime = 0;
  char* temp = '0';
  char* co = '0';
  for(int i=1;i<buffsize;i++){
    switch(i % NB_FIELD_PER_MEASURE){
      case 1:
        if(strlen(ws[i]) == 10){
          etime = atol(ws[i])+ LOCAL_TIME_DIFFERENCE;//for local time    
        }
        else{
          missingInfo = true;
        }
        break;
     case 2:
        if(strlen(ws[i]) == 5 && isNumeric(ws[i]) && !missingInfo){
          Serial.println("temp is numeric");
           temp = ws[i];
           String json = "{\"data\":["+formJson1Value(roomRcv,etime,temp)+"]}"; 

           // Make a HTTP request  
            char json_to_send[80];
            json.toCharArray(json_to_send,sizeof(json_to_send));
            Serial.println(String(json_to_send));
            Serial.flush();
            sendToServer(json_to_send,"addTemp");
        }
        else{
          missingInfo = true;
        }
        break;
     case 3:
        if(isNumeric(ws[i]) && !missingInfo){
          co = ws[i];   
        }
        else{
          missingInfo = true;
        }
        break;
      case 0:
        if(isNumeric(ws[i])&& !missingInfo){
           String json = "{\"data\":["+formJson2Values(roomRcv,etime,co,ws[i])+"]}"; 

           // Make a HTTP request  
            char json_to_send[90];
            json.toCharArray(json_to_send,sizeof(json_to_send));
            Serial.println(String(json_to_send));
            Serial.flush();
            sendToServer(json_to_send,"addAtmos");
              
            etime = 0;  
        }
        else{
          missingInfo = true;
        }
        break;
        
    }
  }
       
      
  digitalWrite(MEASURE_LED, HIGH);
  String json = "{\"data\":["+measureTemp()+"]}"; 
  digitalWrite(MEASURE_LED, LOW);

 // Make a HTTP request  
  char json_to_send[80];
  json.toCharArray(json_to_send,sizeof(json_to_send));
  Serial.println(String(json_to_send));
  Serial.flush();
  sendToServer(json_to_send,"addTemp");
  
  
}


void sendToServer(char json_to_send[],String route){
  if(!sendData.running()){
      digitalWrite(SENDING_LED,HIGH);
      sendData.begin("curl");
      sendData.addParameter("-X");
      sendData.addParameter("POST");
      sendData.addParameter("http://hoomieserver.herokuapp.com/sensors/"+route);
      sendData.addParameter("-H");
      sendData.addParameter("Content-Type:application/json");
      sendData.addParameter("--data");
      sendData.addParameter(json_to_send);
      sendData.run();
      Serial.println("data sent to server");
      delay(500);
      digitalWrite(SENDING_LED, LOW);
    }
}


String formJson1Value(int room,long epoch,char* temp){
  String tempInfo;
  time_t t = (time_t) epoch;
  String date= checkDigit(year(t))+'-'+checkDigit(month(t))+'-'+checkDigit(day(t))+'-'+checkDigit(hour(t))+'-'+checkDigit(minute(t))+'-'+checkDigit(second(t));
  tempInfo = "{\"date\":\""+date+"\",";
  tempInfo +=  "\"value\":"+String(temp)+",\"room\":"+room+"}";
  return tempInfo;
}


String formJson2Values(int room,long epoch,char* co,char* no2){
  String tempInfo;
  time_t t = (time_t) epoch;
  String date= checkDigit(year(t))+'-'+checkDigit(month(t))+'-'+checkDigit(day(t))+'-'+checkDigit(hour(t))+'-'+checkDigit(minute(t))+'-'+checkDigit(second(t));
  tempInfo = "{\"date\":\""+date+"\",";
  float no2Val = atoi(no2)*0.01;
  Serial.println(no2Val);
  tempInfo +=  "\"co\":"+String(co)+",\"no2\":"+no2Val+",\"room\":"+room+"}";
  //Serial.println(tempInfo);
  return tempInfo;
}


String measureTemp(){
  String dateVal,tempInfo;

  date.runShellCommand("echo `date +%Y-%m-%d-%H-%M-%S`");
  // do nothing until the process finishes, so you get the whole output:
  while (date.running());
  //if there's a result from the date process, get it.
  while (date.available()) {
    // print the results we got.
    char c = date.read();
    if(c!='\n') dateVal += c;
  }
  
  tempInfo = "{\"date\":\""+dateVal+"\",";
  sensors.requestTemperatures();
  char tempVal[5];
  dtostrf(sensors.getTempCByIndex(0),5,2,tempVal);
  tempInfo +=  "\"value\":"+String(tempVal)+",\"room\":"+room+"}";
  return tempInfo;
  
  
}

String checkDigit(int d){
  String s;
  if(d < 10)
    s = "0";
  return s+String(d);
  
}

boolean isNumeric(String str) {
    unsigned int stringLength = str.length();
 
    if (stringLength == 0) {
        return false;
    }
 
    boolean seenDecimal = false;
 
    for(unsigned int i = 0; i < stringLength; ++i) {
        if (isDigit(str.charAt(i))) {
            continue;
        }
 
        if (str.charAt(i) == '.') {
            if (seenDecimal) {
                return false;
            }
            seenDecimal = true;
            continue;
        }
        return false;
    }
    return true;
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

