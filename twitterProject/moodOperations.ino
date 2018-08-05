void displayMood(moodState mood);

// calculate which mood has the highest percent of tweets
moodState calculateMood(moodState tweetNums)
{
  moodState mood = {0, 0, 0, 0, 0, 0, 0};
  int allTweets = tweetNums.happy + tweetNums.sad + tweetNums.romantic +
    tweetNums.surprised + tweetNums.scared + tweetNums.angry + tweetNums.envy;
  if (allTweets <= 0)
    return tweetNums;
  mood.happy = tweetNums.happy / allTweets;
  mood.sad = tweetNums.sad / allTweets;
  mood.romantic = tweetNums.romantic / allTweets;
  mood.surprised = tweetNums.surprised / allTweets;
  mood.scared = tweetNums.scared / allTweets;
  mood.angry = tweetNums.angry / allTweets;
  mood.envy = tweetNums.envy / allTweets;
  if (SERIAL_FULL || SERIAL_COMPACT)
  {
    Serial.print("Total tweets found: ");
    Serial.println(allTweets);
    displayMood(mood);
  }

  return mood;
}


// display contents of the mood structure
void displayMood(moodState mood)
{
  Serial.print("Happy: ");
  Serial.println(mood.happy);
  Serial.print("Sad: ");
  Serial.println(mood.sad);
  Serial.print("Romantic: ");
  Serial.println(mood.romantic);
  Serial.print("Surprised: ");
  Serial.println(mood.surprised);
  Serial.print("Scared: ");
  Serial.println(mood.scared);
  Serial.print("Angry: ");
  Serial.println(mood.angry);
  Serial.print("Envy: ");
  Serial.println(mood.envy);
}

// calculate how much exactly each mood has changed
moodState getDiviation(moodState oldState, moodState newState)
{
  moodState mood = {0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01};
  if (oldState.happy != 0)
    mood.happy = (newState.happy - oldState.happy) / oldState.happy;
  else
    mood.happy = newState.happy;
  if (oldState.sad != 0)
    mood.sad = (newState.sad - oldState.sad) / oldState.sad;
  else
    mood.sad = newState.sad;
  if (oldState.romantic != 0)
    mood.romantic = (newState.romantic - oldState.romantic) / oldState.romantic;
  else
    mood.romantic = newState.romantic;
  if (oldState.surprised != 0)
    mood.surprised = (newState.surprised - oldState.surprised) / oldState.surprised;
  else
    mood.surprised = newState.surprised;
  if (oldState.scared != 0)
    mood.scared = (newState.scared - oldState.scared) / oldState.scared;
  else
    mood.scared = newState.scared;
  if (oldState.angry != 0)
    mood.angry = (newState.angry - oldState.angry) / oldState.angry;
  else
    mood.angry = newState.angry;
  if (oldState.envy != 0)
    mood.envy = (newState.envy - oldState.envy) / oldState.envy;
  else
    mood.envy = newState.envy;
  if (SERIAL_FULL || SERIAL_COMPACT)
    displayMood(mood);
  return mood;
}
