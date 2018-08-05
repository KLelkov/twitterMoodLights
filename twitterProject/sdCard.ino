// MOSI - D7
// MISO - D6
// CLK - D5
// CS - D2
#include <SPI.h>
#include <SD.h>


static moodDescription romanticMood = {"Romantic", "", ""};
static moodDescription happyMood = {"Happy", "", ""};

char* filename = "settings.txt";
File myFile;

wifiInfo getWifiSettings()
{
  wifiInfo access = {"", ""};
  if (!SD.begin(D2)) {
    if (SERIAL_FULL || SERIAL_COMPACT)
      Serial.println("SD card initialization failed!");
    return access;
  }

  // If file doesn't exist - create it with default values
  if (!SD.exists(filename))
  {
    myFile = SD.open(filename, FILE_WRITE);
    if (myFile)
    {
      myFile.println("wifi_login:default_ssid");
      myFile.println("wifi_password:default_password");
      myFile.println("##");
      myFile.close();
    }
    else
      if (SERIAL_FULL || SERIAL_COMPACT)
        Serial.println("Error creating a file on the SD card.");
  }

  // Read data from file
  myFile = SD.open(filename);
  if (myFile) {
    long len = myFile.size();
    char filebuffer[len];
    myFile.read(filebuffer, len);
    String fullfile = filebuffer;
    if (fullfile.startsWith("wifi_login="))
    {
      int index = fullfile.indexOf("wifi_password");
      int endIndex = fullfile.indexOf("##");
      String login = fullfile.substring(11, index-2);
      String password = fullfile.substring(index+14, endIndex-2);
      access.login = login;
      access.password = password;
    }
    else
    {
      if (SERIAL_FULL || SERIAL_COMPACT)
        Serial.println("File on SD card is incorrect. Please delete or edit it.");
    }
    myFile.close();
  } else {
    if (SERIAL_FULL || SERIAL_COMPACT)
      Serial.println("Error opening a file on the SD card.");
  }

  return access;
}


int getMoodSettings()
{
  if (!SD.exists(filename))
  {
    if (SERIAL_FULL || SERIAL_COMPACT)
      Serial.println("Could not read mood settings - file doesn't exist");
    return -1;
  }
  int startIndx = 0;
  int endIndx = 0;
  String substr = "";
  // Read data from file
  myFile = SD.open(filename);
  if (myFile) {
    long len = myFile.size();
    char filebuffer[len];
    myFile.read(filebuffer, len);
    String fullfile = filebuffer;
    // check if it is the correct file
    if (fullfile.startsWith("wifi_login="))
    {
      endIndx = fullfile.indexOf("##");
      if (len - endIndx < 20)
      {
        if (SERIAL_FULL || SERIAL_COMPACT)
          Serial.println("Could not read mood settings - file is too short");
        return -1;
      }
      // --
      startIndx = fullfile.indexOf("display_colors=");
      endIndx = fullfile.indexOf("##", startIndx);
      substr = fullfile.substring(startIndx+15, endIndx-2);
      substr.toLowerCase();
      int tmp = substr.compareTo("true");
      if (tmp == 0)
        display_mode = true;
      else
        display_mode = false;
      //Serial.print("display_mode = ");Serial.println(display_mode);
      // --
      startIndx = fullfile.indexOf("scan_interval=");
      endIndx = fullfile.indexOf("##", startIndx);
      substr = fullfile.substring(startIndx+14, endIndx-2);
      loop_interval = substr.toInt();
      //Serial.print("loop_interval = ");Serial.println(loop_interval);
    }
    else
      if (SERIAL_FULL || SERIAL_COMPACT)
        Serial.println("File on SD card is incorrect. Please delete or edit it.");
    myFile.close();
  } else
    if (SERIAL_FULL || SERIAL_COMPACT)
      Serial.println("Error opening a file on the SD card.");

  return 0;
}
