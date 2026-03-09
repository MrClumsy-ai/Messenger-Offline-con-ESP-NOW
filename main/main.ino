// ayuda con deepseek
#include <Wire.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BTN_UP 5
#define BTN_DOWN 19
#define BTN_SUBMIT 15
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint8_t menu_selected = 0;
bool submit_pressed = false;

const uint8_t espAddr[] = { 0x88, 0x57, 0x21, 0x79, 0xC1, 0x3C };  // placa 1
// const uint8_t espAddr[] = { 0x88, 0x57, 0x21, 0x79, 0x81, 0x04 };  // placa 2

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
  displayMainMenu();
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

void displayMainMenu() {
  const int line_height = 20;
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 0);
  display.println("a");
  display.setCursor(20, 20);
  display.println("b");
  display.setCursor(20, 40);
  display.println("c");
  display.setCursor(0, 20 * menu_selected);
  display.println(">");
  display.display();
}

String getMainSelected() {
  switch (menu_selected) {
    case 0:
      return "a";
    case 1:
      return "b";
    case 2:
      return "c";
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  WiFi.mode(WIFI_STA);
  Serial.print("MAC addr: ");
  Serial.println(WiFi.macAddress());
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESP-NOW init success");
    esp_now_register_recv_cb(recieveCallback);
    esp_now_register_send_cb(sentCallback);
  } else {
    Serial.println("ESP-NOW init failed");
    delay(3000);
    ESP.restart();
  }
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  Wire.begin(21, 22);  // SDA, SCL
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.setRotation(2);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(">");
  display.setCursor(20, 0);
  display.println("a");
  display.setCursor(20, 20);
  display.println("b");
  display.setCursor(20, 40);
  display.println("c");
  display.display();
}

void loop() {
  if (digitalRead(BTN_UP) == LOW) {
    if (menu_selected > 0) {
      menu_selected--;
    }
    displayMainMenu();
    delay(200);
  }
  if (digitalRead(BTN_DOWN) == LOW) {
    if (menu_selected < 2) {
      menu_selected++;
    }
    displayMainMenu();
    delay(200);
  }
  if (digitalRead(BTN_SUBMIT) == LOW) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 20);
    display.print("sent: ");
    display.print(getMainSelected());
    display.display();
    delay(1500);
    sendToPeer(getMainSelected(), espAddr);
    displayMainMenu();
  }
}
