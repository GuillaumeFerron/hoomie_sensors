#include <OneWire.h>
#include <DallasTemperature.h>
#include <Bridge.h>
#include <HttpClient.h>

#include <Console.h>

#define ONE_WIRE_BUS 2



OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup(void) {

  sensors.begin();

  Serial.begin(115200);
  Bridge.begin();

  SerialUSB.begin(9600);
  while (!SerialUSB); // wait for a serial connection
  SerialUSB.println("You're Connected");
  /*Console.begin(); //with network 
  while(!Console){
    ;
  }
  Console.println("You're Connected");*/
  // Initialize the client library
  HttpClient client;
}

void loop(void) {
  

  // Make a HTTP request:
  //client.get("http://hoomieserver.herokuapp.com/temperature/year/2017");

  sensors.requestTemperatures();
  SerialUSB.println(sensors.getTempCByIndex(0));
  SerialUSB.flush();
  delay(10000); //wait every 10 sec

}
