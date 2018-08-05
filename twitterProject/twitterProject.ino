#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <string.h>
#include <user_interface.h>

// debug print settings
#define SERIAL_FULL false
#define SERIAL_COMPACT true


const int HTTP_TIMEOUT = 10000;

struct tweetInfo
{
  String message;
  String id;
  String date;
  bool valid;
};

struct LocationInfo
{
  String city; // city name
  float lat; // latitude
  float lon; // longitude
};

struct moodState
{
  float romantic;
  float happy;
  float surprised;
  float sad;
  float envy;
  float angry;
  float scared;
};

struct colour
{
  String text;
  int red;
  int green;
  int blue;
};

struct wifiInfo
{
  String login;
  String password;
};

struct moodDescription
{
  String text;
  String words_raw;
  String words_ulr;
};

static moodState lastMood;
static moodState lastState;
static LocationInfo location;
static colour lastColour;

const static colour romanticColour = {"Pink", 255, 0, 255};
const static colour happyColour = {"Yellow", 255, 255, 0};
const static colour surprisedColour = {"Cyan", 0, 255, 255};
const static colour sadColour = {"Blue", 0, 0, 255};
const static colour envyColour = {"Green", 0, 255, 0};
const static colour angryColour = {"Red", 255, 0, 0};
const static colour scaredColour = {"White", 255, 255, 255};
const static colour blackColour = {"Black", 0, 0, 0};

static bool display_mode = true;
static int loop_interval = 10;
wifiInfo wifiSettings = {"ssid", "password"};


void WiFi_On()
{
  wifi_fpm_do_wakeup();
  wifi_fpm_close();
  wifi_set_opmode(STATION_MODE);
  wifi_station_connect();
  while (WiFi.status() != WL_CONNECTED) {
      if (SERIAL_FULL || SERIAL_COMPACT)
        Serial.print(".");
      delay(200);
    }
  if (SERIAL_FULL || SERIAL_COMPACT)
  {
    Serial.println();
    Serial.println("---connected");
  }
}


void WiFi_Off()
{
  wifi_station_disconnect();
  if (SERIAL_FULL || SERIAL_COMPACT)
    Serial.println("---disconnected");
  bool stopped;
  do
  {
    stopped = wifi_station_get_connect_status() == DHCP_STOPPED;
    if (!stopped)
    {
      if (SERIAL_FULL)
        Serial.println("dhcp not stopped?");
      delay(100);
    }
  } while (!stopped);
  if (SERIAL_FULL)
    Serial.println("---off...");
  wifi_set_opmode(NULL_MODE);
  wifi_set_sleep_type(MODEM_SLEEP_T);
  wifi_fpm_open();
  wifi_fpm_do_sleep(0xFFFFFFF);
  while (WiFi.status() == WL_CONNECTED) {
      if (SERIAL_FULL || SERIAL_COMPACT)
        Serial.print(".");
      delay(200);
    }
  if (SERIAL_FULL || SERIAL_COMPACT)
  Serial.println();
}

void setup() {
  pinMode(D1, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D3, OUTPUT);
  Serial.begin(19200);
  randomSeed(analogRead(D8));
  

  //getMoodSettings();
  // Go through all available colours in random order
  if (display_mode)
    lastColour = colourDisplayMode();
  else
    lastColour = blackColour;

  if (SERIAL_FULL || SERIAL_COMPACT)
    Serial.println("Reading wifi settings from the SD card...");
  //wifiSettings = getWifiSettings();
  if (wifiSettings.login == "")
  {
    if (SERIAL_FULL || SERIAL_COMPACT)
      Serial.println("Failed to get wifi settings");
    return;
  }
  if (SERIAL_FULL || SERIAL_COMPACT)
  {
    Serial.print("connecting to ");
    Serial.println(wifiSettings.login);
    Serial.print("using password: ");
    Serial.println(wifiSettings.password);
  }
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.begin(wifiSettings.login.c_str(), wifiSettings.password.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (SERIAL_FULL || SERIAL_COMPACT)
      Serial.print(".");
  }
  if (SERIAL_FULL || SERIAL_COMPACT)
  {
    Serial.println("");
    Serial.println("WiFi connected");
  }

  // ---------- LOCATION --------
  location = getLocationInfo();
  if (SERIAL_FULL || SERIAL_COMPACT)
  {
    Serial.print("Location: ");
    Serial.print(location.city);
    Serial.print(" ");
    Serial.print(location.lat);
    Serial.print(" ");
    Serial.println(location.lon);
    Serial.println();
  }

  lastMood = {0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1};

}


void smoothLedTransition(colour older, colour newer, int dt = 50) {
  int r = older.red;
  int g = older.green;
  int b = older.blue;
  while (r != newer.red || g != newer.green || b != newer.blue) {
    if (r < newer.red) r += 1;
    if (r > newer.red) r -= 1;

    if (g < newer.green) g += 1;
    if (g > newer.green) g -= 1;

    if (b < newer.blue) b += 1;
    if (b > newer.blue) b -= 1;

    analogWrite(D1, 4 * r);
    analogWrite(D4, 4 * g);
    analogWrite(D3, 4 * b);

    delay(dt);
  }
}



