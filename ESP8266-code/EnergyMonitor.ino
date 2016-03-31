/*
 *  Uses the ADS1115 ADC in differental mode to measure the voltage across a burden resistor  generated from
 *   a current sensor
 *  
 *  voltage values have be calibrated against a standard 60w bulb 
 *  
 */

#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_ADS1015.h>


// WIFI credentials
const char* ssid     = "BTHub3-M4GH";
const char* password = "86857ef6df";

// Data stream
const char* host = "kitwallace.co.uk";
const char* streamId   = "MS-MQ3";
const char* privateKey = "a7fba854-a7b7-4b79-a73a-eb951def1c51";

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */

WiFiClient client;

// calibration
// simple calibration with a 60w lightbulb  
float multiplier = 0.01;   

void setup() {
  Serial.begin(9600);
  delay(10);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("WiFi connected ");  
  Serial.print("IP address: ");
  Serial.println( WiFi.localIP());

  ads.setGain(GAIN_ONE);       
  ads.begin();
}

void loop() {  
  double current = getIrms(1024);
  String params = String("Current=")+current;
  logData(params);
  Serial.println(params);
}

double getIrms(unsigned int Number_of_Samples)
{
  double sum = 0.0;
  double sum2 = 0.0;
  for (int n = 0; n < Number_of_Samples; n++)
  {
    long sampleI = ads.readADC_Differential_0_1();
    sum += sampleI;
    sum2+= sampleI * sampleI;
  }  
  return sqrt( (sum2 - (sum * sum)/Number_of_Samples) / Number_of_Samples)* multiplier; 
}

void httpGet(String url) {
   WiFiClient client;
   url.replace(" ", "+");
   int httpPort = 80;
   while (true) {
    if (client.connect(host, httpPort)) break;
    Serial.print(".");
    delay(100);
  }

  Serial.print("requesting URL: ");
  Serial.println(url);
// This will send the request to the server
  String httprequest = String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n";
// Serial.println(httprequest);
  client.print(httprequest);
  
// Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
//    Serial.print(line);
  }
  
//  Serial.println();
//  Serial.println("closing connection");
}

void logData(String params) {
   String url = "/rt/home.xq?_action=store&_id=";
   url += streamId;
   url += String("&_pk=")+ privateKey + "&";
   url += params;
   httpGet(url); 
}
