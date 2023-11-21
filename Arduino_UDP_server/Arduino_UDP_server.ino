// C++ code
//

#include <WiFi.h>
#include <WiFiUdp.h>

WiFiUDP Udp;
char packetBuffer [255];
unsigned int localPort = 9999;
const char *ssid = "ESP32_for_IMU";
const char *password = "ICSESP32IMU";

void setup ()
{
  Serial.begin(512000);
  WiFi.softAP(ssid, password);
  Udp.begin(localPort);
  Serial.print(WiFi.softAPIP());
}

void loop ()
{
  //Serial.print(WiFi.softAPIP());
  int packetSize = Udp.parsePacket();

  if (packetSize) {
    int len = Udp.read(packetBuffer, 255);

    Serial.print("Raw: ");
    Serial.println(packetBuffer);
    
    if (len > 0) packetBuffer[len-1] = 0;

 //   int receiving_var = packetBuffer - '0'; //line to convert character into integer value

 //  switch (receiving_var){ //switch statement which determines what sort of vibration pattern occurs depending on sign that was collected
 //     case 0:
   //     Serial.print("zero");

     // case 1:
       // Serial.print("one");
      
     // case 2:
       //Serial.print("two"); 
 //   }

    Serial.print("Receiving: ");
    Serial.println(packetBuffer);
    delay(1000);
  }
}
