#include <DynamixelWorkbench.h>

#if defined(__OPENCM904__)
#define DEVICE_NAME "3" //Dynamixel on Serial3(USART3)  <-OpenCM 485EXP
#elif defined(__OPENCR__)
#define DEVICE_NAME ""
#endif

#define BAUDRATE  1000000

int motor_count = 4;

DynamixelWorkbench dx1;

void setup()
{
  Serial.begin(57600);
  // while(!Serial); // Wait for Opening Serial Monitor

  const char *log;
  uint16_t model_number = 0;

  for (int i = 1; i <= motor_count; i++)
  {
    dx1.init(DEVICE_NAME, BAUDRATE, &log);
    dx1.ping(i, &model_number, &log);
    dx1.wheelMode(i, 0, &log);
  }
}

void Spin_wheel(int motor_ID, int velocity)//max_velocity = 255, velocity>0 - вперед
{
  if (motor_ID == 2 || motor_ID == 4)
    velocity = - velocity;

  dx1.goalVelocity(motor_ID, (int32_t)velocity);
}

void Move_to_vector(float* vector, int velocity)
{
  float x = vector[0];
  float y = vector[1];

  int velocity_x = velocity * x / sqrt(pow(x, 2) + pow(y, 2)); //скорость машины вдоль осей
  int velocity_y = velocity * y / sqrt(pow(x, 2) + pow(y, 2));

  int wheel_velocity[motor_count + 1]; //скорость отдельного колеса

  wheel_velocity[1] = velocity_x - velocity_y;
  wheel_velocity[2] = velocity_x + velocity_y;
  wheel_velocity[3] = velocity_x + velocity_y;
  wheel_velocity[4] = velocity_x - velocity_y;

  for (int i = 1; i <= motor_count; i++)
    Spin_wheel(i, wheel_velocity[i]);

}

void Move_on_circle(int velocity, int timer)//timer задает радиус окружности, измеряется в миллисекундах. Чем больше timer, тем больше радиус окружности
{
  float* vector = new float[2];
  vector[0] = 1;
  vector[1] = 0;
  float dx = 0.1;//увеличение координаты на каждой иттерации
  bool revers = false;//х увеличивается против своей оси
  float x, y;
  x = vector[0];
  y = vector[1];

  while (1)
  {
    Move_to_vector(vector, velocity);

    float eps = 0.001;

    if (x >= 1 - eps) revers = true;
    else if (x <= -1 + eps) revers = false;

    if (revers == false) x += dx;
    else x -= dx;

    if (revers == false) y = -sqrt((1 + eps) - pow(x, 2));
    else y = sqrt((1 + eps) - pow(x, 2));

    delay(timer);
  }

  delete[] vector;
}

void loop()
{
  int velocity = 255;
  int* vector = new int[2];

  keyboard_control(vector);

  delete[] vector;
}
