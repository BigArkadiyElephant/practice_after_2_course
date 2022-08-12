#define T1 3//пины для открывания/закрывания полевых транзисторов
#define T2 5
#define T3 6
#define T4 9

void setup()
{
  pinMode(T1, OUTPUT);
  pinMode(T2, OUTPUT);
  pinMode(T3, OUTPUT);
  pinMode(T4, OUTPUT);
}
bool flag = 0;//переменная, регистрирующая направление вращения
int Period = 1000;//регулируется время переключения
void loop()
{
  if (flag)
  {
    digitalWrite(T1, LOW);
    digitalWrite(T2, LOW);
    digitalWrite(T3, HIGH);
    digitalWrite(T4, HIGH);
  }
  else
  {
    digitalWrite(T1, HIGH);
    digitalWrite(T2, HIGH);
    digitalWrite(T3, LOW);
    digitalWrite(T4, LOW);
  }
  flag = !flag;
  delay(Period);
}
