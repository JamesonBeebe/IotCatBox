//void app_main() {}
// IoT Cat Litter Box V0.0 by Jameson Beebe
// Measures Cat weight and frequency of use for multiple cats

/************************ Thingspeak  Configuration ************************/

// visit thingspeak.com if you need to create an account or if you need your write key.
#include "ThingSpeak.h"
#include "secrets.h"
unsigned long myChannelNumber = SECRET_CH_ID;
const char *apiKey = SECRET_WRITE_APIKEY;
const char *readAPIKey = SECRET_READ_APIKEY;
const char *server = "api.thingspeak.com"; //   "184.106.153.149" or api.thingspeak.com

/************************ WIFI Configuration ************************/

#include <ESP8266WiFi.h>        // Wi-Fi
#include <WiFiClient.h>         // Wi-Fi client
const char *ssid = SECRET_SSID; // replace with your wifi ssid and wpa2 key
const char *pass = SECRET_PASS;
WiFiClient client;
#define WIFI_TIMEOUT 20 //Wi-Fi Timeout for local connections (multiply this number by 500mS)

/************************ OTA  Configuration ************************/
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
AsyncWebServer otaServer(1234);

/************************ GUI *********************************/
#include "index.h"
AsyncWebServer gui(80);

/************************ Define IO pins ************************/

#include "HX711.h"
#define calibration_factor -22994.0 //This value is obtained using the SparkFun_HX711_Calibration sketch
#define DOUT 0                      // Pin connected to HX711 data output pin
#define CLK 12                      // Pin connected to HX711 clk pin
HX711 scale;                        // Scale initialization

#define NUM_MEASUREMENTS 10 // Number of measurements
#define THRESHOLD 0.2       // Restart averaging if the weight changes more than 0.2 kg.

float weight = 0;
float sandWeight = 0;
float platformWeight = 1.8; // weight of the platform in kg
float weightbox = 0.7;      // Weight of empty box

float cat1MinWeight = 2;   // expected minimum weight for cat #1
float cat1MaxWeight = 6;   // expected maximum weight for cat #1
float cat2MinWeight = 6.3; // expected minimum weight for cat #2

float cat1MinWeightLbs = 8;    // expected minimum weight for cat #1
float cat1MaxWeightLbs = 13;   // expected maximum weight for cat #1 in Lbs
float cat2MinWeightLbs = 13.5; // expected minimum weight for cat #2 in Lbs

long cat1Uses = 0;   // number of uses of the box in a day
long cat2Uses = 0;   // number of uses of the box in a day
float soilLevel = 0; // track soil level of box

#define reed_switch_PIN 13
int reed_switch_value = 1;
int prev_reed_switch_value = 1;

/************************ Clock settings ************************/

String TimeDate = ""; // Variable used to store the data received from the server
int hours;            // Current hour
int minutes;          // Current minute
int seconds;          // Current second
int seconds0;         // Last time the clock was updated
#define TIMEZONE 4    // Current time zone. Used for correction of server time

/********************** Local Defines ***************************/
//#define __local_wifi__

String readWeight(void);
String processor(const String &var);
void zeroScale(void);
void restartEsp(void);
void resetScale(void);

void setup()
{

  // Start the serial connection
  Serial.begin(115200);
  Serial.println("Testing serial communication...");

  pinMode(reed_switch_PIN, INPUT_PULLUP);

  scale.begin(DOUT, CLK); // Scale initialization
  // Scale calibration
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch

  // Connect to WiFi router
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  int count = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if (count == WIFI_TIMEOUT)
      break; //jump out of loop if local timeout hit to start SmartConfig
    count++;
  }

  if (WiFi.status() != !WL_CONNECTED)
  { //Local Auto-connect did not happen, start WiFi SmartConfig
    WiFi.mode(WIFI_AP_STA);
    // Start WiFi Smart Config
    WiFi.beginSmartConfig();

    // Wait for SmartConfig packet from mobile
    Serial.println("Waiting for SmartConfig.");
    while (WiFi.smartConfigDone())
    {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("SmartConfig done.");

    /* Wait for WiFi to connect to AP */
    Serial.println("Waiting for WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
  }
  Serial.println("WiFi Connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  //Start OTA
  otaServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(200, "text/plain", "Hi! I am ESP8266."); });

  AsyncElegantOTA.begin(&otaServer); // Start ElegantOTA
  otaServer.begin();
  Serial.println("HTTP OTA server started");

  //Start GUI
  gui.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
         { request->send_P(200, "text/html", index_html, processor); });
  gui.on("/zero", HTTP_GET, [](AsyncWebServerRequest *request)
         { zeroScale(); });
  gui.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request)
         { restartEsp(); });
  gui.on("/resetscale", HTTP_GET, [](AsyncWebServerRequest *request)
         { resetScale(); });
  gui.begin();

  //Start ThingSpeak interface
  ThingSpeak.begin(client); // Initialize ThingSpeak

  // Read saved values
  cat1Uses = ThingSpeak.readLongField(myChannelNumber, 5, readAPIKey);
  cat2Uses = ThingSpeak.readLongField(myChannelNumber, 6, readAPIKey);
  sandWeight = ThingSpeak.readLongField(myChannelNumber, 4, readAPIKey);

  // prime the read Reed Switch
  prev_reed_switch_value = digitalRead(reed_switch_PIN);
  if (prev_reed_switch_value == 0)
  {
    ThingSpeak.setField(3, 1);
  }
  else
  {
    ThingSpeak.setField(3, 0);
  }
}

