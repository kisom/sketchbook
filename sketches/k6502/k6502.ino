#include <SPI.h>
#include <GD.h>
#include <font8x8.h>
#include <cp437_data.h>
#include <cp437.h>
#include "cpu.h"

void setup() {
  GD.begin();
  cp437_setup();
  Serial.begin(9600);
  randomSeed(analogRead(2));
  Serial.println("BOOTING");
}

// stack manipulation

static unsigned char	program[] = {
 0xa2, 0x00, 0xa0, 0x00, 0x8a, 0x99, 0x00, 0x02,
 0x48, 0xe8, 0xc8, 0xc0, 0x10, 0xd0, 0xf5, 0x68,
 0x99, 0x00, 0x02, 0xc8, 0xc0, 0x20, 0xd0, 0xf7, 
 0x00
 };


// JSR / RTS
/*
unsigned char	program[] = {
 0x20, 0x09, 0x03, 0x20, 0x0c, 0x03, 0x20, 0x12,
 0x03, 0xa2, 0x00, 0x60, 0xe8, 0xe0, 0x05, 0xd0,
 0xfb, 0x60, 0x00, 0x00
 };
 */

// branching
/*
unsigned char	program[] = {
 0xa2, 0x08, 0xca, 0x8e, 0x00, 0x02, 0xe0, 0x03,
 0xd0, 0xf8, 0x8e, 0x01, 0x02, 0x00
 };
 */

/*
unsigned char program[] = {
  0xa5, 0xfe, 0x8d, 0x01, 0x10, 0x4c, 0x00, 0x10,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
 */

void loop() {
  static CPU  cpu(1024);
  static bool start = false;
  static bool running = true;
  static uint8_t rand;
  uint16_t i = 0;
  uint8_t val = 0;
  uint8_t s = 0;

  if (!start) {
    cpu.dump_registers();
    cpu.load(program, 0x300, sizeof(program));
    cpu.start_pc(0x300);
    start = true;
  }
  if (running) {
    rand = (uint8_t)random(0xff);
    Serial.print("rand: ");
    Serial.println(rand, HEX);
    cpu.DMA(0x00fe, rand);
    for (i = 0; i < 0xff; i++) {
      val = cpu.DMA(i);
      if (val < 0x10)
        Serial.print("0");
      Serial.print(val, HEX);
      Serial.print(" ");
      if (s++ == 15) {
        Serial.println("");
        s = 0;
      }
    }
    Serial.println("");

    running = cpu.step();
    if (!running)
      cpu.dump_registers();

  }
  delay(10);
}






