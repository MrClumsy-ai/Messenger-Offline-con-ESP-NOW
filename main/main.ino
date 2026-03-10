// ayuda con deepseek
#include <Wire.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BTN_UP 15
#define BTN_DOWN 5
#define BTN_SUBMIT 19
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// const uint8_t espAddr[] = { 0x88, 0x57, 0x21, 0x79, 0xC1, 0x3C };  // placa 1
const uint8_t espAddr[] = { 0x88, 0x57, 0x21, 0x79, 0x81, 0x04 };  // placa 2

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool submit_pressed = false;

struct Menu {
  int selected;
  const int optsLen;
  const char *const *opts;
  void show(uint8_t padTop = 0, uint8_t fontSize = 1) {
    display.setTextSize(fontSize);
    display.setCursor(0, selected * 10 * fontSize + padTop);
    display.println(">");
    for (int i = 0; i < optsLen; i++) {
      display.setCursor(10, i * fontSize * 10 + padTop);
      display.println(opts[i]);
    }
    display.display();
  }
};

const char *mainMenuOpts[] = { "a", "b", "c" };
Menu mainMenu = { 0, 3, mainMenuOpts };

Menu *menuSelected = &mainMenu;

std::vector<String> them;
std::vector<String> us;

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

void setMenu(const String opts[], uint8_t paddingTop, uint8_t fontSize) {
  display.setTextSize(fontSize);
  for (int i = 0; i < sizeof(&opts) - 1; i++) {
    display.setCursor(10, i * fontSize * 10 + paddingTop);
    display.println(opts[i]);
  }
}

void displayCurrMenu() {
  const uint8_t fontSize = 1;
  const uint8_t paddingLeft = 10;
  const uint8_t lineHeight = fontSize * 10;
  display.setTextSize(fontSize);
  display.clearDisplay();
  // history
  display.setCursor(paddingLeft, 0 * lineHeight);
  display.print("them: ");
  if (!them.empty()) {
    display.print(them.back());
  }
  display.println();
  display.setCursor(paddingLeft, 1 * lineHeight);
  display.print("us: ");
  if (!us.empty()) {
    display.print(us.back());
  }
  display.println();
  // TODO
  menuSelected->show(20, fontSize);
}

String getMainSelected() {
  switch (menu_selected) {
    case 0:
      us.push_back("a");
      return "a";
    case 1:
      us.push_back("b");
      return "b";
    case 2:
      us.push_back("c");
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
  pinMode(BTN_SUBMIT, INPUT_PULLUP);
  Wire.begin(21, 22);  // SDA, SCL
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.setTextColor(SSD1306_WHITE);
  display.setRotation(2);
  display.setTextSize(1);  // default
  displayCurrMenu();
}

void loop() {
  if (digitalRead(BTN_UP) == LOW) {
    if (menuSelected->selected > 0) {
      menuSelected->selected--;
    }
    displayCurrMenu();
    delay(200);
  }
  if (digitalRead(BTN_DOWN) == LOW) {
    if (menuSelected->selected < menuSelected->optsLen - 1) {
      menuSelected->selected++;
    }
    displayCurrMenu();
    delay(200);
  }
  if (digitalRead(BTN_SUBMIT) == LOW) {
    display.clearDisplay();

    display.clearDisplay();
    display.print(".");
    display.display();
    delay(250);
    display.setCursor(20, 20);
    display.clearDisplay();
    display.print("..");
    display.display();
    delay(250);
    display.setCursor(20, 20);
    display.clearDisplay();
    display.print("...");
    display.display();
    delay(250);
    sendToPeer(menuSelected->opts[menuSelected->selected], espAddr);
    displayCurrMenu();
  }
}
