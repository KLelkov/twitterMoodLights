

// All available colours will be displayed in random order
colour colourDisplayMode()
{
  int coloursDisplay[] = {0, 1, 2, 3, 4, 5, 6};
  int tempColour = 0;
  int randIndx = 0;
  for (int i = 0; i < 7; i++)
  {
    tempColour = coloursDisplay[i];
    randIndx = random(7);
    coloursDisplay[i] = coloursDisplay[randIndx];
    coloursDisplay[randIndx] = tempColour;
  }
  colour lastColour = blackColour;
  if (SERIAL_FULL)
    Serial.println("Colour display mode. The sequence is:");
  for (int i = 0; i < 7; i++)
  {
    if (SERIAL_FULL)
    {
      Serial.print(moodIndx2colour(coloursDisplay[i]).text);
      Serial.print(" ");
    }
    smoothLedTransition(lastColour, moodIndx2colour(coloursDisplay[i]), 20);
    lastColour = moodIndx2colour(coloursDisplay[i]);
    delay(1000);
  }
  if (SERIAL_FULL)
    Serial.println();
  return lastColour;
}

colour mood2colour(moodState mood)
{
  int strongestMood = -1;
  float maxValue = -1;
  if (mood.romantic > maxValue) {
    maxValue = mood.romantic;
    strongestMood = 0;
  }
  if (mood.happy > maxValue) {
    maxValue = mood.happy;
    strongestMood = 1;
  }
  if (mood.surprised > maxValue) {
    maxValue = mood.surprised;
    strongestMood = 2;
  }
  if (mood.sad > maxValue) {
    maxValue = mood.sad;
    strongestMood = 3;
  }
  if (mood.envy > maxValue) {
    maxValue = mood.envy;
    strongestMood = 4;
  }
  if (mood.angry > maxValue) {
    maxValue = mood.angry;
    strongestMood = 5;
  }
  if (mood.scared > maxValue) {
    maxValue = mood.scared;
    strongestMood = 6;
  }
  colour newColour = moodIndx2colour(strongestMood);
  return newColour;
}

colour moodIndx2colour(int moodIndx)
{
  colour newColour = {"", 0, 0, 0};
  switch (moodIndx) {
    case 0:
      newColour = romanticColour;
      break;
    case 1:
      newColour = happyColour;
      break;
    case 2:
      newColour = surprisedColour;
      break;
    case 3:
      newColour = sadColour;
      break;
    case 4:
      newColour = envyColour;
      break;
    case 5:
      newColour = angryColour;
      break;
    case 6:
      newColour = scaredColour;
      break;
    default:
      newColour = blackColour;
      break;
  }  
  return newColour;
}
