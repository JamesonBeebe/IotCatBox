//  IoT Cat Litter Box V0.3 by Jameson Beebe
//  Measures Cat weight and frequency of use for multiple cats
#include "application.h"
#include "HX711.h"
#include "box.h"
#include "cat.h"
#include "ThingSpeak.h"
#include "realsecrets.h"
#include <ESP8266WiFi.h> // Wi-Fi
#include <WiFiClient.h>  // Wi-Fi client
#include <ESPAsyncTCP.h> //used for OTA and web GUI
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include "index.htm"

/************************ Thingspeak  Configuration ************************/

// visit thingspeak.com if you need to create an account or if you need your write key.
unsigned long myChannelNumber = SECRET_CH_ID;
const char *apiKey = SECRET_WRITE_APIKEY;
const char *readAPIKey = SECRET_READ_APIKEY;
const char *server = "api.thingspeak.com"; //   "184.106.153.149" or api.thingspeak.com

// Define Channels
#define CAT_1_WEIGHT 1
#define CAT_2_WEIGHT 2
#define RUN_STATE 3
#define SAND_WEIGHT 4
#define CAT_1_USES 5
#define CAT_2_USES 6
#define POO_WEIGHT 7

/************************ WIFI Configuration ************************/
const char *ssid = SECRET_SSID; // replace with your wifi ssid and wpa2 key
const char *pass = SECRET_PASS;
WiFiClient client;
#define WIFI_TIMEOUT 20 // Wi-Fi Timeout for local connections (multiply this number by 500mS)

/************************ OTA  Configuration ************************/
AsyncWebServer otaServer(1234);

/************************ GUI *********************************/
AsyncWebServer gui(2468);

/************************ Define IO pins ************************/
#define calibration_factor -22994.0 // This value is obtained using the SparkFun_HX711_Calibration sketch
#define DOUT 0                      // Pin connected to HX711 data output pin
#define CLK 12                      // Pin connected to HX711 clk pin
HX711 *scale = new HX711();
Application *application = new Application(1.8);
Box *box = new Box(0.7);
/************************ Application Defines ************************/
#define NUM_MEASUREMENTS 10 // Number of measurements

bool newData = false;

/******************CATS******************/
Cat *cat1 = new Cat(2, 6); // initialize cats with weights in Kg's
Cat *cat2 = new Cat(6.3, 11);
bool cat1Use = false;

float weight = 0;

#define reed_switch_PIN 13
int reed_switch_value = digitalRead(reed_switch_PIN);
int prev_reed_switch_value = reed_switch_value;

/************************ Clock settings ************************/
uint32_t internetTime = 0;    // initialize internet time
uint32_t timeTimeout = 60000; // 10 second timeout
uint32_t lastInetTime;
String TimeDate = ""; // Variable used to store the data received from the server
int hours;            // Current hour
int minutes;          // Current minute
int seconds;          // Current second
int seconds0;         // Last time the clock was updated
#define TIMEZONE 4    // Current time zone. Used for correction of server time
/********************** Local Defines ***************************/
// #define __local_wifi__

String readWeightString(void);
String processor(const String &var);
void zeroScale(void);
void restartEsp(void);
void resetScale(void);
float getLastFieldValue(unsigned long channel);
void readServerValues(void);
void zeroCatUses(void);
uint32_t getTime(void);
float readScaleFloat(void);
String getCurrentState(void);
String getNextState(void);

void initIWDT(void);

void resetting(void);
void runningLoop(void);
void cleaningLoop(void);
void catLoop(void);
void weighCat(void);

enum State
{
  initializing,
  cleaning,
  running,
  cat
} currentState,
    nextState;

