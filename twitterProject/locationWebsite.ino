#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <string.h>

const char* locationHost = "ip-api.com";
const char* locationUrl = "/json";
const int locationPort = 80;
WiFiClient locationClient;


LocationInfo getLocationInfo()
{
  LocationInfo result = {"", 0, 0};
  locationClient.setTimeout(HTTP_TIMEOUT);
  if (SERIAL_FULL)
  {
    Serial.print("connecting to ");
    Serial.println(locationHost);
  }
  if (!locationClient.connect(locationHost, locationPort))
  {
    if (SERIAL_FULL)
      Serial.println("connection failed");
    return result;
  }
  sendLocationRequest(locationHost, locationUrl);

  // Clear HTTP headers --------------
  // HTTP headers end with an empty line
  char endOfHeaders[] = "\r\n\r\n";

  //locationClient.setTimeout(HTTP_TIMEOUT);
  if (!locationClient.find(endOfHeaders)) {
    if (SERIAL_FULL)
      Serial.println("No response or invalid response!");
  }
  // -------
  String line = locationClient.readStringUntil('}');
  line  = line + "}";

  // Allocate JsonBuffer
  // Use arduinojson.org/assistant to compute the capacity.
  const size_t capacity = JSON_OBJECT_SIZE(14) + 330;
  DynamicJsonBuffer jsonBuffer(capacity);

  // Parse JSON object
  JsonObject& root = jsonBuffer.parseObject(line);
  if (!root.success()) {
    if (SERIAL_FULL)
    {
      Serial.println("Failed to parse following response:");
      Serial.println(line);
    }
    return result;
  }

  if (SERIAL_FULL)
  {
    Serial.print("Disconnecting from ");
    Serial.println(locationHost);
  }
  locationClient.stop();

  result.city = root["city"].as<char*>();
  result.lat = root["lat"].as<float>();
  result.lon = root["lon"].as<float>();
  return result;
}

// Send the HTTP GET request to the server
bool sendLocationRequest(const char* host, const char* resource) {
  //Serial.print("GET ");
  //Serial.println(resource);
  locationClient.print("GET ");
  locationClient.print(resource);
  locationClient.println(" HTTP/1.1");
  locationClient.print("Host: ");
  locationClient.println(host);
  locationClient.println("Connection: close");
  locationClient.println();
  return true;
}


