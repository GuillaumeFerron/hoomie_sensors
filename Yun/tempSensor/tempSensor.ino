#include <OneWire.h>
#include <DallasTemperature.h>
#include <Bridge.h>
#include <HttpClient.h>
#include <Process.h>
#include <Console.h>
#include <stdio.h>

#define ONE_WIRE_BUS 2
#define ROOM_NUMBER 205
#define NB_MEASURE_PER_SEND 4
#define DELAY 30000

Process date;
Process sendData;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
// Initialize the client library
HttpClient client;
int room;

void setup(void) {

  sensors.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Bridge.begin();

  SerialUSB.begin(9600);
  while (!SerialUSB); // wait for a serial connection
  SerialUSB.println("You're Connected");
  room = ROOM_NUMBER;
 
  
}

void loop(void) {
  
  int nb_measure = 0;
  String json="[";
  // Make a HTTP request:
  while( nb_measure < NB_MEASURE_PER_SEND ){
    digitalWrite(LED_BUILTIN, HIGH);
    String tempDoc = measureTemp();
    SerialUSB.println(tempDoc);
    json += tempDoc;
    SerialUSB.flush();
    digitalWrite(LED_BUILTIN, LOW);
    if(nb_measure != NB_MEASURE_PER_SEND-1){
      json += ",";
      delay(DELAY); //wait every 30 sec 
    }
    
    nb_measure +=1;
  }
  json += "]";
  
  String json_to_send = "{\"data\":"+json+"}";
  SerialUSB.println(json_to_send);
  
  if(!sendData.running()){
    sendData.begin("curl");
    sendData.addParameter("-X");
    sendData.addParameter("POST");
    sendData.addParameter("http://hoomieserver.herokuapp.com/temperature/addDoc");
    sendData.addParameter("-H");
    sendData.addParameter("Content-Type:application/json");
    sendData.addParameter("--data-binary");
    sendData.addParameter(json_to_send);
    SerialUSB.println("sent");
    sendData.run();
  }
  

}

String measureTemp(){
  String tempInfo,dateVal;

  date.runShellCommand("echo `date +%Y-%m-%d-%H-%M-%S`");
  // do nothing until the process finishes, so you get the whole output:
  while (date.running());
  //if there's a result from the date process, get it.
  while (date.available()) {
    // print the results we got.
    char c = date.read();
    if(c!='\n') dateVal += c;
  }
  
  tempInfo = "{\"date\": \""+dateVal+"\",";
  sensors.requestTemperatures();
  char tempVal[5];
  dtostrf(sensors.getTempCByIndex(0),5,2,tempVal);
  String tempValString ;
  for (int i=0;i<5;i++){
    tempValString += tempVal[i];
  }
  tempInfo +=  "\"value\": "+tempValString+", \"room\": "+room+"}";
  
  return tempInfo;
  
}


