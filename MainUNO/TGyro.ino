#include <Wire.h>
#include "TGyro.h"
#include "Constants.h"

#define MPU6050_ACCEL_XOUT_H       0x3B
#define MPU6050_PWR_MGMT_1         0x6B
#define MPU6050_PWR_MGMT_2         0x6C
#define MPU6050_WHO_AM_I           0x75
#define MPU6050_I2C_ADDRESS        0x68

#define FS_SEL 131.0

typedef union accel_t_gyro_union
{
  struct
  {
    uint8_t x_accel_h;
    uint8_t x_accel_l;
    uint8_t y_accel_h;
    uint8_t y_accel_l;
    uint8_t z_accel_h;
    uint8_t z_accel_l;
    uint8_t t_h;
    uint8_t t_l;
    uint8_t x_gyro_h;
    uint8_t x_gyro_l;
    uint8_t y_gyro_h;
    uint8_t y_gyro_l;
    uint8_t z_gyro_h;
    uint8_t z_gyro_l;
  } reg;
  struct
  {
    int x_accel;
    int y_accel;
    int z_accel;
    int temperature;
    int x_gyro;
    int y_gyro;
    int z_gyro;
  } value;
};

void TGyro::set_last_read_angle_data(unsigned long time, float x, float y, float z, float x_gyro, float y_gyro, float z_gyro) {
  last_read_time = time;
  last_x_angle = x;
  last_y_angle = y;
  last_z_angle = z;
  last_gyro_x_angle = x_gyro;
  last_gyro_y_angle = y_gyro;
  last_gyro_z_angle = z_gyro;
}

int TGyro::read_gyro_accel_vals(uint8_t* accel_t_gyro_ptr) {
  accel_t_gyro_union* accel_t_gyro = (accel_t_gyro_union *)accel_t_gyro_ptr;
  int error = MPU6050_read(MPU6050_ACCEL_XOUT_H, (uint8_t *)accel_t_gyro, sizeof(*accel_t_gyro));

  uint8_t swap;
#define SWAP(x,y) swap = x; x = y; y = swap
  SWAP((*accel_t_gyro).reg.x_accel_h, (*accel_t_gyro).reg.x_accel_l);
  SWAP((*accel_t_gyro).reg.y_accel_h, (*accel_t_gyro).reg.y_accel_l);
  SWAP((*accel_t_gyro).reg.z_accel_h, (*accel_t_gyro).reg.z_accel_l);
  SWAP((*accel_t_gyro).reg.t_h, (*accel_t_gyro).reg.t_l);
  SWAP((*accel_t_gyro).reg.x_gyro_h, (*accel_t_gyro).reg.x_gyro_l);
  SWAP((*accel_t_gyro).reg.y_gyro_h, (*accel_t_gyro).reg.y_gyro_l);
  SWAP((*accel_t_gyro).reg.z_gyro_h, (*accel_t_gyro).reg.z_gyro_l);

  return error;
}

void TGyro::calibrate_sensors() {
  int                   num_readings = 10;
  float                 x_accel = 0;
  float                 y_accel = 0;
  float                 z_accel = 0;
  float                 x_gyro = 0;
  float                 y_gyro = 0;
  float                 z_gyro = 0;

  accel_t_gyro_union    accel_t_gyro;
  read_gyro_accel_vals((uint8_t *)&accel_t_gyro);

  for (int i = 0; i < num_readings; i++) {
    read_gyro_accel_vals((uint8_t *)&accel_t_gyro);
    x_accel += accel_t_gyro.value.x_accel;
    y_accel += accel_t_gyro.value.y_accel;
    z_accel += accel_t_gyro.value.z_accel;
    x_gyro += accel_t_gyro.value.x_gyro;
    y_gyro += accel_t_gyro.value.y_gyro;
    z_gyro += accel_t_gyro.value.z_gyro;
    delay(100);
  }
  x_accel /= num_readings;
  y_accel /= num_readings;
  z_accel /= num_readings;
  x_gyro /= num_readings;
  y_gyro /= num_readings;
  z_gyro /= num_readings;

  base_x_accel = x_accel;
  base_y_accel = y_accel;
  base_z_accel = z_accel;
  base_x_gyro = x_gyro;
  base_y_gyro = y_gyro;
  base_z_gyro = z_gyro;
}


