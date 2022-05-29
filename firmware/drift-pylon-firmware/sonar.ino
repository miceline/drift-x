void sonar()
{
  if (systemArm) {
    digitalWrite(TRIGPIN, LOW);                   // Set the trigger pin to low for 2uS
    delayMicroseconds(2);
    digitalWrite(TRIGPIN, HIGH);                  // Send a 10uS high to trigger ranging
    delayMicroseconds(10);
    digitalWrite(TRIGPIN, LOW);                   // Send pin low again
    int distance = pulseIn(ECHOPIN, HIGH);        // Read in times pulse
    distance = distance / 58;                     // Calculate distance from time of pulse

    if (recording)
    {
      //int ppSize = pp.Size();
      Serial.print(distance);
      Serial.print(" cm, fc: ");
      Serial.print(frameCount);
      Serial.print(", pop stack: ");
      Serial.print(pp.Size());
      int runTime = round((millis() - startTime) / 1000);
      runTime = runTime + 1;
      Serial.print(", runTime: ");
      Serial.print(runTime);
      Serial.print(", fps: ");
      Serial.println( pp.Size() / runTime);
    }

    int startFrame = 4;

    if (distance > triggerZone) frameCount --; // trigger range
    else frameCount ++;
    if (frameCount > 4) frameCount = 4; // max frame
    if (frameCount < 0) frameCount = 0; //static

    if (frameCount > 2) //steady reading
    {
      if (!recording && frameCount > 3) {
        startTime = millis();
        recording = true;
        //frameCount = frameCount - startFrame;
      }
      if (distance < acceptanceZone) {
        pp.Push(distance); // accepted criteria

      }
    } else {
      //Serial.print('.');
      if (pp.Size() > 0)
      {
        recording = false;
        int sum = 0;
        int sampleSize = 0;
        int lowestNum = acceptanceZone;
        Serial.println();
        Serial.println("Dump pp in reverse");
        while (pp.CanPop())
        {
          int v = pp.Pop();
          if (v < lowestNum) lowestNum = v;
          sum = sum + v;
          sampleSize ++;
          Serial.print(v);
          Serial.print(',');
        }
        Serial.println();
        Serial.println("Sample info...");
        int avg = sum / (sampleSize);
        int score = acceptanceZone - lowestNum;

        Serial.print("lowestNum: ");
        Serial.println(lowestNum);

        Serial.print("avg: ");
        Serial.println(avg);

        Serial.print("score: ");
        Serial.println(score);

        Serial.print("sampleSize: ");
        Serial.println(sampleSize);

        time_t now = time(nullptr);
        struct tm * timeinfo;
        timeinfo = localtime(&now);

        StaticJsonDocument<200> doc;
        doc["msgType"] = "scoring";
        doc["time"] = asctime(timeinfo);
        doc["systemArmId"] = systemArmId;
        doc["triggerZone"] = triggerZone;
        doc["acceptanceZone"] = acceptanceZone;
        doc["lowestNum"] = lowestNum;
        doc["avg"] = avg;
        doc["score"] = score;
        doc["sampleSize"] = sampleSize;
        doc["name"] = NODE_ID;
        doc["nodeId"] = mesh.getNodeId();
        doc["freeMem"] = String(ESP.getFreeHeap());

        String json_string;
        serializeJson(doc, json_string);
        mesh.sendBroadcast(json_string);
        //pp.Clear();
      }
    }
  }
}
