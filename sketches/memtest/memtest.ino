static uint8_t *mem;

const size_t MEMSIZE = 500;

void setup() {
  Serial.begin(9600);
  Serial.println("starting");
  randomSeed(analogRead(2));

  mem = (uint8_t *)malloc(MEMSIZE);
  if (mem == NULL) {
    Serial.println("failed to allocate memory");
  }
  memset(mem, 0, MEMSIZE);
  dump_memory();
}

static void
dump_memory(void)
{
  static int i;
  static int s;

  s = 0;
  Serial.println("dumping memory");
  for (i = 0; i < MEMSIZE; i++) {
    if (mem[i] < 0x10)
      Serial.print("0");
    Serial.print(mem[i], HEX);
    Serial.print(" ");
    if (s++ == 15) {
      Serial.println("");
      s = 0;
    }
  }
  Serial.println(""); 
}

void loop() {
  static bool started = false;
  static int i = 0;
  static int s = 0;
  static uint8_t r = 0;

  if (!started) {
    Serial.println("randomising memory");
    for (i = 0; i < MEMSIZE; i++) {
      r = (uint8_t)random(0xff);
      if (!(i % 100)) {
        Serial.println(i);
        Serial.println(r, HEX);
      }
      mem[i] = r;
    }

    dump_memory();
    started = true;
  }
  delay(1000);
}


