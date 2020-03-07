// Tag we search for to ensure we aren't necessarily getting random other messages.
#define TAG_RGAM 0x7267616D // 'rgam'

enum PacketType8_t: uint8_t {
  PACKET_TYPE_UNKNOWN = 0,
  PACKET_TYPE_COLOR_PALETTE = 1
  // Do not change order, always append new values.
  // 255 max.
};

// Packet = 32 bytes
// Packet byte structure:
// 0..1  Type
// 2..31 Data
struct BasePacket {
  PacketType8_t type;
  uint8_t data[31];
};

// Color palette, 32 bits.
struct PaletteColor32_t {
  uint8_t location;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

// Subtype of packet, defines a color palette.
// Palette contains 2-7 colors.
struct ColorPalette {
  PacketType8_t type; // always COLOR_PALETTE.
  uint8_t count; // Min: 2, max: 7.
  PaletteColor32_t colors[7];
  uint8_t unused[2];
};

enum ParseErrorType8_t: uint8_t {
  NONE = 0,
  JUNK_DATA,
  UNKNOWN_PACKET_TYPE,
  MALFORMED_PACKET,
  PALETTE_TOO_FEW_COLORS,
  PALETTE_TOO_MANY_COLORS
};

#define getTag(data) get32(data)
#define getType(data) get8(data)
// #define getCount(data) 

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

uint16_t get16(const uint8_t* data) {
  uint32_t v;
  v = data[0];
  v <<= 8;
  v += data[1];
  return v;
}

uint8_t get8(const uint8_t* data) {
  return data[0];
}

void put32(uint8_t* data, uint32_t value) {
  data[3] = (value >> 0) & 255;
  data[2] = (value >> 8) & 255;
  data[1] = (value >> 16) & 255;
  data[0] = (value >> 24) & 255;
}

void put16(uint8_t* data, uint16_t value) {
  data[1] = (value >> 0) & 255;
  data[0] = (value >> 8) & 255;
}

void put8(uint8_t* data, uint8_t value) {
  data[0] = value & 255;
}

// Attempt to read a packet.
ParseErrorType8_t parseString(const uint8_t *data) {
  uint32_t tag = get32(data);
  if (tag != TAG_RGAM) {
    return JUNK_DATA;
  }

  PacketType8_t packet_type = (PacketType8_t)get8(data+32);
  switch (packet_type) {
    case 1:

      break;
    default: return UNKNOWN_PACKET_TYPE;
  }

  return UNKNOWN_PACKET_TYPE;
}