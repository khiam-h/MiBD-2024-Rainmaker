/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <ultrasonic.h>

#define DEFAULT_DIST 0.0
#define REPORTING_PERIOD    0.01 /* Seconds */

extern esp_rmaker_device_t *US_sensor_device;
extern esp_rmaker_param_t *distance_param;


void app_driver_init(void);
//float app_get_current_temperature();
//void ultrasonic_test();
float get_distance();