void setup()
{
  currentState = initializing;
  box = new Box(0.7);                 // initialize box with weight of 0.7 Kg's
  application = new Application(1.8); // initialize the application with a platform weight of 1.8 Kg's

  // Start the serial connection
  Serial.begin(115200);
  Serial.println("IoT CatBox by: Jameson Beebe\r\n");
  Serial.printf("V%d.%d\r\n", SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR);

  
  pinMode(reed_switch_PIN, INPUT_PULLUP); //This is for a Mag Switch if one is used for the Lid (not currently implemented)

  scale->begin(DOUT, CLK); // Scale initialization
  // Scale calibration
  scale->set_scale(calibration_factor); // This value is obtained by using the SparkFun_HX711_Calibration sketch
  box->setPooWeight(0);                 // initialize poo weight to 0
  resetting();

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
      break; // jump out of loop if local timeout hit to start SmartConfig
    count++;
  }

  if ((WiFi.status() != !WL_CONNECTED))
  { // Local Auto-connect did not happen, start WiFi SmartConfig
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

  // Start OTA
  otaServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(200, "text/plain", "Hi! I am ESP8266."); });

  AsyncElegantOTA.begin(&otaServer); // Start ElegantOTA
  otaServer.begin();
  Serial.println("HTTP OTA server started");

  // Start GUI
  gui.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
         { request->send_P(200, "text/html", index_html, processor); });
  gui.on("/updateWeight", HTTP_GET, [](AsyncWebServerRequest *request)
         { request->send(200, "text/plain", String(box->getCurrentWeight())); });
  gui.on("/litterWeight", HTTP_GET, [](AsyncWebServerRequest *request)
         { request->send(200, "text/plain", String(box->getSandWeight())); });
  gui.on("/rigginsUses", HTTP_GET, [](AsyncWebServerRequest *request)
         { request->send(200, "text/plain", String(cat2->getUses())); });
  gui.on("/whiskeyUses", HTTP_GET, [](AsyncWebServerRequest *request)
         { request->send(200, "text/plain", String(cat1->getUses())); });
  gui.on("/updateState", HTTP_GET, [](AsyncWebServerRequest *request)
         { request->send(200, "text/plain", String(getCurrentState())); });
  // Route for getting variable values
  gui.on("/getValues", HTTP_GET, [](AsyncWebServerRequest *request)
         {
    String response = String("{\"weight\":") + String(box->getCurrentWeight()) + 
                      String(",\"state\":") + "\"" + getCurrentState() + "\"" + "}";
    request->send(200, "application/json", response); });
  gui.on("/zero", HTTP_GET, [](AsyncWebServerRequest *request)
         { zeroScale(); });
  gui.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request)
         { restartEsp(); });
  gui.on("/resetscale", HTTP_GET, [](AsyncWebServerRequest *request)
         { resetScale(); });
  gui.begin();

  // Start ThingSpeak interface
  ThingSpeak.begin(client); // Initialize ThingSpeak
}

void loop()
{
  currentState = nextState;

  // Controls state changes and it's also the "current weight" for each State
  box->setLastWeight(box->getCurrentWeight());
  box->setCurrentWeight(readScaleFloat());

  if (box->weightIsStable())
  {
    Serial.println("Weight: STABLE");

    switch (currentState)
    {
    case initializing:
      Serial.println(getCurrentState());
      // ThingSpeak.setField(RUN_STATE, currentState);
      // newData = true;
      nextState = running;
      break;

    case cleaning:
      currentState = cleaning;
      Serial.println(getCurrentState());
      cleaningLoop();

      if ((box->getCurrentWeight() > MEASUREMENT_TOLERANCE))
      {                                             // someone put the box on the platform
        Serial.println("Leaving cleaning mode..."); // When it leaves cleaning mode, it shall wait some seconds and measure the weight of sand in the litter box
        nextState = running;
        resetting();
        Serial.print("Current Weight: ");
        Serial.println(box->getCurrentWeight());
        Serial.print("Next State: ");
        Serial.println(getNextState());
      }
      break;

    case running:
      currentState = running;
      Serial.println(getCurrentState());

      runningLoop();

      if ((box->getCurrentWeight() < (-MEASUREMENT_TOLERANCE)))
      { // someone lifted the box off the platform

        nextState = cleaning;

        Serial.print("Current Weight: ");
        Serial.println(box->getCurrentWeight());
        Serial.print("Next State: ");
        Serial.println(getNextState());

        zeroScale(); // zero scale to more easily detect state transition
      }
      if (box->catPresent())
      { // cat entered the box
        Serial.println("Cat detected!");
        nextState = cat;
        weighCat();

        Serial.print("Current Weight: ");
        Serial.println(box->getCurrentWeight());
        Serial.print("Next State: ");
        Serial.println(getNextState());
      }
      break;

    case cat:
      currentState = cat;
      Serial.println(getCurrentState());

      if (!box->catPresent())
      {
        catLoop();

        nextState = running;

        Serial.print("Current Weight: ");
        Serial.println(box->getCurrentWeight());
        Serial.print("Next State: ");
        Serial.println(getNextState());
      }
      break;

    default:
      break;
    }
  }
  else
  {
    Serial.println("Weight: UNSTABLE");
  }

  if (newData)
  {
    gui.end();
    // save values to Thingspeak only if something needs updating'
    Serial.print("new data sending...");
    if (client.connect(server, 80))
    {
      int x = ThingSpeak.writeFields(myChannelNumber, apiKey);
      switch (x)
      {
      case 200:
        Serial.println("Success!");
        newData = false;
        break;

      case 404:
        Serial.println("Incorrect API key or invalid ThingSpeak server address.");
        break;

      case -301:
        Serial.println("Failed to connect to ThingSpeak.");
        break;

      case -304:
        Serial.println("Timeout waiting for server to respond.");
        break;

      default:
        Serial.printf("TS Error: %d\r", x);
        break;
      }
    }
    client.stop();
    gui.begin();
  }
}

