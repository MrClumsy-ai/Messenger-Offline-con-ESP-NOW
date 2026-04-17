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

uint8_t fontSize = 1;

std::vector<std::array<String, 2>> history;
std::vector<String> them;
std::vector<String> us;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool submit_pressed = false;

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
  history.push_back({ "them", String(buffer) });
  them.push_back(String(buffer));
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("(!)");
  display.println(them.back());
  display.display();
  delay(1000);  // prevent multiple triggers
  displayCurrMenu();
}

bool deliveredGood;

void sentCallback(const esp_now_send_info_t *tx_info, esp_now_send_status_t status) {
  // Extract MAC address from the new structure
  const uint8_t *macAddr = tx_info->des_addr;  // des_addr is the destination MAC
  char macStr[18];
  formatMacAddress(macAddr, macStr, 18);
  Serial.print("Last packet sent to: ");
  Serial.println(macStr);
  if (status == ESP_NOW_SEND_SUCCESS) {
    deliveredGood = true;
    Serial.println("delivery success");
  } else {
    deliveredGood = false;
    Serial.println("delivery fail");
  }
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

struct Menu;

struct MenuOption {
  const char *title;
  const bool isDir;
  union {
    // make command and args doable together
    struct {
      const char *command;
      const char *args;
    } cmd;
    Menu *subDir;
  };
  void execute() const {
    if (strcmp("send", this->cmd.command) == 0) {
      sendToPeer(this->cmd.args, espAddr);
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(0, 0);
      if (deliveredGood) {
        display.printf("delivery\nsuccess");
        history.push_back({ "us", this->cmd.args });
        us.push_back(this->cmd.args);
        display.display();
      } else {
        display.printf("delivery\nfail");
        display.display();
      }
      delay(1000);
    } else if (strcmp("font", this->cmd.command) == 0) {
      fontSize = atoi(this->cmd.args);
      display.setTextSize(fontSize);
    } else if (strcmp("back", this->cmd.command) == 0) {
      // current dir -> dir of parent
    } else if (strcmp("hist", this->cmd.command) == 0) {
      // display history
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print(this->cmd.args);
      display.println(" history");
      if (strcmp("Us", this->cmd.args) == 0) {
        for (int i = us.size() - 1; i >= 0; i--) {
          display.printf("(%d): ", i);
          display.println(us[i]);
        }
      } else if (strcmp("Them", this->cmd.args) == 0) {
        for (int i = them.size() - 1; i >= 0; i--) {
          display.printf("(%d): ", i);
          display.println(them[i]);
        }
      } else if (strcmp("General", this->cmd.args) == 0) {
        for (int i = history.size() - 1; i >= 0; i--) {
          display.printf("(%d) ", i);
          display.print(history[i][0]);
          display.printf(": ");
          display.println(history[i][1]);
        }
      }
      display.display();
      delay(2500);
    }
  }
  // puede ser o un cmd, o un dir (con titulo)
  MenuOption(const char *cmd, const char *args = nullptr) : title(args), isDir(false) {
    this->cmd.command = cmd;
    this->cmd.args = args;
  }
  MenuOption(const char *title, Menu *menu) : title(title), isDir(true), subDir(menu) {}
};

struct Menu {
  const char *title;
  int selected;
  const Menu *parent;
  const int optsLen;
  const MenuOption *opts;
  void show(uint8_t padTop = 0) {
    display.setTextSize(fontSize);
    // displays 3, then scrolls
    for (int i = 3 * (this->selected / 3); i < (3 * (this->selected / 3)) + 3; i++) {
      if (i >= this->optsLen) { break; }
      display.setCursor(10, i % 3 * fontSize * 10 + padTop);
      display.println(opts[i].title);
      if (this->selected % 3 + 1 == 0) { break; }
    }
    display.setCursor(0, this->selected % 3 * 10 * fontSize + padTop);
    display.println(">");
  }
  Menu(const char *title, const Menu *parent, const int optsLen, const MenuOption *opts)
    : title(title), selected(0), parent(parent), optsLen(optsLen), opts(opts) {}
};

extern Menu emoticonsMenu;
extern Menu historyMenu;
extern Menu quickMsg;

MenuOption mainOpts[] = {
  MenuOption("Quick Msg", &quickMsg),
  MenuOption("Emoticons", &emoticonsMenu),
  MenuOption("History", &historyMenu),
};
Menu mainMenu("Main menu", nullptr, sizeof(mainOpts) / sizeof(MenuOption), mainOpts);

MenuOption quickMsgOpts[] = {
  MenuOption("send", "foo"),
  MenuOption("send", "bar"),
};
Menu quickMsg("Emoticons", &mainMenu, sizeof(quickMsgOpts) / sizeof(MenuOption), quickMsgOpts);

MenuOption emoticonsOpts[] = {
  MenuOption("send", ":)"),
  MenuOption("send", ":("),
  MenuOption("send", ":O"),
  MenuOption("send", ":/"),
  MenuOption("send", ":p"),
};
Menu emoticonsMenu("Emoticons", &mainMenu, sizeof(emoticonsOpts) / sizeof(MenuOption), emoticonsOpts);

MenuOption historyOpts[] = {
  MenuOption("hist", "Us"),
  MenuOption("hist", "Them"),
  MenuOption("hist", "General"),
};
Menu historyMenu("History", &mainMenu, sizeof(historyOpts) / sizeof(MenuOption), historyOpts);

Menu *menuSelected = &mainMenu;

void displayCurrMenu() {
  const uint8_t paddingLeft = 10 * fontSize;
  const uint8_t lineHeight = 10 * fontSize;
  display.clearDisplay();
  display.setTextSize(fontSize);
  // history
  display.setCursor(0, 0 * lineHeight);
  display.print("them: ");
  if (!them.empty()) { display.print(them.back()); }
  display.println();
  display.setCursor(0, 1 * lineHeight);
  display.print("us: ");
  if (!us.empty()) { display.print(us.back()); }
  display.println();
  display.println(menuSelected->title);
  menuSelected->show(3 * lineHeight);
  display.display();
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
  // display.setRotation(2);
  display.setTextSize(fontSize);  // default
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
    if (menuSelected->opts->isDir) {
      menuSelected = menuSelected->opts[menuSelected->selected].subDir;
      displayCurrMenu();
    } else {
      menuSelected->opts[menuSelected->selected].execute();
      menuSelected = &mainMenu;
      displayCurrMenu();
    }
    delay(200);
  }
}