void loop()
{
  getTime(); // get current time

  // restart daily parameters
  if ((hours == 0) && (minutes == 0))
  {
    cat1Uses = 0;
    ThingSpeak.setField(5, cat1Uses);
    cat2Uses = 0;
    ThingSpeak.setField(6, cat2Uses);
  }

  // prepare to updated status of the box on Thingspeak if lid changed

  // read Reed Switch
  reed_switch_value = digitalRead(reed_switch_PIN);

  if (reed_switch_value == prev_reed_switch_value)
  {
    //do nothing
  }
  else if (reed_switch_value == 0)
  {
    ThingSpeak.setField(3, 1);
  }
  else
  {
    ThingSpeak.setField(3, 0);
  }

  /*
  if((reed_switch_value==1) && (prev_reed_switch_value != reed_switch_value)){
    ThingSpeak.setField(3, 0);
  }
  else if((reed_switch_value==0) && (prev_reed_switch_value!= reed_switch_value)){
    ThingSpeak.setField(3, 1);
  }
*/
  // check if something changed
  if ((reed_switch_value == 1) && (prev_reed_switch_value == 1))
  {
    Serial.println("Cleaning mode"); // On cleaning mode the controller will stop measuring the weight (do nothing)
  }
  if ((reed_switch_value == 0) && (prev_reed_switch_value == 1))
  {
    Serial.println("Leaving cleaning mode"); // When it leaves cleaning mode, it shall wait some seconds and measure the weight of sand in the litter box
    delay(5000);
    float weight0 = scale.get_units() - weightbox - platformWeight;
    float avgweight = 0;
    for (int i = 0; i <= NUM_MEASUREMENTS - 1; i++)
    { // Takes several measurements
      weight = scale.get_units() - weightbox - platformWeight;
      delay(100);
      avgweight += weight;
      if (abs(weight - weight0) > THRESHOLD)
      {
        avgweight = 0;
        i = 0;
      }
      weight0 = weight;
    }
    sandWeight = avgweight / NUM_MEASUREMENTS;
    Serial.print("Measured sand weight: ");
    Serial.print(sandWeight, 2);
    Serial.println(" lbs");
    ThingSpeak.setField(4, sandWeight);

    //reset soil level after cleaning
    soilLevel = 0;
    ThingSpeak.setField(7, soilLevel);
  }
  if ((reed_switch_value == 0) && (prev_reed_switch_value == 0))
  {
    Serial.println("Running mode"); // On running mode the controller will measure the weight of the cats
    weight = scale.get_units() - weightbox - platformWeight - sandWeight;

    if (weight > cat1MinWeight)
    { // cat detected
      Serial.println("Cat detected!");
      float weight0 = scale.get_units() - weightbox - platformWeight - sandWeight;
      float avgweight = 0;
      for (int i = 0; i <= NUM_MEASUREMENTS; i++)
      { // Takes several measurements
        weight = scale.get_units() - weightbox - platformWeight - sandWeight;
        delay(100);
        avgweight += weight;
        if (abs(weight - weight0) > THRESHOLD)
        {
          avgweight = 0;
          i = 0;
        }
        weight0 = weight;
      }
      avgweight = avgweight / NUM_MEASUREMENTS; // Calculate average weight kg
      avgweight = avgweight * 2.2;              //convert to lbs
      Serial.print("Measured weight: ");
      Serial.print(avgweight, 2);
      Serial.println(" lbs");
      if ((avgweight >= cat1MinWeightLbs) && (avgweight <= cat1MaxWeightLbs))
      { // cat #1 detected
        Serial.println("Cat #1");
        ThingSpeak.setField(1, avgweight); // set measured value to cat #1
        cat1Uses += 1;
        ThingSpeak.setField(5, cat1Uses);
      }
      if (avgweight >= cat2MinWeightLbs)
      { // cat #2 detected
        Serial.println("Cat #2");
        ThingSpeak.setField(2, avgweight); // set measured value to cat #2
        cat2Uses += 1;
        ThingSpeak.setField(6, cat2Uses);
      }

      // wait while there's someone on the scale
      while ((scale.get_units() - weightbox - platformWeight - sandWeight) > cat1MinWeight)
      {
        delay(1000);
      }
      //update the level of soil in the box
      soilLevel = ((scale.get_units() - weightbox - platformWeight - sandWeight) * 2.2);
      ThingSpeak.setField(7, soilLevel);
    }
  }

  if ((reed_switch_value == 1) && (prev_reed_switch_value == 0))
  {
    Serial.println("Leaving running mode"); // When it leaves running mode, it will stop measurint the weight (do nothing)
  }

  // save values do Thingspeak
  if (client.connect(server, 80))
  {
    int x = ThingSpeak.writeFields(myChannelNumber, apiKey);
    switch (x)
    {
    case 200:
      Serial.println("Success!");
      break;

    case 404:
      Serial.println("Incorrect API key or invalid ThingSpeak server address.");

    case -301:
      Serial.println("Failed to connect to ThingSpeak.");

    case -304:
      Serial.println("Timeout waiting for server to respond.");

    default:
      break;
    }
  }
  client.stop();
  prev_reed_switch_value = reed_switch_value; // prepare for next scan

  Serial.println("Waiting...");
  delay(5000);
}

