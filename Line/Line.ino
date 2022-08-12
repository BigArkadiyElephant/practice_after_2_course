#include <DynamixelWorkbench.h>

#if defined(__OPENCM904__)
#define DEVICE_NAME "3" //Dynamixel on Serial3(USART3) <-OpenCM 485EXP
#elif defined(__OPENCR__)
#define DEVICE_NAME ""
#endif

#define BAUDRATE 57600

DynamixelWorkbench dx1;

void setup()
{
  Serial.begin(9600);

  const char *log;
  uint16_t model_number = 0;

  dx1.init(DEVICE_NAME, BAUDRATE, &log);
  dx1.ping(1, &model_number, &log);
  dx1.wheelMode(1, 0, &log);
  dx1.goalPosition(1, (int32_t)0);
}

const float real_max_velocity = 282.6; // мм/с максимально возможная угловая скорость при 12 вольт
const float max_velocity = 110;// мм/с  максимальная линейная скорость, не вредящая конструкции ленты


float SerialDataRead()//прочитывает число(т.е. считывает данные, пока не встретиться пробел или конец строки)в случае ошибки возвращает 0
{
  const int maxcount = 9;//максимальное число разрядов в числе
  int realcount = 0;//реальное число разрядов в числе
  int pointposition = 0;//номер точки в дробном числе
  bool minusexist = false;

  int var[maxcount];

  int i = 0;
  while (i != maxcount)
  {
    var[i] = Serial.read();
    if (var[i] == 44 || var[i] == 46) pointposition = i;
    if (i == 0 && var[i] == 45)
    {
      minusexist = true;
      i --;
      realcount--;
    }
    if (var[i] == '\n' || var[i] == 32)
    {
      if (pointposition == 0)
        pointposition = realcount;
      break;
    }

    realcount++;
    i++;
  }

  int j = 0;
  float result = 0;

  for (j = 0; j < pointposition; j++)
  {
    if (var[j] >= 48 && var[j] <= 57)
      result = result + (var[j] - 48) * pow(10, pointposition - j - 1);
    else if (var[j] == 44 || var[j] == 46) break;
    else
      return 0;
  }

  for (int k = j + 1; k < realcount; k++)
  {
    if (var[k] >= 48 && var[k] <= 57)
      result = result + (var[k] - 48) * pow(10, j - k);
    else
      return 0;
  }

  if (minusexist) result = - result;

  return result;
}

void SerialRead(bool* mode, float* velocity, float* x)
{
  float epsilon = 0.001;
  float _mode = SerialDataRead();

  if (_mode < 1 + epsilon && _mode > 1 - epsilon)
    *mode = 1;
  else if (_mode < epsilon && _mode > -epsilon)
    *mode = 0;
  else
  {
    *x = 0;
    *velocity = 0;
    return;
  }

  (*velocity) = SerialDataRead();
  (*x) = SerialDataRead(); 

  if (*velocity < 0 && *mode == 1) *velocity = -*velocity;
  else if(*velocity < -max_velocity && *mode == 0) *velocity = -max_velocity;
  else if (*velocity > max_velocity) *velocity = max_velocity;

  if(*x < 0 && *mode == 1) *velocity = - *velocity;
}

void loop()
{
  if (Serial.available() > 0)
  {
    float velocity = 0, x = 0;
    bool mode = 1; //1 - задаётся перемещение, 0 - вращается бесконечно
    SerialRead(&mode, &velocity, &x);

    dx1.goalVelocity(1, (int32_t)(velocity * 1023 / real_max_velocity));
    if (mode)
    {
      float eps = 0.001;
      if (abs(velocity) > eps)
        delay(1000 * abs(x / velocity));
    }
    else
    {
      while (Serial.available() == 0) delay(30);
    }
    dx1.goalVelocity(1, (int32_t)0);
  }
}
