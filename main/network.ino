
void formatMacAddress(const uint8_t *macAddr, char *buffer, int maxLen) {
  snprintf(buffer, maxLen, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
}

void recieveCallback(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int dataLen) {
  // Extract MAC address from the new structure
  const uint8_t *macAddr = esp_now_info->src_addr;
  char buffer[ESP_NOW_MAX_DATA_LEN + 1];
  int msgLen = min(ESP_NOW_MAX_DATA_LEN, dataLen);
  strncpy(buffer, (const char *)data, msgLen);
  buffer[msgLen] = 0;
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);
  Serial.printf("Received message from: %s - %s\n", macStr, buffer);
  them.push_back(String(buffer));
  display.clearDisplay();
  display.setCursor(10, 20);
  if (strcmp("a", buffer) == 0) {
    display.println("got a");
    Serial.println("got a");
  } else if (strcmp("b", buffer) == 0) {
    display.println("got b");
    Serial.println("got b");
  } else {
    display.println("got c");
    Serial.println("got c");
  }
  display.display();
  delay(1000);  // prevent multiple triggers
  displayCurrMenu();
}

void sentCallback(const esp_now_send_info_t *tx_info, esp_now_send_status_t status) {
  // Extract MAC address from the new structure
  const uint8_t *macAddr = tx_info->des_addr;  // des_addr is the destination MAC
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);
  Serial.print("Last packet sent to: ");
  Serial.println(macStr);
  Serial.print("Last packet send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void sendToPeer(const String &message, const uint8_t *peerAddress) {
  // if peer hasn't been added, add it
  if (!esp_now_is_peer_exist(peerAddress)) {
    esp_now_peer_info_t peerInfo = {};
    memcpy(&peerInfo.peer_addr, peerAddress, 6);
    peerInfo.channel = 0;  // wifi channel
    peerInfo.encrypt = false;
    esp_err_t addStatus = esp_now_add_peer(&peerInfo);
    if (addStatus != ESP_OK) {
      Serial.println("FAILED TO ADD PEER");
      return;
    }
  }
  esp_err_t result = esp_now_send(peerAddress, (const uint8_t *)message.c_str(), message.length());
  switch (result) {
    case ESP_OK:
      Serial.println("broadcast message success");
      break;
    case ESP_ERR_ESPNOW_NOT_INIT:
      Serial.println("ESP-NOW not init");
      break;
    case ESP_ERR_ESPNOW_ARG:
      Serial.println("invalid argument");
      break;
    case ESP_ERR_ESPNOW_INTERNAL:
      Serial.println("internal error");
      break;
    case ESP_ERR_ESPNOW_NO_MEM:
      Serial.println("ESP_ERR_ESPNOW_NO_MEM");
      break;
    case ESP_ERR_ESPNOW_NOT_FOUND:
      Serial.println("peer not found");
      break;
    default:
      Serial.println("unknown error");
      break;
  }
}
