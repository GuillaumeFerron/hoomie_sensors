#include <OneWire.h>
#include <DallasTemperature.h>
#include <Bridge.h>
#include <HttpClient.h>
#include <Process.h>
#include <Console.h>

#define ONE_WIRE_BUS 2


Process date;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
// Initialize the client library
HttpClient client;
int room;

void setup(void) {

  sensors.begin();

  Serial.begin(115200);
  Bridge.begin();

  SerialUSB.begin(9600);
  while (!SerialUSB); // wait for a serial connection
  SerialUSB.println("You're Connected");
  room = 205;
  /*Console.begin(); //with network 
  while(!Console){
    ;
  }
  Console.println("You're Connected");*/
  
}

void loop(void) {
  

  // Make a HTTP request:
  //client.get("http://hoomieserver.herokuapp.com/temperature/year/2017");

  
  
  SerialUSB.println(measureTemp());
  /*while (client.available()) {
    char c = client.read();
    SerialUSB.print(c);
    
  }
  SerialUSB.print("\n");*/
  SerialUSB.flush();
  delay(3000); //wait every 10 sec 

}

float measureTemp(){
  date.runShellCommand("echo `date +%Y-%m-%d-%H-%M-%S`");
  // do nothing until the process finishes, so you get the whole output:
  while (date.running());
  //if there's a result from the date process, get it.
  while (date.available()>0) {
    // print the results we got.
    SerialUSB.println(date.readString());
  }
  
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
  
}


