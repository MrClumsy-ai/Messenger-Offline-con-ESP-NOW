// referencia: https://www.youtube.com/watch?v=bEKjCDDUPaU&t=1649s
#include <esp_now.h>
#include <WiFi.h>

int int_value;
float float_value;
bool bool_value;

// (cambiar el MAC address de tu placa aqui)
// Placa 1: 88:57:21:79:C1:3C 
// Placa 2: 88:57:21:79:81:04
// de la 1 a la 2
uint8_t broadcastAddress[] = { 0x88, 0x57, 0x21, 0x79, 0x81, 0x04 }; // el MAC en hexadecimal de la placa 2 (receptora)
typedef struct struct_message {
  char a[32];
  int b;
  float c;
  bool d;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

void OnDataSent(const wifi_tx_info_t *foo, esp_now_send_status_t status) {
  Serial.print("\r\nLast packet send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery succes" : "Delivery fail");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("failed to add peer");
    return;
  }
}

void loop() {
  int_value = random(1, 20);
  float_value = 1.3 * int_value;
  bool_value = !bool_value;
  strcpy(myData.a, "welcome to the poya");
  myData.b = int_value;
  myData.c = float_value;
  myData.d = bool_value;

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
  if (result == ESP_OK) {
    Serial.println("sending confirmed");
  } else {
    Serial.println("Sending error");
  }
  delay(2000);
}
