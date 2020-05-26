/*
 Basic MQTT example

 This sketch demonstrates the basic capabilities of the library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 
*/

#define UPLOAD_THINGSPEAK 1
#include <LWiFi.h>
#include <PubSubClient.h>

#include "DHT.h"
#include <stdlib.h>
#define DHTPIN A0
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define UPLOAD_PERIOD 5000
#define USR_UPLOAD_PERIOD 1000     

unsigned long current_time = 0;
unsigned long last_time_upload = 0;
unsigned long usr_current_time = 0;
unsigned long usr_last_time_upload = 0;
bool flags = false;
bool change = false;

char ssid[] = "3715";      //  your network SSID (name)
char pass[] = "12345678";  // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;               // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;

// Update these with values suitable for your network.
//byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress server(192, 168, 251, 51);

String data_json = "";
char data_str[50];

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println();
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  if(topic == "IoT/10603030a/USR"){
      int count = 0;   //get payload length
  
      for (int i=0;i<length;i++) {
        Serial.print((char)payload[i]);
        if(payload[i]!='\0' ){
          count++;
        }
      }
      if((char)payload[0] == '0' && count == 1){
         //Serial.print((char)payload[0]);
         digitalWrite(LED_BUILTIN, LOW);
         flags = false;
      }
      else if((char)payload[0] == '1' && count == 1){
         //erial.print((char)payload[0]);
         digitalWrite(LED_BUILTIN, HIGH);
         flags = false;
      }
      else if((char)payload[0] == '2' && count == 1){
         flags = true;
      }
      else{
         Serial.println();
         Serial.println("Please enter the number : 0, 1, 2");
      }
      Serial.println();
      }

  

}

WiFiClient ethClient;
PubSubClient client(ethClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("10603030A")) {

      Serial.println("connected");
      // ... and resubscribe
      client.subscribe("IoT/10603030a/USR");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(57600);
  dht.begin();

  pinMode(LED_BUILTIN, OUTPUT);
  //digitalWrite(LED_BUILTIN, HIGH);
  
  client.setServer(server, 1883);
  client.setCallback(callback);

  //Ethernet.begin(mac, ip);
  // Allow the hardware to sort itself out
 //delay(1500);
 
 // attempt to connect to Wifi network:
    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(ssid, pass);
    }
    Serial.println("Connected to wifi");
    printWifiStatus();
}

void loop()
{
    // check if returns are valid, if they are NaN (not a number) then something went wrong!
    if (!client.connected()) {
      reconnect();
    }
    client.loop();

    float h = dht.readHumidity();
    float t = dht.readTemperature();
      
    String temp = String(t);
    String humi = String(h);

    current_time = millis();
    if(UPLOAD_PERIOD < (current_time - last_time_upload)) {
    
      char Temp[100];
      char Humi[100];
      Serial.println();
      
      sprintf(Temp, "Temp:%s", String(t,2).c_str());
      sprintf(Humi, "Hum:%s", String(h,2).c_str());
      
      Serial.println(Temp);
      Serial.println(Humi);  

      data_json = "{\"temp\": " + String(Temp) + ", \"hum\": " + String(Humi) + "}";
      data_json.toCharArray(data_str, 50);
      
      client.publish("sensor/room", data_str);
      data_json = "";

      last_time_upload = millis();
    } 
    
    usr_current_time = millis();
    
    if(USR_UPLOAD_PERIOD < (usr_current_time - usr_last_time_upload)) {
      if(flags == true){
        if(change == false){
          digitalWrite(LED_BUILTIN, HIGH); 
          change = true;
        }
        else{
          digitalWrite(LED_BUILTIN, LOW);
          change = false;
        }
      }
      usr_last_time_upload = millis();
    }       
}

void printWifiStatus() {
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
}
