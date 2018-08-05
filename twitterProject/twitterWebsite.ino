#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <string.h>

const char* BEARER_TOKEN = "twitter_should_provide_you_with_this_key";
const char* twitterHost = "api.twitter.com";
const char* twitterUrl = "/json";
const int twitterPort = 443;
WiFiClientSecure twitterClient;

tweetInfo searchOnTwitter(String keyWords, float lat, float lon, int count, String maxID, float area);
bool sendTwitterRequest(const char* host, const char* resource);


int forceTwitterRequest(String keyWords, float lat, float lon, tweetInfo origin)
{
  twitterClient.setTimeout(HTTP_TIMEOUT);
  if (SERIAL_FULL)
  {
    Serial.print("Connecting to ");
    Serial.println(twitterHost);
  }
  if (!twitterClient.connect(twitterHost, twitterPort))
  {
    if (SERIAL_FULL || SERIAL_COMPACT)
      Serial.println("twitter connection failed");
    return -1;
  }
  tweetInfo response = {"", "", "", false};
  unsigned int timemark = 0;
  String newNumber = "";
  bool searchfinished = false;
  int tweetcounter = 0;

  unsigned int originmark = string2time(getSubstring(origin.date, ' ', 3));
  newNumber = str_substractOne(origin.id);
  if (originmark == 0) {
    if (SERIAL_FULL)
      Serial.println("Failed to generate time origin point.");
    return -1;
  }
  while (!searchfinished) {
    delay(20);
    response = searchOnTwitter(urlencode(keyWords), lat, lon, 1, newNumber, 75);
    if (response.message == "") {
      searchfinished = true;
      if (SERIAL_FULL)
      {
        Serial.print("Search is finished (empty). Tweets found: ");
        Serial.println(tweetcounter);
      }
    }
    else {
      tweetcounter++;
      timemark = string2time(getSubstring(response.date, ' ', 3));
      if ((originmark - timemark) / 60 > 120) {
        searchfinished = true;
        tweetcounter--;
        if (SERIAL_FULL)
        {
          Serial.print("Search is finished (filled). Tweets found: ");
          Serial.println(tweetcounter);
        }
      }
      if (response.id.equals(newNumber)) {
        searchfinished = true;
        tweetcounter--;
        if (SERIAL_FULL)
        {
          Serial.print("Search is finished (repeated). Tweets found: ");
          Serial.println(tweetcounter);
        }
      }
      else {
        newNumber = str_substractOne(response.id);
      }
    }
  }
  if (SERIAL_FULL)
  {
    Serial.print("Disconnecting from ");
    Serial.println(twitterHost);
  }
  twitterClient.stop();
  return tweetcounter;
}



tweetInfo getOrigin()
{
  tweetInfo response = {"", "", "", false};
  twitterClient.setTimeout(HTTP_TIMEOUT);
  if (SERIAL_FULL)
  {
    Serial.print("Connecting to ");
    Serial.println(twitterHost);
  }
  if (!twitterClient.connect(twitterHost, twitterPort))
  {
    if (SERIAL_FULL || SERIAL_COMPACT)
      Serial.println("twitter connection failed");
    return response;
  }
  response = searchOnTwitter("", 10, 10, 1, ".", 20);
  if (SERIAL_FULL)
  {
    Serial.print("Disconnecting from ");
    Serial.println(twitterHost);
  }
  twitterClient.stop();
  return response;
}






