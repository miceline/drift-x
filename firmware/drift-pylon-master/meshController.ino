// Needed for painless library
void broadcastToNodes(String &json_string ) {
  Serial.print("[mesh-loop] broadcast: ");
  Serial.println(json_string);
  mesh.sendBroadcast(json_string);
}

void receivedCallback( uint32_t from, String &msg ) {
  Serial.println(msg.c_str());
  //Serial.printf("INCOMMING: Received from %u msg=%s\n", from, msg.c_str());
  //ws.textAll(msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
//  Serial.printf("Changed connections %s\n", mesh.subConnectionJson().c_str());
//  nodes = mesh.getNodeList();
//
//  Serial.printf("Num nodes: %d\n", nodes.size());
//  Serial.printf("Connection list:");
//
//  SimpleList<uint32_t>::iterator node = nodes.begin();
//  while (node != nodes.end()) {
//    Serial.printf(" %u", *node);
//    node++;
//  }
//  Serial.println();
  calc_delay = true;
  sendSerialNodeNotification();
}


void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
  Serial.println();
}

void delayReceivedCallback(uint32_t from, int32_t delay) {
  Serial.printf("Delay to node %u is %d us\n", from, delay);
  Serial.println();
}
