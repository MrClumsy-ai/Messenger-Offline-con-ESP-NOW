#include <esp_now.h>
#include <WiFi.h>

typedef struct struct_message {
  char a[32];
  int b;
  float c;
  bool d;
} struct_message;

struct_message myData;

void OnDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("data recieved: ");
  Serial.println(len);
  Serial.print("character recieved: ");
  Serial.println(myData.a);
  Serial.print("integer recieved: ");
  Serial.println(myData.b);
  Serial.print("float recieved: ");
  Serial.println(myData.c);
  Serial.print("bool recieved: ");
  Serial.println(myData.d);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("error initializing ESP NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
}
