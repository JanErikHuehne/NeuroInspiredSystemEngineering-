// C++ code
//

#include <WiFi.h>
#include <WiFiUdp.h>

int motorPin1 = 13; // GPIO 13 for vibration motor 1
int motorPin2 = 12; // GPIO 12 for vibration motor 2

WiFiUDP Udp;
char packetBuffer [255];
unsigned int localPort = 9999;
const char *ssid = "ESP32_for_IMU";
const char *password = "ICSESP32IMU";

int convertCharArrayToInt(const char *arr, int length) {
    int var = 0;
    
    for (int i = 0; i < length; i++) {
        int num = arr[i] - '0'; // Convert char to int
        var = var * 10 + num; // Combine all the numbers into a single integer
    }
    return var;
}



void setup ()
{

  pinMode ( motorPin1 , OUTPUT );
  pinMode ( motorPin2 , OUTPUT );

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

    int receiving_var = convertCharArrayToInt(packetBuffer, len); //convert char to int

  //  if (len > 0) packetBuffer[len-1] = 0;
    
    switch (receiving_var){ //switch statement which determines what sort of vibration pattern occurs depending on sign that was collected (e.g. using binary mode for two motors, should extend to three)
      case 0:
        Serial.print("zero");
        digitalWrite ( motorPin1 , HIGH );
        digitalWrite ( motorPin2 , HIGH );
        delay (1000);
        digitalWrite ( motorPin1 , LOW );
        digitalWrite ( motorPin2 , LOW );
        delay (1000);    
      case 1:
        Serial.print("one");
        break;
      case 2:
        Serial.print("two"); 
        break;
      case 3:
        Serial.print("three");
        break;
      case 4:
        Serial.print("four");
        break;
      case 5:
        Serial.print("five"); 
        break;
      case 6:
        Serial.print("six");
        break;
      case 7:
        Serial.print("seven"); 
        break;
      case 8:
        Serial.print("eight");
        break;
      case 9:
        Serial.print("nine");
        break;
    }

    Serial.print("Receiving: ");
    Serial.println(packetBuffer);
    delay(1000);
  }
}
