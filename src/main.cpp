#include <Arduino.h>
#include <painlessMesh.h>
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>

#include "meshtoy.h"

FASTLED_USING_NAMESPACE

#define BRIGHTNESS  48
#define DATA_PIN    D2
#define LED_TYPE    WS2812
#define COLOR_ORDER RGB
#define NUM_LEDS    16
#define FRAMES_PER_SECOND  65
#define ONBOARD_PIN 2

#define MESH_PREFIX   "synchrobike"
#define MESH_PASSWORD "synchrobike"
#define MESH_PORT     5555

CRGB leds[NUM_LEDS];
CRGBPalette16 palette = RainbowColors_p;
painlessMesh mesh;

void confetti();
void receivedCallback(uint32_t from, String &msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);
void runTests();

// ARDUINO

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  // Print all debug messages.
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE );
  // Print some debug messages.
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION);
  
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
}

void loop() {
  mesh.update();
  EVERY_N_MILLISECONDS(1000/FRAMES_PER_SECOND) {
    confetti();
  }
  EVERY_N_MILLISECONDS(5000) {
    Serial.println("=====");
    runTests();
    Serial.println("=====");
  }
  FastLED.show();
}

// MESH

void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("SYSTEM: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("SYSTEM: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("SYSTEM: Changed connections %s\n",mesh.subConnectionJson().c_str());
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("SYSTEM: Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

// ANIMATIONS

void confetti()  {
  #define CONFETTI_MS 200
  fadeToBlackBy(leds, NUM_LEDS, 1);
  EVERY_N_MILLIS_I(timer, CONFETTI_MS) {
    int pos = random16(NUM_LEDS);
    leds[pos] += ColorFromPalette(palette, random8(), 255, NOBLEND);
  }
  timer.setPeriod(CONFETTI_MS);
}

// TESTS

void runTests() {
  Serial.println("TESTS:");

  uint8_t packet[32] = {0};

  // put tag
  put32(packet, 0x7267616D);
  
  Serial.print("read tag test: ");
  Serial.println(get32(packet) == 0x7267616D ? "pass" : "fail");

  // put type
  put8(packet+4, PACKET_TYPE_COLOR_PALETTE);

  Serial.print("read packet type test: ");
  Serial.println(get8(packet+4) == PACKET_TYPE_COLOR_PALETTE ? "pass" : "fail");

  
}