// unicode decode dictionary (ony the parts I actually need)
String unicodeDictionary[][2] = {
  {"u0410", "А"},    {"u0411", "Б"},    {"u0412", "В"},    {"u0413", "Г"},
  {"u0414", "Д"},    {"u0415", "Е"},    {"u0401", "Ё"},    {"u0416", "Ж"},
  {"u0417", "З"},    {"u0418", "И"},    {"u0419", "Й"},    {"u041a", "К"},
  {"u041b", "Л"},    {"u041c", "М"},    {"u041d", "Н"},    {"u041e", "О"},
  {"u041f", "П"},    {"u0420", "Р"},    {"u0421", "С"},    {"u0422", "Т"},
  {"u0423", "У"},    {"u0424", "Ф"},    {"u0425", "Х"},    {"u0426", "Ц"},
  {"u0427", "Ч"},    {"u0428", "Ш"},    {"u0429", "Щ"},    {"u042a", "Ъ"},
  {"u042d", "Ы"},    {"u042c", "Ь"},    {"u042d", "Э"},    {"u042e", "Ю"},
  {"u042f", "Я"},    {"u0430", "а"},    {"u0431", "б"},    {"u0432", "в"},
  {"u0433", "г"},    {"u0434", "д"},    {"u0435", "е"},    {"u0451", "ё"},
  {"u0436", "ж"},    {"u0437", "з"},    {"u0438", "и"},    {"u0439", "й"},
  {"u043a", "к"},    {"u043b", "л"},    {"u043c", "м"},    {"u043d", "н"},
  {"u043e", "о"},    {"u043f", "п"},    {"u0440", "р"},    {"u0441", "с"},
  {"u0442", "т"},    {"u0443", "у"},    {"u0444", "ф"},    {"u0445", "х"},
  {"u0446", "ц"},    {"u0447", "ч"},    {"u0448", "ш"},    {"u0449", "щ"},
  {"u044a", "ъ"},    {"u044b", "ы"},    {"u044c", "ь"},    {"u044d", "э"},
  {"u044e", "ю"},    {"u044f", "я"},    {"u2026", "..."}, {"&amp;#171;", "\""},
  {"&amp;#187;", "\""}
};


String unicode2russian(String unicodeString)
{
  String russianString = unicodeString;
  int len = sizeof(unicodeDictionary) / 24; // 24? dont aske me why!
  // for some reason arduino doesnt have array.length() function
  // so I had to find out how many BYTES it takes to store those strings.
  for (int re = 0; re < len; re++)
    russianString.replace(unicodeDictionary[re][0], unicodeDictionary[re][1]);
  return russianString;
}

// takes time in hh:mm:ss and transforms it into seconds
unsigned int string2time(String timeString)
{
  unsigned int timeInSeconds = 0;
  int iHour = getSubstring(timeString, ':', 0).toInt();
  int iMinute = getSubstring(timeString, ':', 1).toInt();
  int iSecond = getSubstring(timeString, ':', 2).toInt();
  timeInSeconds = iHour * 3600 + iMinute * 60 + iSecond;
  return timeInSeconds;
}

// returns [index] substring from original string
String getSubstring(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  // I understand it now, but I will probably regret it later xD
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


// takes number as a string, substracts 1 from it and returns to the user
// twitter id is 18 digit number at the moment, even unsigned long int cant store it!
String str_substractOne(String source)
{
  String response = source;
  int indx = source.length() - 1;
  bool shift = false;
  char tempChar = source.charAt(indx);
  if (tempChar == '0')
    shift = true;
  response.setCharAt(indx, --tempChar);
  indx--;
  tempChar = source.charAt(indx);
  if (shift) {
    while (tempChar == '0') {
      response.setCharAt(indx, --tempChar);
      indx--;
      tempChar = source.charAt(indx);
    }
    response.setCharAt(indx, --tempChar);
  }

  for (int i = 0; i < response.length(); i++) {
    if (response.charAt(i) == '/')
      response.setCharAt(i, '9');
  }
  return response;
}

char* string2char(String command) {
  if (command.length() != 0) {
    char *p = const_cast<char*>(command.c_str());
    return p;
  }
}




tweetInfo bruteForceJSON_c(const char* data)
{
  //Serial.println(data);
  if (SERIAL_FULL)
    Serial.print("Attempting to bruteforce: ");
  tweetInfo result = {"", "", "", false};
  unsigned len = (unsigned)strlen(data);
  if (len < 20) {
    if (SERIAL_FULL)
      Serial.println("no data!");
    return result;
  }
  char* indxS = strstr(data, "created_at");
  char* indxF;
  len = (unsigned)strlen(indxS);
  if (len > 10) { // 30 just to be safe
    indxF = strstr(data, "id");
    // indxS should contain something like this:
    // created_at":"Thu May 24 19:12:05 +0000 2018","id
    // <created_at":"> = 13 symbols
    // <","> = 3 symbols
    int tempo = indxF - indxS - 13 - 3;
    char datesub[tempo + 1]; // + 1 for terminator symbol
    for (int i = 0; i < tempo; i++)
      datesub[i] = indxS[i + 13];
    datesub[tempo] = '\0';
    result.date = String(datesub);
    if (SERIAL_FULL)
     Serial.print("1. ");
  }

  indxS = strstr(data, "id");
  len = (unsigned)strlen(indxS);
  if (len > 10) {
    indxF = strstr(data, "id_str");
    int tempo = indxF - indxS - 4 - 2;
    char idsub[tempo + 1]; // + 1 for terminator symbol
    for (int i = 0; i < tempo; i++)
      idsub[i] = indxS[i + 4];
    idsub[tempo] = '\0';
    result.id = String(idsub);
    if (SERIAL_FULL)
      Serial.print("2. ");
  }

  indxS = strstr(data, "text");
  len = (unsigned)strlen(indxS);
  if (len > 10) {
    indxF = strstr(data, "truncated");
    int tempo = indxF - indxS - 7 - 3;
    char textsub[tempo + 1]; // + 1 for terminator symbol
    for (int i = 0; i < tempo; i++)
      textsub[i] = indxS[i + 7];
    textsub[tempo] = '\0';
    result.message = String(textsub);
    if (SERIAL_FULL)
      Serial.print("3. ");
  }


  if (result.message != "")
  {
    result.valid = true;
    if (SERIAL_FULL)
      Serial.println("Bruteforce success!");
  }
  else
    if (SERIAL_FULL)
      Serial.println("Bruteforce failed :(");

  return result;
}

String urlencode(String str)
{
  String encodedString = "";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encodedString += '+';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
        code0 = c - 10 + 'A';
      }
      code2 = '\0';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
      //encodedString+=code2;
    }
    yield();
  }
  return encodedString;
}

String utf8rus(String source)
{
  int i,k;
  String target;
  unsigned char n;
  char m[2] = { '0', '\0' };

  k = source.length(); i = 0;

  while (i < k) {
    n = source[i]; i++;

    if (n >= 0xC0) {
      switch (n) {
        case 0xD0: {
          n = source[i]; i++;
          if (n == 0x81) { n = 0xA8; break; }
          if (n >= 0x90 && n <= 0xBF) n = n + 0x30;
          break;
        }
        case 0xD1: {
          n = source[i]; i++;
          if (n == 0x91) { n = 0xB8; break; }
          if (n >= 0x80 && n <= 0x8F) n = n + 0x70;
          break;
        }
      }
    }
    m[0] = n; target = target + String(m);
  }
return target;
}