void TGyro::CalibrationGyro()
{
  Wire.begin();
  int error;
  uint8_t c;
  error = MPU6050_read(MPU6050_WHO_AM_I, &c, 1);
  error = MPU6050_read(MPU6050_PWR_MGMT_2, &c, 1);
  MPU6050_write_reg(MPU6050_PWR_MGMT_1, 0);

  calibrate_sensors();
  set_last_read_angle_data(millis(), 0, 0, 0, 0, 0, 0);
  lastTime = millis();
}

void TGyro::UpdateGyro()
{
  accel_t_gyro_union accel_t_gyro;
  read_gyro_accel_vals((uint8_t*)&accel_t_gyro);
  unsigned long t_now = millis();

  float gyro_x = (accel_t_gyro.value.x_gyro - base_x_gyro) / FS_SEL;
  float gyro_y = (accel_t_gyro.value.y_gyro - base_y_gyro) / FS_SEL;
  float gyro_z = (accel_t_gyro.value.z_gyro - base_z_gyro) / FS_SEL;

  float accel_x = accel_t_gyro.value.x_accel;
  float accel_y = accel_t_gyro.value.y_accel;
  float accel_z = accel_t_gyro.value.z_accel;

  float accel_angle_y = atan(-1 * accel_x / sqrt(pow(accel_y, 2) + pow(accel_z, 2))) * ToGrad;
  float accel_angle_x = atan(accel_y / sqrt(pow(accel_x, 2) + pow(accel_z, 2))) * ToGrad;

  float dt = (t_now - last_read_time) / 1000.0;
  float gyro_angle_x = gyro_x * dt + last_x_angle;
  float gyro_angle_y = gyro_y * dt + last_y_angle;
  angels.angle_z = gyro_z * dt + last_z_angle;

  float unfiltered_gyro_angle_x = gyro_x * dt + last_gyro_x_angle;
  float unfiltered_gyro_angle_y = gyro_y * dt + last_gyro_y_angle;
  float unfiltered_gyro_angle_z = gyro_z * dt + last_gyro_z_angle;

  float alpha = 1 / (1 + dt);
  float angle_x = alpha * gyro_angle_x + (1.0 - alpha) * accel_angle_x;
  float angle_y = alpha * gyro_angle_y + (1.0 - alpha) * accel_angle_y;

  set_last_read_angle_data(t_now, angle_x, angle_y, angels.angle_z, unfiltered_gyro_angle_x, unfiltered_gyro_angle_y, unfiltered_gyro_angle_z);

  float x = tan(ToRad * angle_x);
  float y = tan(ToRad * angle_y);
  angels.horizontal = ToGrad * atan(x / y);
  if (y < 0)
    angels.horizontal += 90.0;
 else
    angels.horizontal -= 90.0;
  angels.vertical = ToGrad * atan(sqrt(x * x + y * y));
}

int TGyro::MPU6050_read(int start, uint8_t *buffer, int size)
{
  int i, n, error;

  Wire.beginTransmission(MPU6050_I2C_ADDRESS);
  n = Wire.write(start);
  if (n != 1)
    return (-10);

  n = Wire.endTransmission(false);
  if (n != 0)
    return (n);

  Wire.requestFrom(MPU6050_I2C_ADDRESS, size, true);
  i = 0;
  while (Wire.available() && i < size)
  {
    buffer[i++] = Wire.read();
  }
  if (i != size)
    return (-11);

  return (0);
}

int TGyro::MPU6050_write(int start, const uint8_t *pData, int size)
{
  int n, error;

  Wire.beginTransmission(MPU6050_I2C_ADDRESS);
  n = Wire.write(start);
  if (n != 1)
    return (-20);

  n = Wire.write(pData, size);
  if (n != size)
    return (-21);

  error = Wire.endTransmission(true);
  if (error != 0)
    return (error);

  return (0);
}

int TGyro::MPU6050_write_reg(int reg, uint8_t data)
{
  int error;
  error = MPU6050_write(reg, &data, 1);
  return (error);
}