//Get current time Google server
void getTime()
{

  // connect server
  WiFiClient client;
  while (!!!client.connect("google.com", 80))
  {
    Serial.println("connection failed, retrying...");
  }
  client.print("HEAD / HTTP/1.1\r\n\r\n");

  delay(500); //wait a few milisseconds for incoming message

  //if there is an incoming message
  if (client.available())
  {
    while (client.available())
    {
      if (client.read() == '\n')
      {
        if (client.read() == 'D')
        {
          if (client.read() == 'a')
          {
            if (client.read() == 't')
            {
              if (client.read() == 'e')
              {
                if (client.read() == ':')
                {
                  client.read();
                  String theDate = client.readStringUntil('\r');
                  client.stop();

                  TimeDate = theDate.substring(7);
                  String strCurrentHour = theDate.substring(17, 19);
                  String strCurrentMinute = theDate.substring(20, 22);
                  String strCurrentSecond = theDate.substring(23, 25);
                  // store received data on global variables
                  hours = strCurrentHour.toInt();
                  hours = hours - TIMEZONE; // compensate for TIMEZONE
                  if (hours < 0)
                  {
                    hours = hours + 24;
                  }
                  minutes = strCurrentMinute.toInt();
                  seconds = strCurrentSecond.toInt();
                }
              }
            }
          }
        }
      }
    }
  }
  //if no message was received (an issue with the Wi-Fi connection, for instance)
  else
  {
    seconds = 0;
    minutes += 1;
    if (minutes > 59)
    {
      minutes = 0;
      hours += 1;
      if (hours > 23)
      {
        hours = 0;
      }
    }
  }

  // serial output current time
  Serial.print("Current time: ");
  if (hours < 10)
  {
    Serial.print("0");
  }
  Serial.print(hours);
  Serial.print(":");
  if (minutes < 10)
  {
    Serial.print("0");
  }
  Serial.println(minutes);
}

String readWeight(void)
{
  float tempWeight = scale.get_units();
  float tempSandWeight = 0;
  float tempWeightBox = 0;
  float tempPlatformWeight = 0;


if(tempWeight>=platformWeight){
  tempPlatformWeight = platformWeight;
  
  if(tempWeight>=(platformWeight+weightbox)){
    tempWeightBox = weightbox;

    if(tempWeight>=(platformWeight+weightbox+5))
    tempSandWeight = sandWeight;
  }
}
  Serial.print("Platform weight: ");
  Serial.println(tempPlatformWeight);
  Serial.print("Box weight: ");
  Serial.println(tempWeightBox);
  Serial.print("Litter weight: ");
  Serial.println(tempSandWeight);
  
  Serial.print("Compensated weight: ");
  tempWeight = (tempWeight - tempSandWeight - tempWeightBox - tempPlatformWeight)*2.205;
  Serial.println((tempWeight));

    
/*
  //is there litter?
  Serial.print("Litter weight: ");
  if(tempWeight<(platformWeight+weightbox+1)){
    tempSandWeight = 0;
  }
  else{
    tempSandWeight = sandWeight;
  }
  Serial.println(tempSandWeight);

  //is there a box?
  Serial.print("Box weight: ");
  if((platformWeight + weightbox)>tempWeight>=platformWeight){
    tempWeightBox = 0;
  }
  else {
    tempWeightBox = weightbox;
  }
  Serial.println(tempWeightBox);

  //is there a platform?
  Serial.print("Platform Weight: ");
  if(tempWeight<platformWeight){
    tempPlatformWeight = 0;
  }
  else{
    tempPlatformWeight = platformWeight;
  }
  Serial.println(tempPlatformWeight);

  Serial.print("compensated weight: ");
  tempWeight = tempWeight - tempSandWeight - tempWeightBox - tempPlatformWeight;
  Serial.println((tempWeight));
*/
  return String(tempWeight);
}

// Replaces placeholder with DHT values
String processor(const String &var)
{
  //Serial.println(var);
  if (var == "WEIGHT")
  {
    return readWeight();
  }
  return String();
}

void zeroScale(void)
{
  scale.tare(); //reset the scale weight
}

void restartEsp()
{
  ESP.reset();
}

void resetScale()
{
  scale.power_down();
  delay(250);
  scale.power_up();
  //scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor);
}