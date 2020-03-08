// Tag we search for so we aren't getting random data.
#define TAG_RGAM 0x7267616D // 'rgam'

// Get 32 bits.
uint32_t get32(const uint8_t* data) {
  uint32_t v;
  v = data[0];
  v <<= 8;
  v += data[1];
  v <<= 8;
  v += data[2];
  v <<= 8;
  v += data[3];
  return v;
}
// Get 16 bits.
uint16_t get16(const uint8_t* data) {
  uint32_t v;
  v = data[0];
  v <<= 8;
  v += data[1];
  return v;
}
// Get 8 bits.
uint8_t get8(const uint8_t* data) {
  return data[0];
}
// Put 32 bits.
void put32(uint8_t* data, const uint32_t value) {
  data[3] = (value >> 0) & 255;
  data[2] = (value >> 8) & 255;
  data[1] = (value >> 16) & 255;
  data[0] = (value >> 24) & 255;
}
// Put 16 bits.
void put16(uint8_t* data, const uint16_t value) {
  data[1] = (value >> 0) & 255;
  data[0] = (value >> 8) & 255;
}
// Put 8 bits.
void put8(uint8_t* data, const uint8_t value) {
  data[0] = value & 255;
}

enum PacketType1_t: byte {
  PACKET_TYPE_UNKNOWN = 0,
  // Data is ColorPalette32_t.
  PACKET_TYPE_COLOR_PALETTE = 1
  // Do not change order, always append new values.
  // 255 max.
};

// Packet byte structure:
// 0..3  Tag  (4 bytes)
// 4     Type (1 byte)
// 5..37 Data (32 bytes)
struct BasePacket {
  byte tag[4];
  PacketType1_t type;
  byte data[32];
};

struct PaletteColor4_t {
  uint8_t location;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

// A 32 byte data structure encoded in the `data` portion of the packet.
// Defines a color palette containing 2-7 colors.
struct ColorPalette32_t {
  byte color_count; // Min: 2, max: 7.
  PaletteColor4_t colors[7];
  byte unused[3];
};

// Basic packet manipulation:
#define getTag(data) get32(data)
#define getType(data) ((PacketType1_t)get8(data+4))
#define getPacketData(data) (data[5])
#define getColorPaletteFromPacket(data) ((ColorPalette32_t)getPacketData(data))

// Color palette manipulation:
#define getColorPaletteColorCount(data) get8(data)
#define putColorPaletteColorCount(data, value) put8(data, value)
#define offsetForPaletteColor(idx) (1 + idx * sizeof(PaletteColor4_t))
#define getColorPaletteColor(data, idx) get32(data + offsetForPaletteColor(idx))
#define putColorPaletteColor(data, color, idx) if (idx < 7) put32(data + offsetForPaletteColor(idx), color)
void putColorPalette(uint8_t* data, ColorPalette32_t palette) {
  putColorPaletteColorCount(data, palette.color_count);
  for (uint8_t i = 0; i < palette.color_count; i++) {
    uint32_t color_data = getColorPaletteColor(data, i);
    putColorPaletteColor(data, color_data, i);
  }
}

// Attempt to read a packet.
enum ParseErrorType8_t: uint8_t {
  PARSE_ERROR_TYPE_NONE = 0,
  PARSE_ERROR_TYPE_JUNK_DATA,
  PARSE_ERROR_TYPE_UNKNOWN_PACKET_TYPE,
  PARSE_ERROR_TYPE_MALFORMED_PACKET,
  PARSE_ERROR_TYPE_PALETTE_TOO_FEW_COLORS,
  PARSE_ERROR_TYPE_PALETTE_TOO_MANY_COLORS
};
ParseErrorType8_t parseString(const uint8_t *data) {
  uint32_t tag = getTag(data);
  if (tag != TAG_RGAM) {
    return PARSE_ERROR_TYPE_JUNK_DATA;
  }

  PacketType1_t packet_type = getType(data);
  switch (packet_type) {
    case 1:

      break;
    default: return PARSE_ERROR_TYPE_UNKNOWN_PACKET_TYPE;
  }

  return PARSE_ERROR_TYPE_UNKNOWN_PACKET_TYPE;
}

void runMeshtoyTests() {
  Serial.println("TESTS:");

  Serial.print("PaletteColor4_t size test: ");
  Serial.println(sizeof(PaletteColor4_t) == 4 ? "pass" : "FAIL");

  Serial.print("ColorPalette32_t size test: ");
  Serial.println(sizeof(ColorPalette32_t) == 32 ? "pass" : "FAIL");

  Serial.print("BasePacket size test: ");
  Serial.println(sizeof(BasePacket) == 37 ? "pass" : "FAIL");

  uint8_t packet[sizeof(BasePacket)] = {0};

  // put tag
  put32(packet, 0x7267616D);
  
  Serial.print("read tag test: ");
  Serial.println(getTag(packet) == 0x7267616D ? "pass" : "FAIL");

  // put type
  put8(packet+4, PACKET_TYPE_COLOR_PALETTE);

  Serial.print("read packet type test: ");
  Serial.println(getType(packet) == PACKET_TYPE_COLOR_PALETTE ? "pass" : "FAIL");

  // Color palette tests
  uint8_t color_palette_data[32] = {0};
  // put color palette
  ColorPalette32_t in_palette = {
    .color_count = 3,
    .colors = {
      (PaletteColor4_t){
        .location = 0,
        .red = 255,
        .green = 0,
        .blue = 0
      },
      (PaletteColor4_t){
        .location = 128,
        .red = 0,
        .green = 255,
        .blue = 0
      },
      (PaletteColor4_t){
        .location = 255,
        .red = 0,
        .green = 0,
        .blue = 1
      }
    }
  };

  putColorPalette(color_palette_data, in_palette);

  // ColorPalette32_t out_palette = *color_palette_data;

  Serial.print("color palette - read count: ");
  Serial.println(getColorPaletteColorCount(color_palette_data) == 3 ? "pass" : "FAIL");

  // Serial.print("color palette - get color 0:" );
  // uint32_t color_data = getColorPaletteColor(color_palette_data, 0);
  // PaletteColor4_t color = (PaletteColor4_t)color_data;
  // Serial.println(get8(&color) == 0 ? "pass" : "FAIL");
  // auto color = (PaletteColor4_t)getColorPaletteColor(color_palette_data, 0);
  // Serial.println(color->location == 0 ? "pass" : "FAIL");
}
