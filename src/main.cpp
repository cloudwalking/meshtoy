#include <Arduino.h>
#include <painlessMesh.h>
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
#include "painlessmesh/base64.hpp"

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

#define TAG_PALETTE "palette"

#define MASTER 0

painlessMesh _mesh;
CRGB _mesh_indicator = CRGB::Black;
CRGB leds[NUM_LEDS];
CRGBPalette16 _mesh_palette = RainbowColors_p;
bool _has_palette = false;

void confetti();
void receivedCallback(uint32_t from, String &msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);
bool decodeMeshPalette(String buffer, CRGBPalette16* palette);
String encodeMeshPalette(CRGBPalette16 palette);
void runTests();

extern const CRGBPalette16 _palettes[];
extern const uint8_t _numPalettes;
int8_t _palettePointer = 0;

// ARDUINO

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  // Print all debug messages.
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE );
  // Print some debug messages.
  _mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION);
  
  _mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  _mesh.onReceive(&receivedCallback);
  _mesh.onNewConnection(&newConnectionCallback);
  _mesh.onChangedConnections(&changedConnectionCallback);
  _mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
}

void loop() {
  _mesh.update();
  EVERY_N_MILLISECONDS(1000/FRAMES_PER_SECOND) {
    // _has_palette set when a palette is received from the mesh.
    if (_has_palette) {
      confetti();
    }
    leds[0] = _mesh_indicator;
  }
  EVERY_N_MILLISECONDS(250) {
    if (_mesh.getNodeList().size() > 0) {
      _mesh_indicator = CRGB::Green;
    } else {
      _mesh_indicator = CRGB::Red;
    }
  }
  EVERY_N_SECONDS(6) {
    if (MASTER) {
      _palettePointer = (_palettePointer + 1) % _numPalettes;
      // _palettes[_palettePointer]
      String packet = encodeMeshPalette(_palettes[_palettePointer]);
      _mesh.sendBroadcast(packet, /*includeSelf=*/true);
    }
  }
  // EVERY_N_MILLISECONDS(5000) {
  //   Serial.println("=====");
  //   runTests();
  //   Serial.println("=====");
  // }
  FastLED.show();
}

// MESH

void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("SYSTEM: Received from %u msg=%s\n", from, msg.c_str());

  CRGBPalette16 read_palette;
  if (decodeMeshPalette(msg, &read_palette)) {
    Serial.println("Received mesh palette!");
    _has_palette = true;
    _mesh_palette = read_palette;
  }
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("SYSTEM: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("SYSTEM: Changed connections %s\n", _mesh.subConnectionJson().c_str());
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("SYSTEM: Adjusted time %u. Offset = %d\n", _mesh.getNodeTime(), offset);
}

// Returns true if a new palette was read.
bool decodeMeshPalette(String buffer, CRGBPalette16* in_palette) {
  String decoded = painlessmesh::base64::decode(buffer);
  
  Serial.print("decoded: ");
  Serial.println(decoded);

  char tag[sizeof(TAG_PALETTE)];
  memcpy(tag, decoded.c_str(), sizeof(TAG_PALETTE));
  if (strcmp(tag, TAG_PALETTE) == 0) {
    // Tag indicates this is a palette message.
    CRGB colors[16];
    memcpy(colors, decoded.c_str() + sizeof(TAG_PALETTE), sizeof(colors));
    *in_palette = CRGBPalette16(colors);
    return true;
  }

  // Doesn't look like a palette message.
  return false;
}

// Create base64 encoding of TAG + palette.entries.
String encodeMeshPalette(CRGBPalette16 palette) {
  size_t size = sizeof(TAG_PALETTE) + sizeof(palette.entries);
  unsigned char buffer[size];
  memcpy(buffer, &TAG_PALETTE, sizeof(TAG_PALETTE));
  memcpy(buffer + sizeof(TAG_PALETTE), &palette.entries, sizeof(palette.entries));

  String encoded = painlessmesh::base64::encode(buffer, size);

  Serial.println("encodeMeshPalette():");
  Serial.print("  raw buffer size: ");
  Serial.println(size);
  Serial.print("  encoded buffer size: ");
  Serial.println(encoded.length());

  return encoded;
}

// ANIMATIONS

void confetti()  {
  #define CONFETTI_MS 200
  fadeToBlackBy(leds, NUM_LEDS, 1);
  EVERY_N_MILLIS_I(timer, CONFETTI_MS) {
    int pos = random16(NUM_LEDS);
    leds[pos] += ColorFromPalette(_mesh_palette, random8(), 255, NOBLEND);
  }
  timer.setPeriod(CONFETTI_MS);
}

// TESTS

void runTests() {
  String encoded_palette = encodeMeshPalette(RainbowColors_p);
  Serial.print("encoded_palette: ");
  Serial.println(encoded_palette);

  CRGBPalette16 mesh_palette;
  if (decodeMeshPalette(encoded_palette, &mesh_palette)) {
    Serial.println("Got mesh palette!");
  } else {
    Serial.println("no mesh palette");
  }
}

// PALETTES

DEFINE_GRADIENT_PALETTE(_netfish_blue) {
  0, 0, 64, 255,
  255, 0, 16, 128
};

DEFINE_GRADIENT_PALETTE(_palette_white) {
  0, 255, 255, 255,
  255, 255, 255, 255
};

const CRGBPalette16 _palettes[] = {
  _netfish_blue,
  _palette_white,
  OceanColors_p,
  // ForestColors_p,
  // PartyColors_p,
  // RainbowColors_p,
  HeatColors_p
};
 
const uint8_t _numPalettes = sizeof(_palettes) / sizeof(CRGBPalette16);
