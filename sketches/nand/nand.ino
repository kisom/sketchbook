const int A = 10;
const int B = 11;
const int Out = 12;


void setup() {
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(Out, INPUT);
  Serial.begin(115200);
}

void printPinState(int val) {
  if (LOW == val)
    Serial.print("0");
  else if (HIGH == val)
    Serial.print("1");
  else
    Serial.print("[UNK]");
}

void NAND(int a, int b) {
  static int out = LOW;

  digitalWrite(A, a);
  digitalWrite(B, b);
  out = digitalRead(Out);

  Serial.print("NAND(");
  printPinState(a);
  Serial.print(", ");
  printPinState(b);
  Serial.print(") -> ");
  printPinState(out);
  Serial.println("");
}

void NOT(int a) {
  static int out = LOW;
  digitalWrite(A, a);
  out = digitalRead(Out);

  Serial.print("NOT(");
  printPinState(a);
  Serial.print(") -> ");
  printPinState(out);
  Serial.println("");  
}

void NAND_cycle() {
  NAND(LOW, LOW);
  delay(1000);
  NAND(LOW, HIGH);
  delay(1000);
  NAND(HIGH, LOW);
  delay(1000);
  NAND(HIGH, HIGH);
  delay(1000); 
}

void NOT_cycle() {
  NOT(LOW);
  delay(1000);
  NOT(HIGH);
  delay(1000);
}

void loop() {
  NAND_cycle();  
}




