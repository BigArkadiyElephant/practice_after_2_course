#define potent A0//потенциометр
#define T1 3//пины для открывания/закрывания полевых транзисторов
#define T2 5
#define T3 6
#define T4 9

void setup()
{
  pinMode(potent, INPUT);
  pinMode(T1, OUTPUT);
  pinMode(T2, OUTPUT);
  pinMode(T3, OUTPUT);
  pinMode(T4, OUTPUT);
}
int speed = 0;//скорость вращения электродвигателя

void loop()
{
  speed = map(analogRead(potent), 0, 1023, -255, 255);

  if (speed > 0)
  {
    digitalWrite(T1, LOW);
    digitalWrite(T2, LOW);
    digitalWrite(T3, HIGH);
    digitalWrite(T4, abs(speed));
  }
  else
  {
    digitalWrite(T1, HIGH);
    digitalWrite(T2, abs(speed));
    digitalWrite(T3, LOW);
    digitalWrite(T4, LOW);
  }
}
