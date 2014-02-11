/*
 * /SR latch sketch.
 *
 * An /SR latch is active low; therefore, a 1 is represented by a LOW state,
 * while a 0 is represented by a HIGH state. The /Q pin becomes, in essence,
 * an active high output, and therefore can be used directly to set the LED
 * indicating the current bit in memory.
 *
 * Output state table:
 * +====+====+====+
 * | /S | /R |  Q |
 * +====+====+====+
 * | 0  | 0  | -  | <- This combination is not permitted.
 * +----+----+----+
 * | 0  | 1  | 1  |
 * +----+----+----+
 * | 1  | 0  | 0  |
 * +----+----+----+
 * | 1  | 1  | NC | <- Q remains unchanged.
 * +----+----+----+
 */

const int s_pin = 8;     // Input for /S.
const int r_pin = 9;     // Input for /R.
const int q_pin = 10;    // Q output. Active-low.
const int qq_pin = 11;   // /Q -- active-high translation.
const int s_led = 2;     // LED indicator for current /S status.
const int r_led = 3;     // LED indicator for current /R status.
const int out_led = 13;  // LED indicator for current bit in memory (on
// indicates 1, off indicates 0).

/*
 * An active low system represents 0 as the higher voltage, and 1 as the lower
 * voltage. Therefore, we define an A(ctive)L(ow)0 as HIGH, and AL1 as LOW.
 */
#define AL0  HIGH
#define AL1 LOW

/*
 * Set stores a 1 in the the latch.
 */
void
set(void)
{
  digitalWrite(s_pin, AL0);
  setLED(s_led, AL0);
  digitalWrite(r_pin, AL1);
  setLED(r_led, AL1);
  setDataLED();
}

/*
 * Clear stores a 0 in the latch.
 */
void
clear(void)
{
  digitalWrite(s_pin, AL1);
  setLED(s_led, AL1);
  digitalWrite(r_pin, AL0);
  setLED(r_led, AL0);
  setDataLED();
}

/*
 * Store keeps the present state of the latch.
 */
void
store(void)
{
  digitalWrite(s_pin, AL0);
  setLED(s_led, AL1);
  digitalWrite(r_pin, AL0);
  setLED(r_led, AL1);
  setDataLED();
}

/*
 * SetDataLED reads the current status off the /Q pin, and sets the
 * indicator appropriately.
 */
void
setDataLED(void)
{
  int out = digitalRead(qq_pin);

  digitalWrite(out_led, out);
}

/*
 * setLED takes a pin and its current state, and sets the appropriate indicator
 * accordingly, accounting for the fact that the LEDs on the proto-shield are
 * active high.
 */
void
setLED(int pin, int state)
{
  if (AL0 == state)
    digitalWrite(pin, LOW);
  else
    digitalWrite(pin, HIGH);
}

void setup() {
  pinMode(s_pin, OUTPUT);
  pinMode(r_pin, OUTPUT);
  pinMode(q_pin, INPUT);
  pinMode(qq_pin, INPUT);
  pinMode(s_led, OUTPUT);
  pinMode(r_led, OUTPUT);
  pinMode(out_led, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  set();
  delay(1000);
  store();
  delay(1000);
  clear();
  delay(1000);
  store();
  delay(1000);
}
