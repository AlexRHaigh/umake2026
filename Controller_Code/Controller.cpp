#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

// Button pins
#define BTN_UP 32
#define BTN_RIGHT 33
#define BTN_LEFT 25
#define BTN_DOWN 26

// debounce time
const int debounceDelay = 50;

// Receiver MAC
uint8_t receiverMAC[] = {0x70, 0x4B, 0xCA, 0x8E, 0x9F, 0x48};

// Message structure
typedef struct {
  char button[10];
} ButtonMessage;

ButtonMessage msg;

// Store our MAC
String myMAC;

// Button states
bool upState = HIGH;
bool downState = HIGH;
bool leftState = HIGH;
bool rightState = HIGH;

bool lastUpReading = HIGH;
bool lastDownReading = HIGH;
bool lastLeftReading = HIGH;
bool lastRightReading = HIGH;

unsigned long lastUpTime = 0;
unsigned long lastDownTime = 0;
unsigned long lastLeftTime = 0;
unsigned long lastRightTime = 0;


// NEW ESP-NOW SEND CALLBACK (required by new ESP32 cores)
void onDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {

  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");

  if (info) {
    Serial.printf("Dest MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
      info->des_addr[0], info->des_addr[1], info->des_addr[2],
      info->des_addr[3], info->des_addr[4], info->des_addr[5]);
  }
}


// Send button message
void sendButton(const char *name) {

  strcpy(msg.button, name);

  esp_err_t result = esp_now_send(receiverMAC, (uint8_t *)&msg, sizeof(msg));

  if (result == ESP_OK) {
    Serial.print("Sent: ");
    Serial.println(name);
  } else {
    Serial.println("Error sending message");
  }
}


// Debounced button check
void checkButton(int pin, bool &state, bool &lastReading, unsigned long &lastTime, const char *name) {

  bool reading = digitalRead(pin);

  if (reading != lastReading) {
    lastTime = millis();
  }

  if ((millis() - lastTime) > debounceDelay) {

    if (reading != state) {
      state = reading;

      if (state == LOW) {
        sendButton(name);
      }
    }
  }

  lastReading = reading;
}


void setup() {

  Serial.begin(115200);

  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);

  // ESP-NOW requires station mode
  WiFi.mode(WIFI_STA);
  WiFi.begin();      // start WiFi driver
  delay(100);        // give it time to initialize
  myMAC = WiFi.macAddress();

  Serial.print("My MAC Address: ");
  Serial.println(myMAC);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(onDataSent);

  // Add peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("ESP-NOW Ready");
}


void loop() {
  checkButton(BTN_UP, upState, lastUpReading, lastUpTime, "UP");
  checkButton(BTN_DOWN, downState, lastDownReading, lastDownTime, "DOWN");
  checkButton(BTN_LEFT, leftState, lastLeftReading, lastLeftTime, "LEFT");
  checkButton(BTN_RIGHT, rightState, lastRightReading, lastRightTime, "RIGHT");

}