void loop() {
  if (WiFi.status() != WL_CONNECTED)
    WiFi_On();
  // ---------- TWITTER --------
  moodState currentMood = {0, 0, 0, 0, 0, 0, 0};

  tweetInfo origin = getOrigin();
  while (origin.date == "") {
    if (SERIAL_FULL || SERIAL_COMPACT)
      Serial.println("Failed to set origin point. Retrying in 20 seconds...");
    delay(20000); // 10 seconds
    origin = getOrigin();
  }

  String lovetweets = "любовь+OR+влюбился+OR+влюбилась+OR+люблю+OR+обожаю+OR+романтика";
  int num = forceTwitterRequest(lovetweets, location.lat, location.lon, origin);
  if (SERIAL_FULL)
  {
    Serial.println();
    Serial.print("Requesting twitter search for: ");
    Serial.println(lovetweets);
    Serial.print("Number of tweets matching this criteria in the last 60 minutes: ");
    Serial.println(num);
    Serial.println();
  }
  currentMood.romantic = num;

  String happytweets = "счастлив+OR+счастлива+OR+ура+OR+взволнована+OR+отлично+OR+супер";
  num = forceTwitterRequest(happytweets, location.lat, location.lon, origin);
  if (SERIAL_FULL)
  {
    Serial.println();
    Serial.print("Requesting twitter search for: ");
    Serial.println(happytweets);
    Serial.print("Number of tweets matching this criteria in the last 60 minutes: ");
    Serial.println(num);
    Serial.println();
  }
  currentMood.happy = num;

  String surprisetweets = "ого!+OR+неожиданно+OR+невероятно+OR+не+могу+поверить+OR+удивительно";
  num = forceTwitterRequest(surprisetweets, location.lat, location.lon, origin);
  if (SERIAL_FULL)
  {
    Serial.println();
    Serial.print("Requesting twitter search for: ");
    Serial.println(surprisetweets);
    Serial.print("Number of tweets matching this criteria in the last 60 minutes: ");
    Serial.println(num);
    Serial.println();
  }
  currentMood.surprised = num;

  String hatetweets = "ненавижу+OR+уроды+OR+ненависть+OR+в+бешенстве+OR+зол+OR+зла";
  num = forceTwitterRequest(hatetweets, location.lat, location.lon, origin);
  if (SERIAL_FULL)
  {
    Serial.println();
    Serial.print("Requesting twitter search for: ");
    Serial.println(hatetweets);
    Serial.print("Number of tweets matching this criteria in the last 60 minutes: ");
    Serial.println(num);
    Serial.println();
  }
  currentMood.angry = num;

  String wishtweets = "моя+мечта+OR+очень+хочу+OR+хочу+быть+OR+вот+если+бы+OR+хотелось+бы+OR+хочу+чтобы";
  num = forceTwitterRequest(wishtweets, location.lat, location.lon, origin);
  if (SERIAL_FULL)
  {
    Serial.println();
    Serial.print("Requesting twitter search for: ");
    Serial.println(wishtweets);
    Serial.print("Number of tweets matching this criteria in the last 60 minutes: ");
    Serial.println(num);
    Serial.println();
  }
  currentMood.envy = num;

  String sadtweets = "грустно+OR+в+печали+OR+депрессия+OR+плачу+OR+скучно+OR+разбитое+сердце";
  num = forceTwitterRequest(sadtweets, location.lat, location.lon, origin);
  if (SERIAL_FULL)
  {
    Serial.println();
    Serial.print("Requesting twitter search for: ");
    Serial.println(sadtweets);
    Serial.print("Number of tweets matching this criteria in the last 60 minutes: ");
    Serial.println(num);
    Serial.println();
  }
  currentMood.sad = num;

  String scaredtweets = "страшно+OR+в+ужасе+OR+очень+боюсь+OR+опасаюсь+OR+страх+OR+боюсь";
  num = forceTwitterRequest(scaredtweets, location.lat, location.lon, origin);
  if (SERIAL_FULL)
  {
    Serial.println();
    Serial.print("Requesting twitter search for: ");
    Serial.println(scaredtweets);
    Serial.print("Number of tweets matching this criteria in the last 60 minutes: ");
    Serial.println(num);
    Serial.println();
  }
  currentMood.scared = num;

  if (SERIAL_FULL || SERIAL_COMPACT)
    Serial.println();
  moodState state = calculateMood(currentMood);

  if (SERIAL_FULL || SERIAL_COMPACT)
  {
    Serial.println();
    Serial.println("Diviation from last mood:");
  }
  moodState mood = getDiviation(lastMood, state);
  // ------------------------------------------------
  colour lastc = lastColour;
  colour newc = mood2colour(mood);
  if (SERIAL_FULL || SERIAL_COMPACT)
  {
    Serial.println("--------------------");
    Serial.print("Changing color from [");
    Serial.print(lastc.text);
    Serial.print("] to [");
    Serial.print(newc.text);
    Serial.println("].");
    Serial.println("--------------------");
  }
  smoothLedTransition(lastc, newc);
  lastColour = newc;
  lastMood = state;

  if (WiFi.status() == WL_CONNECTED)
    WiFi_Off();
  int sleepTime = loop_interval * 60 * 1000;
  delay(sleepTime);
}