void runningLoop(void)
{
  // do nothing here. Wait for main application to change states
}

void cleaningLoop(void)
{
  // do nothing here. Wait for main application to change states
}

void weighCat(void)
{
  float tmpWeight = 0;

  tmpWeight = readScaleFloat();
  tmpWeight -= box->getPooWeight(); // subtract out the previous poo weight to reveal (cat weight + poo weight)

  if ((tmpWeight >= cat1->getMinWeight()) && (box->getCurrentWeight() <= cat1->getMaxWeight()))
  { // cat #1 detected
    cat1Use = true;
    cat1->incrementUses();

    ThingSpeak.setField(CAT_1_USES, cat1->getUses());
    newData = true;
  }
  else if (tmpWeight >= cat2->getMinWeight())
  { // cat #2 detected
    cat2->incrementUses();

    ThingSpeak.setField(CAT_2_USES, cat2->getUses());
    newData = true;
  }
  else
  {
    // nothing to do here if it wasn't a cat
    return;
  }
}

/**
 * When a cat enters the box, the scale will read the cat's weight + poo weight.
 * Take the measurement when the cat first gets in, then when the cat leaves.
 * first measurement = cat + poop in cat + poop
 * second measurement = poop in cat + poop
 */
void catLoop(void)
{
  // update the level of poop in the box
  box->setPooWeight(box->getCurrentWeight());

  if (cat1Use)
  {
    cat1->setCurrentWeight(box->lastStableWeight - box->getPooWeight());
    ThingSpeak.setField(CAT_1_WEIGHT, cat1->getCurrentWeight()); // set measured value to cat #1
  }
  else
  {
    cat2->setCurrentWeight(box->lastStableWeight - box->getPooWeight());
    ThingSpeak.setField(CAT_2_WEIGHT, cat2->getCurrentWeight()); // set measured value to cat #2
  }

  ThingSpeak.setField(POO_WEIGHT, (float)(box->getPooWeight() * 2.2)); // convert to lbs and send to ThingSpeak

  newData = true;
}

void resetting(void)
{
  float weight0 = scale->get_units(3) - box->getBoxWeight() - application->getPlatformWeight();
  float avgweight = 0;
  for (int i = 0; i <= NUM_MEASUREMENTS - 1; i++)
  { // Takes several measurements
    weight = scale->get_units(3) - box->getBoxWeight() - application->getPlatformWeight();
    delay(100);
    avgweight += weight;
    if (abs(weight - weight0) > MEASUREMENT_TOLERANCE)
    {
      avgweight = 0;
      i = 0;
    }
    weight0 = weight;
  }
  box->setSandWeight((avgweight / NUM_MEASUREMENTS) - box->getBoxWeight() - application->getPlatformWeight());
  Serial.print("Measured sand weight: ");
  Serial.print((box->getSandWeight() * 2.2), 2);
  Serial.println(" lbs");
  ThingSpeak.setField(SAND_WEIGHT, box->getSandWeight());

  // reset soil level after cleaning
  box->zeroPooValue();

  zeroScale();

  box->setCurrentWeight(readScaleFloat());
  box->setLastWeight(readScaleFloat());

  if (currentState != initializing)
  {
    ThingSpeak.setField(POO_WEIGHT, box->getPooWeight());
    newData = true;
  }
}

