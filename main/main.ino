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
    display.setTextSize(2);
    display.setCursor(20, 20);
    display.print(".");
    display.display();
    delay(200);
    display.print(".");
    display.display();
    delay(200);
    display.print(".");
    display.display();
    delay(200);
    sendToPeer(menuSelected->opts[menuSelected->selected], espAddr);
    displayCurrMenu();
  }
}
