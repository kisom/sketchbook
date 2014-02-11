/*
 * /SR latch sketch.
 */

const int s_pin = 8;
const int r_pin = 9;
const int q_pin = 10;
const int qq_pin = 11;
const int set_led = 2;
const int data_led = 3;
const int out_led = 13;

void
set(void)
{
  int out;

  Serial.println("/S->0, /R->1");
  Serial.println("    (/Q should be 1)");
  digitalWrite(s_pin, LOW);
  digitalWrite(set_led, LOW);
  digitalWrite(r_pin, HIGH);
  digitalWrite(data_led, HIGH);
  delay(1000);
  out = digitalRead(q_pin);
  digitalWrite(out_led, out);
  digitalWrite(s_pin, HIGH);

  digitalWrite(set_led, HIGH);
  digitalWrite(s_pin, HIGH);
}

void
reset(void)
{
  int out;

  Serial.println("/S->0, /R->1");
  Serial.println("    (/Q should be 0)");
  digitalWrite(s_pin, HIGH);
  digitalWrite(set_led, HIGH);
  digitalWrite(r_pin, LOW);
  digitalWrite(data_led, LOW);
  out = digitalRead(q_pin);
  digitalWrite(out_led, out);
  delay(1000);
  digitalWrite(r_pin, HIGH);

  digitalWrite(data_led, HIGH);
  digitalWrite(r_pin, HIGH);
}

void
setup(void)
{
  pinMode(s_pin, OUTPUT);
  pinMode(r_pin, OUTPUT);
  pinMode(q_pin, INPUT);
  pinMode(qq_pin, INPUT);
  pinMode(set_led, OUTPUT);
  pinMode(data_led, OUTPUT);
  pinMode(out_led, OUTPUT);
  digitalWrite(s_pin, HIGH);
  digitalWrite(r_pin, HIGH);
  digitalWrite(set_led, HIGH);
  digitalWrite(data_led, HIGH);
  Serial.begin(9600);
  Serial.println("0 -> ");
  Serial.println(LOW);  
  Serial.println("1 -> ");
  Serial.println(HIGH);
}

void
loop(void)
{
  int out;
  set();
  out = digitalRead(q_pin);
  Serial.print("Q->");
  Serial.println(out);
  delay(1000);
  reset();
  out = digitalRead(q_pin);
  Serial.print("Q->");
  Serial.println(out);  
  delay(1000);
}