// Get current time Google server
uint32_t getTime()
{
  Serial.println("Fetching Time");
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect("worldtimeapi.org", httpPort))
  {
    Serial.println("Connection failed");
    return 0;
  }

  // We now create a URI for the request
  String url = "/api/ip";
  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + "worldtimeapi.org" + "\r\n" +
               "Connection: close\r\n\r\n");

  unsigned long timeout = millis();
  while (client.available() == 0)
  {
    if (millis() - timeout > 5000)
    {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return 0;
    }
  }
  String line;
  // Read all the lines of the reply from server and print them to Serial
  while (client.available())
  {
    line = client.readStringUntil('\r');
    Serial.print(line);
  }

  // Find the index of the time string within the line string
  int timeIndex = line.indexOf("unixtime\":");
  String timeStr;
  // If the time string was found, extract it and print it to Serial
  if (timeIndex != -1)
  {
    timeStr = line.substring(timeIndex + 10, timeIndex + 20);
    Serial.print("Time: ");
    Serial.println(timeStr);
    Serial.println(timeStr.toDouble());
  }

  Serial.println();
  Serial.println("closing connection");
  return ((uint32_t)timeStr.toDouble());
}

String readWeightString(void)
{
  scale->wait_ready_retry(3, 100);
  box->setCurrentWeight(scale->get_units(3));
  return String(box->getCurrentWeight());
}

// Replaces placeholder with DHT values
String processor(const String &var)
{
  if (var == "WEIGHT")
  {
    return readWeightString();
  }
  return String();
}

void zeroScale(void)
{
  scale->wait_ready_retry(3, 10);
  scale->tare(); // reset the scale weight
}

void restartEsp()
{
  ESP.reset();
}

void resetScale()
{
  scale->wait_ready_retry(3, 100);
  scale->power_down();
  scale->power_up();
  scale->wait_ready_timeout(3000, 100);
  scale->set_scale(calibration_factor);
}

float getLastFieldValue(unsigned long field)
{
  return ThingSpeak.readFloatField(myChannelNumber, field);
}

void readServerValues(void)
{
  // Read saved values
  cat1->setUses((byte)ThingSpeak.readLongField(myChannelNumber, CAT_1_USES, readAPIKey));
  cat2->setUses((byte)ThingSpeak.readLongField(myChannelNumber, CAT_2_USES, readAPIKey));
  box->setSandWeight((float)ThingSpeak.readLongField(myChannelNumber, SAND_WEIGHT, readAPIKey));
}

void zeroCatUses(void)
{
  cat1->setUses(0);
  cat2->setUses(0);
}

float readScaleFloat(void)
{
  box->setLastWeight(box->getCurrentWeight());
  scale->wait_ready_retry(3, 10);
  box->setCurrentWeight(scale->get_units(3));
  return box->getCurrentWeight();
}

String getCurrentState(void)
{
  switch (currentState)
  {
  case initializing:
    return String("Initializing");
    break;

  case cleaning:
    return String("Cleaning");
    break;

  case running:
    return String("Running");
    break;

  case cat:
    return String("Cat");
    break;

  default:
    break;
  }
  return String(currentState);
}

String getNextState(void)
{
  switch (nextState)
  {
  case initializing:
    return String("Initializing");
    break;

  case cleaning:
    return String("Cleaning");
    break;

  case running:
    return String("Running");
    break;

  case cat:
    return String("Cat");
    break;

  default:
    break;
  }
  return String(nextState);
}

// Initialize IWDT
void initIWDT(void)
{
  ESP.wdtDisable(); // Disable the WDT (in case it was previously enabled)

  // Configure IWDG
  ESP.wdtEnable(WDTO_8S); // Enable the IWDG with a timeout of 8 seconds
  ESP.wdtFeed();
}