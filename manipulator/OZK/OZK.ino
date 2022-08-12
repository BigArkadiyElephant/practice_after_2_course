#include <actuator.h>
#include <Dynamixel2Arduino.h>
#include "math.h"

#define DXL_SERIAL Serial3
#define DEBUG_SERIAL Serial
const uint8_t DXL_DIR_PIN = 22;
const float DXL_PROTOCOL_VERSION = 1.0;

Dynamixel2Arduino dx1(DXL_SERIAL, DXL_DIR_PIN);

const int servocount = 4;
const float L1 = 4.6, L2 = 10.4, L3 = 8, L4 = 16.7;

void setup()
{
  pinMode(17, INPUT);
  pinMode(18, INPUT);
  DXL_SERIAL.begin(57600);
  dx1.begin(1000000);
  dx1.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
  for (int i = 0; i < servocount; i++)
  {
    dx1.setOperatingMode(i + 1, OP_POSITION);
    dx1.setGoalVelocity(i + 1, 50);
    delay(50);
  }

  for (int i = 0; i < servocount; i++)
  {
    dx1.setGoalPosition(i + 1, 512);
    dx1.torqueOff(i+1);
    delay(1000);
  }

  Serial.begin(11200);
}


void PZK(float* x, float* y, float* z, float* angle)
{
  *z = L1 + L2 * cos(angle[1]) + L3 * cos(angle[1] + angle[2]) + L4 * cos(angle[1] + angle[2] + angle[3]);

  *x = (- L2 * sin(angle[1]) - L3 * sin(angle[1] + angle[2]) - L4 * sin(angle[1] + angle[2] + angle[3])) / sqrt(1 + pow(tan(angle[0]), 2));

  *y = (*x) * tan(angle[0]);
}


void OZK(float* x, float* y, float* z, float* angle)
{
  if ((*y) > 0 && (*x) > 0) angle[0] = atan(-(*x) / (*y)) + PI / 2;
  else if ((*y) > 0 && (*x) < 0) angle[0] = atan(-(*x) / (*y)) - PI / 2;
  else if ((*y) < 0 && (*x) > 0) angle[0] = atan(-(*x) / (*y)) - PI / 2;
  else if ((*y) < 0 && (*x) < 0) angle[0] = atan(-(*x) / (*y)) + PI / 2;
  else if ((*y) == 0) angle[0] = 0;

  float r = sqrt(pow((*x), 2) + pow((*y), 2) + pow((*z), 2));
  float gamma;

  if ((*x) != 0 || (*y) != 0)
    gamma = atan((*z) / sqrt(pow((*x), 2) + pow((*y), 2)));
  else gamma = PI / 2;

  float AC = sqrt(pow((L1 + L2), 2) + pow(r, 2) - 2 * (L1 + L2) * r * sin(gamma));

  angle[1] = 0;//эта кинематическая пара для упрощения задачи заморожена

  float t;
  t = acos((pow(AC, 2) + pow((L1 + L2), 2) - pow(r, 2)) / (2 * AC * (L1 + L2)));//вспомогательный угол

  if (AC < L3 + L4)
  {
    angle[2] = acos((pow(L4, 2) - pow(L3, 2) - pow(AC, 2)) / (2 * L3 * AC)) - t;

    if ((L3 + L4)*sin(PI / 2 - angle[2]) >= (*z) - L1 - L2) angle[3] = -AC * sin(angle[2] + t) / L4;
    else angle[3] = -AC * sin(angle[2] + t) / L4 + PI;
  }
  else
  {
    angle[2] = PI - t;
    angle[3] = 0;
  }

  if ((*x) < 0)
  {
    angle[3] = -angle[3];
  }
}

bool grow_up = true;//переменная отвечает за движение схвата по/против оси y
int counter = 0;

void loop()
{
  float x = -16, y = 0.5, z = 18;//некоторве начальные координаты
  const float dy = 0.5;//шаг по оси у

  if (grow_up == true )
  {
    y += dy * counter;
    counter++;

    if (counter > 40)
    {
      grow_up = false;
      counter = 0;
    }
  }
  else
  {
    y -= dy * counter;
    counter++;

    if (counter > 40)
    {
      grow_up = true;
      counter = 0;
    }
  }
  float* angle = new float[servocount];

  OZK(&x, &y, &z, angle);// решается обратная задача кинематики, чтобы переместить схва в нужную точку
  PZK(&x, &y, &z, angle);//решается прямая задача кинематики для отладки. Нужно понимать какие реальные координаты...
  //...имеет схват, чтобы можно было посмотреть в консоли, также нужно понимать насколько правильно была решена ОЗК

  Serial.print("  ||  углы:");
  for (int i = 0; i < servocount; i++)
  {
    angle[i] = (angle[i] + 5 / 6 * PI) * 900 / (5 * PI);
    Serial.print(angle[i]);//печатаются углы поворота сервоприводов
    Serial.print("  ");
    angle[i] = map(angle[i], -180, 180, 0, 1023);
    dx1.setGoalPosition(i + 1, angle[i]);// задются нужные углы поворота сервоприводов
  }

  delay(100);
  Serial.print("  || координаты:");//печатаются координаты
  Serial.print(x);
  Serial.print(" ");
  Serial.print(y);
  Serial.print(" ");
  Serial.println(z);

  delete[] angle;
}