tweetInfo searchOnTwitter(String keyWords, float lat, float lon, int count, String maxID = "", float area = 20)
{
  // ----------------------------------------
  // Oh boy, lets construct twitter request from function argumets
  char cKeyWords[keyWords.length() + 1]; // +1 is essential for char[]
  keyWords.toCharArray(cKeyWords, keyWords.length() + 1);
  char cLat[8]; // Buffer big enough for 7-character float
  dtostrf(lat, 2, 4, cLat);
  char cLon[8]; // Buffer big enough for 7-character float
  dtostrf(lon, 2, 4, cLon);
  char cArea[5]; // 4 digits
  dtostrf(area, 3, 1, cArea);
  char cMaxID[maxID.length() + 1]; // 22 digits
  maxID.toCharArray(cMaxID, maxID.length() + 1);
  char combinedArray[700]; // 250 was not enough
  if (maxID == "")
  {
      sprintf(combinedArray, "/1.1/search/tweets.json?q=%s&lang=ru&geocode=%s,%s,%skm&count=%d&result_type=recent", cKeyWords, cLat, cLon, cArea, count);
  }
  else if (maxID == ".")
  {
    // this is used to get initial time. Based on my obserbations
    // tweets with "and" word in it occur every 20-30 seconds.
    // So it is actually pretty accurate time measurement system. Genious things xD
      sprintf(combinedArray, "/1.1/search/tweets.json?q=and+OR+was&count=%d&result_type=recent", count);
  }
  else
  {
      sprintf(combinedArray, "/1.1/search/tweets.json?q=%s&lang=ru&geocode=%s,%s,%skm&count=%d&result_type=recent&max_id=%s", cKeyWords, cLat, cLon, cArea, count, cMaxID);
  }
  // -----------------------------------------
  // Now with that out of the way, we can procceed to download data from twitter!
  tweetInfo result = {"", "", "", false}; // default return value
  if (SERIAL_FULL)
  {
    Serial.print("Sending request: ");
    Serial.println(combinedArray);
  }
  sendTwitterRequest(twitterHost, combinedArray);
  // Clear HTTP headers --------------
  // HTTP headers end with an empty line
  char endOfHeaders[] = "\r\n\r\n";
  //twitterClient.setTimeout(HTTP_TIMEOUT);
  if (!twitterClient.find(endOfHeaders)) {
    if (SERIAL_FULL)
      Serial.println("No response or invalid response!");
    return result;
  }
  delay(50);
  String line = "";
  Serial.flush();
  while (twitterClient.available()) {
    line += twitterClient.readStringUntil('}');
    line += "}\n";
    delay(10);
  }
  //String line = twitterClient.readStringUntil('}');
  //line  = line + "}";
  //Serial.println(line); // used for debug puproses
  // Yeah, one twitter JSON is a massive structure
  const size_t twitterCapacity = 6 * JSON_ARRAY_SIZE(0) + 10 * JSON_ARRAY_SIZE(1) + 9 * JSON_ARRAY_SIZE(2) +
                                 6 * JSON_OBJECT_SIZE(1) + 4 * JSON_OBJECT_SIZE(2) + 16 * JSON_OBJECT_SIZE(3) +
                                 8 * JSON_OBJECT_SIZE(4) + 3 * JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(9) +
                                 2 * JSON_OBJECT_SIZE(10) + 2 * JSON_OBJECT_SIZE(14) + JSON_OBJECT_SIZE(26) +
                                 JSON_OBJECT_SIZE(27) + JSON_OBJECT_SIZE(41) + JSON_OBJECT_SIZE(42);
  DynamicJsonBuffer twitterJsonBuffer(twitterCapacity);
  // Parse JSON object
  JsonObject& twitterRoot = twitterJsonBuffer.parseObject(line);
  if (!twitterRoot.success()) {
    //if (true) {
    if (SERIAL_FULL)
      Serial.println("Failed to parse twitter response:");
    //Serial.println(line);
    result = bruteForceJSON_c(line.c_str());
    return result;
  }
  if ((line.length() < 800) || (line.indexOf("text") < 0)) {
    if (SERIAL_FULL)
      Serial.println("It appears that the JSON was epmty :(");
    //Serial.println(line);
    return result;
  }
  // ----------------------------------------
  // Time to parse our fresh json!
  if (SERIAL_FULL)
    Serial.println("--- Parsed json ---");
  //Serial.println(line);
  // You may notice that in case of count > 1, only last json will be returned as result/
  // And this is exactly correct! In my short study, i found that parsing two or more jsons
  // from twitter at once is not reliable way to get things done. Its size would be so large,
  // that ESP8266 chip simply cannot store it in it's memory! So I will be only using this
  // function with count=1, but feel free to make it right and improve it!
  for (int qw = 0; qw < count; qw++)
  {
    JsonObject& rootStatus = twitterRoot["statuses"][qw];
    result.id = rootStatus["id"].as<char*>();
    result.date = rootStatus["created_at"].as<char*>();
    result.message = rootStatus["text"].as<char*>();
    result.valid = true;
  }
  return result;
}



// Send the HTTPS GET request to the server
bool sendTwitterRequest(const char* host, const char* resource) {
  // Serial.print("GET ");
  //Serial.println(resource);

  twitterClient.print("GET ");
  twitterClient.print(resource);
  twitterClient.println(" HTTP/1.1");
  twitterClient.print("Host: ");
  twitterClient.println(host);
  twitterClient.println("User-Agent: arduino/1.0.0");
  twitterClient.print("Authorization: Bearer ");
  twitterClient.println(BEARER_TOKEN);
  twitterClient.println();

  return true;
}
