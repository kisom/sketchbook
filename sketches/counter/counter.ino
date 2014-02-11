#include <SPI.h>
#include <GD.h>
#include <font8x8.h>
#include <cp437_data.h>
#include <cp437.h>

void setup() {
  GD.begin();
  cp437_setup();
  Serial.begin(9600);
  Serial.println("starting up");
  drawstr(screen_addr(0, 0), "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ");
  delay(5000);
  clearscr();
}

static int counter = 0;

static void clearscr(void) {
 static char line[41];
 memset(line, 0x20, 40);
 for (int i = 0; i <= 30; i++)
   drawstr(screen_addr(0, i), line); 
}

static int nextaddr() {
  int addr = 0;
  static int y = 0;

  if (y >= 20) {
    clearscr();
    y = 0;
  }  
  Serial.print("screen address: ");
  Serial.print(y);
  Serial.print(" -> ");
  addr = screen_addr(0, y);
  Serial.println(addr);
  y += 2;
  return addr; 
}

void loop() {
  char ctrBuf[32];
  String ctrString = String(counter);
  ctrString.toCharArray(ctrBuf, ctrString.length()+1);
  drawstr(nextaddr(), ctrBuf);
  Serial.print("counter: ");
  Serial.println(counter);
  counter++;
  delay(1000);
}


