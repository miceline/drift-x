void serialController(String incomingByte) {
  //  // say what you got:
  //    Serial.print(">:|");
  //    Serial.print(incomingByte);
  //    Serial.println("|");
  StaticJsonDocument<200> input;
  DeserializationError err = deserializeJson(input, incomingByte);
  switch (err.code()) {
    case DeserializationError::Ok:
      //            Serial.print(F(">Deserialization succeeded"));
      break;
    case DeserializationError::InvalidInput:
      Serial.print(F(">Invalid input!\n"));
      break;
    case DeserializationError::NoMemory:
      Serial.print(F(">Not enough memory\n"));
      break;
    default:
      Serial.print(F(">Deserialization failed\n"));
      break;
  }
  String action = input["action"];
  if (action == "arm") //{"action":"arm"}
  {
    StaticJsonDocument<200> doc;
    doc["systemArmId"] = input["armId"];
    doc["action"] = "arm_system";

    String json_string;
    serializeJson(doc, json_string);
    //      Serial.print("broadcast: ");
    //      Serial.println(json_string);
    broadcastToNodes(json_string);

  } else if (action == "disarm") //{"action":"disarm"}
  {
    StaticJsonDocument<200> doc;
    doc["action"] = "dis_arm_system";

    String json_string;
    serializeJson(doc, json_string);
    broadcastToNodes(json_string);
  } else if (action == "read_config") //{"action":"read_config"}
  {
    StaticJsonDocument<200> doc;
    doc["action"] = "read_config";

    String json_string;
    serializeJson(doc, json_string);
    broadcastToNodes(json_string);
  } else if (action == "config") //{"action":"config"}
  {
    StaticJsonDocument<200> doc;
    doc["action"] = "set_config";
    doc["triggerZone"] = 160;//170
    doc["acceptanceZone"] = 140; //150
    doc["masterNode"] = mesh.getNodeId();

    String json_string;
    serializeJson(doc, json_string);
    broadcastToNodes(json_string);
  } else if (action == "ping") //{"action":"ping"}
  {

    JsonVariant _hr = input["hr"];
    JsonVariant _min = input["min"];
    JsonVariant _sec = input["sec"];
    JsonVariant _day = input["day"];
    JsonVariant _month = input["month"];
    JsonVariant _yr = input["yr"];

    struct tm tm;
    tm.tm_year = _yr.as<int>() - 1900;
    tm.tm_mon = _month.as<int>();
    tm.tm_mday = _day.as<int>();
    tm.tm_hour = _hr.as<int>();
    tm.tm_min = _min.as<int>();
    tm.tm_sec = _sec.as<int>();

    time_t t = mktime(&tm);
    struct timeval now = { .tv_sec = t };
    settimeofday(&now, NULL);

    String json_string;
    serializeJson(input, json_string);
    //      Serial.print("broadcast: ");
    //      Serial.println(json_string);
    broadcastToNodes(json_string);

  }else if (action == "topology_refresh") //{"action":"topology_refresh"}
  {
    sendSerialNodeNotification();
  }else if (action == "version") //{"action":"version"}
  {
    StaticJsonDocument<200> doc;
    doc["msgType"] = "version";
    doc["version"] = "0.0.1";
    doc["boardType"] = "master";

    String json_string;
    serializeJson(doc, json_string);
    Serial.println(json_string);
  }
    
//  }else if (action == "file_transfer_start")
//  {
//    int fileSize = input["fileSize"];
//    if (Serial.available())
//    {
//      for (int i = 0; i <= fileSize; i++)
//      {
//        //fileTransfer = Serial.read();
//      }
//    }
//    Serial.print("File Size: ");
//    Serial.print(fileTransfer);
//    Serial.println(". . .");
//  }
}

void sendSerialNodeNotification() {

  StaticJsonDocument<200> doc;
  StaticJsonDocument<200> docTopology;
  //deserializeJson(docTopology, mesh.subConnectionJson());
  JsonArray arrayNodes = docTopology.to<JsonArray>();
  doc["msgType"] = "node-update";
  doc["parentNodeId"] = mesh.getNodeId();
  doc["timeZone"] = tzname[0];
  //doc["time"] = asctime(timeinfo);
 // doc["name"] = NODE_ID;
  doc["nodeId"] = mesh.getNodeId();
  

  nodes = mesh.getNodeList();

  Serial.printf("Num nodes: %d\n", nodes.size());
  Serial.printf("Connection list:");
  //Serial.println(mesh.subConnectionJson());

  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end()) {
    Serial.printf(" %u", *node);
    arrayNodes.add(*node);
    //StaticJsonDocument<200> docNode;
    //docNode["nodeId"] = 11;
    //docTopology.add(docNode);
//    doc["timeZone"] = tzname[0];
//    doc["time"] = asctime(timeinfo);
//    doc["name"] = NODE_ID;
//    doc["nodeId"] = mesh.getNodeId();
    
    node++;
  }
  doc["topology"] = arrayNodes;
  Serial.println();
  calc_delay = true;

  String json_string;
  serializeJson(doc, json_string);
  Serial.println(json_string);
}
