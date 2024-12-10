/*  Temperature Sensor demo implementation using RGB LED and timer

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <sdkconfig.h>
#include <esp_rmaker_core.h>
#include <esp_rmaker_standard_types.h> 
#include <esp_rmaker_standard_params.h> 
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/task.h"


#include <ultrasonic.h>
#include <esp_err.h>

#include <app_reset.h>
#include <ws2812_led.h>
#include "app_priv.h"

 // This is the button that is used for toggling the power 
#define BUTTON_GPIO          CONFIG_EXAMPLE_BOARD_BUTTON_GPIO
#define BUTTON_ACTIVE_LEVEL  0
// This is the GPIO on which the power will be set 
#define OUTPUT_GPIO    19 


static TimerHandle_t sensor_timer;

#define DEFAULT_SATURATION  100
#define DEFAULT_BRIGHTNESS  50

#define WIFI_RESET_BUTTON_TIMEOUT       3
#define FACTORY_RESET_BUTTON_TIMEOUT    10

//define US sensor set up
#define TRIGGER_GPIO 5
#define ECHO_GPIO 18
#define MAX_DISTANCE_CM 500
extern esp_rmaker_device_t *US_sensor_device;
extern esp_rmaker_param_t *distance_param;

//declare bin capacity params
float bin_height = 100; //in cm, to change
int notif_counter = 1;
static char cap_alert[] = "Recycling Capacity above 80% !!";
static char max_cap_warning[] = "Capacity at 95% ... PLEASE EMPTY !!!";


static uint16_t g_hue;
static uint16_t g_saturation = DEFAULT_SATURATION;
static uint16_t g_value = DEFAULT_BRIGHTNESS;
static float g_temperature;





//updating rainmaker
static void app_sensor_update(TimerHandle_t handle)
{
   // ultrasonic sensor function
    ultrasonic_sensor_t sensor = {
        .trigger_pin = TRIGGER_GPIO,
        .echo_pin = ECHO_GPIO
    };
    ultrasonic_init(&sensor); 

    //testing sensor and getting distance
    float distance;
    esp_err_t res = ultrasonic_measure(&sensor, MAX_DISTANCE_CM, &distance);
    if (res != ESP_OK)
    {
        printf("Error %d: ", res);
        switch (res)
        {
            case ESP_ERR_ULTRASONIC_PING:
                printf("Cannot ping (device is in invalid state)\n");
                break;
            case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
                printf("Ping timeout (no device found)\n");
                break;
            case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
                printf("Echo timeout (i.e. distance too big)\n");
                break;
            default:
                printf("%s\n", esp_err_to_name(res));
        }
    }
    else
        printf("Distance: %0.04f cm\n", distance*100);
    
    //capacity calculation
    float max_height = bin_height-10;
    distance = distance*100;
    float capacity = ((max_height-distance)/(max_height))*100;
    //float cap_round = round(capacity);

    //udpating of capacity
    esp_rmaker_param_update_and_report(
            esp_rmaker_device_get_param_by_name(US_sensor_device, "Capacity/%"),
            esp_rmaker_float(capacity));
    
    //capacity notification cases
    if (capacity >= 80)
    {
        if (capacity < 95) 
        {
            switch (notif_counter)
            {
            case 1:
                // send notif
                esp_rmaker_raise_alert (cap_alert);

                notif_counter+= 1;
                break;

            case 10:
                //send notif
                esp_rmaker_raise_alert (cap_alert);
                notif_counter = -9;
                break;
            
            default:
                notif_counter += 1;
                break;
            }
            printf("notif count: %i \n", notif_counter);
        }
        else
        {    
            switch (notif_counter)
            {
            case 10:
                //send notif
                esp_rmaker_raise_alert (max_cap_warning);
                notif_counter = 5;
                break;

            default:
                notif_counter += 1;
                break;
            }
        }

    }

    //delay update loop
    vTaskDelay(pdMS_TO_TICKS(2000));
}



esp_err_t app_sensor_init(void)
{
    esp_err_t err = ws2812_led_init();
    if (err != ESP_OK) {
        return err;
    }

    g_temperature = DEFAULT_DIST;
    sensor_timer = xTimerCreate("app_sensor_update_tm", (REPORTING_PERIOD * 1000) / portTICK_PERIOD_MS,
                            pdTRUE, NULL, app_sensor_update);
    if (sensor_timer) {
        xTimerStart(sensor_timer, 0);
        g_hue = (100 - g_temperature) * 2;
        ws2812_led_set_hsv(g_hue, g_saturation, g_value);
        return ESP_OK;
    }
    return ESP_FAIL;
}

void app_driver_init()
{
    app_sensor_init();
    app_reset_button_register(app_reset_button_create(BUTTON_GPIO, BUTTON_ACTIVE_LEVEL),
                WIFI_RESET_BUTTON_TIMEOUT, FACTORY_RESET_BUTTON_TIMEOUT);
}

