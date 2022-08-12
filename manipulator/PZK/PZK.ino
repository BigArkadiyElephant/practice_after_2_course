#include <actuator.h>
#include <Dynamixel2Arduino.h>
#include "math.h"

#define DXL_SERIAL Serial3
#define DEBUG_SERIAL Serial
#define svetodiod 11
const uint8_t DXL_DIR_PIN = 22;
const float DXL_PROTOCOL_VERSION = 1.0;

Dynamixel2Arduino dx1(DXL_SERIAL, DXL_DIR_PIN);

const int servocount = 4;
const float L1 = 4.6, L2 = 10.4, L3 = 8, L4 = 16.7;

void setup()
{
  DXL_SERIAL.begin(57600);//инициализация двигателей
  dx1.begin(1000000);
  dx1.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
  pinMode(svetodiod, OUTPUT);
  for (int i = 0; i < servocount; i++)//задаю скорость и режим работы, двигатели управляются по позиции, а не по скорости
  {
    dx1.setOperatingMode(i + 1, OP_POSITION);
    dx1.setGoalVelocity(i + 1, 50);
    delay(50);
  }

  for (int i = 0; i < servocount; i++)// задаю изначальную позицию. манипулятор будет в вертикальном положении
  {
    dx1.setGoalPosition(i + 1, 512);
  }

  delay(4000);
  for (int i = 0; i < servocount; i++)// выключаю усилие в двигателях, чтобы человек мог спокойно передвигать рабочий орган манипулятора
  {
    dx1.torqueOff(i + 1);
  }

  Serial.begin(11200);
}


void PZK(float* x, float* y, float* z, float* angle)//решение прямой задачи кинематики 
{
  *z = L1 + L2 * cos(angle[1]) + L3 * cos(angle[1] + angle[2]) + L4 * cos(angle[1] + angle[2] + angle[3]);

  /*if(angle[0]<=90 && angle[0]>=-90)
    {
    x = abs((- L2*sin(angle[1])-L3*sin(angle[1]+angle[2])-L4*sin(angle[1]+angle[2]+angle[3]))/sqrt(1+pow(tan(angle[0]),2)));
    }
    else *x = -abs((- L2*sin(angle[1])-L3*sin(angle[1]+angle[2])-L4*sin(angle[1]+angle[2]+angle[3]))/sqrt(1+pow(tan(angle[0]),2)));*/
  *x = (- L2 * sin(angle[1]) - L3 * sin(angle[1] + angle[2]) - L4 * sin(angle[1] + angle[2] + angle[3])) / sqrt(1 + pow(tan(angle[0]), 2));

  *y = (*x) * tan(angle[0]);
}

void loop()
{
  float x, y , z;
  float* angle = new float[servocount];

  Serial.print("Углы: ");
  for (int j = 0; j < servocount; j++)
  {
    angle[j] = map(dx1.getPresentPosition(j + 1), 0, 1023, -150, 150);
    Serial.print(angle[j]);
    Serial.print("  ");
    angle[j] = angle[j] * 5 / 3 * PI / 300 - 5 / 6 * PI;// нормирую углы, изначальный сигнал [0,1023], угол в радианах [0, -5/3*PI]
  }

  PZK(&x, &y, &z, angle);

  Serial.print("   Координаты: ");
  Serial.print(x);
  Serial.print(" ");
  Serial.print(y);
  Serial.print(" ");
  Serial.println(z);

  delete[] angle;
  delay(500);
}
