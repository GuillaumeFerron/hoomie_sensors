import processing.net.*;

PFont font;
Client client;
String temp1="0";

void setup() {
  client = new Client (this,"192.168.240.1",6666);
  client.write('\n');

  size(310,200);
  strokeWeight(5);
  stroke(255,100);

  textAlign(LEFT);
  textFont(createFont("Georgie",36));
  
}

void draw() {
  background(0);

  tmp1 = client.readStringUntil('\n');

  text("temp = "+temp1,55,100);
  print(temp1);
  delay(1000);
  client.write('\n');
}
