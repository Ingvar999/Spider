#pragma once

#ifndef _Gyro_h_
#define _Gyro_h_

#include "AnglesStruct.h"

class TGyro
{
  public:
    volatile struct Angels angels;

    void CalibrationGyro();
    void UpdateGyro();

  private:
    unsigned long lastTime;
    unsigned long last_read_time;

    float    last_x_angle;
    float    last_y_angle;
    float    last_z_angle;

    float    last_gyro_x_angle;
    float    last_gyro_y_angle;
    float    last_gyro_z_angle;

    float    base_x_accel;
    float    base_y_accel;
    float    base_z_accel;

    float    base_x_gyro;
    float    base_y_gyro;
    float    base_z_gyro;

    void set_last_read_angle_data(unsigned long time, float x, float y, float z, float x_gyro, float y_gyro, float z_gyro);
    int read_gyro_accel_vals(uint8_t* accel_t_gyro_ptr);
    void calibrate_sensors();
    int MPU6050_read(int start, uint8_t *buffer, int size);
    int MPU6050_write(int start, const uint8_t *pData, int size);
    int MPU6050_write_reg(int reg, uint8_t data);
};

#endif

