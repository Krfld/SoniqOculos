/* Deep sleep wake up example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp32/ulp.h"
#include "driver/touch_pad.h"
#include "driver/adc.h"
#include "driver/rtc_io.h"
#include "soc/sens_periph.h"
#include "soc/rtc.h"

static RTC_DATA_ATTR struct timeval sleep_enter_time;

#ifdef CONFIG_EXAMPLE_ULP_TEMPERATURE_WAKEUP
#if CONFIG_IDF_TARGET_ESP32

/*
 * Offset (in 32-bit words) in RTC Slow memory where the data is placed
 * by the ULP coprocessor. It can be chosen to be any value greater or equal
 * to ULP program size, and less than the CONFIG_ESP32_ULP_COPROC_RESERVE_MEM/4 - 6,
 * where 6 is the number of words used by the ULP coprocessor.
 */
#define ULP_DATA_OFFSET 36

_Static_assert(ULP_DATA_OFFSET < CONFIG_ESP32_ULP_COPROC_RESERVE_MEM / 4 - 6,
               "ULP_DATA_OFFSET is set too high, or CONFIG_ESP32_ULP_COPROC_RESERVE_MEM is not sufficient");

/**
 * @brief Start ULP temperature monitoring program
 *
 * This function loads a program into the RTC Slow memory and starts up the ULP.
 * The program monitors on-chip temperature sensor and wakes up the SoC when
 * the temperature goes lower or higher than certain thresholds.
 */
static void start_ulp_temperature_monitoring(void);

/**
 * @brief Utility function which reads data written by ULP program
 *
 * @param offset offset from ULP_DATA_OFFSET in RTC Slow memory, in words
 * @return lower 16-bit part of the word writable by the ULP
 */
static inline uint16_t ulp_data_read(size_t offset)
{
    return RTC_SLOW_MEM[ULP_DATA_OFFSET + offset] & 0xffff;
}

/**
 * @brief Utility function which writes data to be ready by ULP program
 *
 * @param offset offset from ULP_DATA_OFFSET in RTC Slow memory, in words
 * @param value lower 16-bit part of the word to be stored
 */
static inline void ulp_data_write(size_t offset, uint16_t value)
{
    RTC_SLOW_MEM[ULP_DATA_OFFSET + offset] = value;
}
#endif // CONFIG_IDF_TARGET_ESP32
#endif // CONFIG_EXAMPLE_ULP_TEMPERATURE_WAKEUP

void app_main(void)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    int sleep_time_ms = (now.tv_sec - sleep_enter_time.tv_sec) * 1000 + (now.tv_usec - sleep_enter_time.tv_usec) / 1000;

    switch (esp_sleep_get_wakeup_cause())
    {
#ifdef CONFIG_EXAMPLE_EXT1_WAKEUP
    case ESP_SLEEP_WAKEUP_EXT1:
    {
        uint64_t wakeup_pin_mask = esp_sleep_get_ext1_wakeup_status();
        if (wakeup_pin_mask != 0)
        {
            int pin = __builtin_ffsll(wakeup_pin_mask) - 1;
            printf("Wake up from GPIO %d\n", pin);
        }
        else
        {
            printf("Wake up from GPIO\n");
        }
        break;
    }
#endif // CONFIG_EXAMPLE_EXT1_WAKEUP
    case ESP_SLEEP_WAKEUP_TIMER:
    {
        printf("Wake up from timer. Time spent in deep sleep: %dms\n", sleep_time_ms);
        break;
    }
#ifdef CONFIG_EXAMPLE_TOUCH_WAKEUP
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
    {
        printf("Wake up from touch on pad %d\n", esp_sleep_get_touchpad_wakeup_status());
        break;
    }
#endif // CONFIG_EXAMPLE_TOUCH_WAKEUP
#ifdef CONFIG_EXAMPLE_ULP_TEMPERATURE_WAKEUP
#if CONFIG_IDF_TARGET_ESP32
    case ESP_SLEEP_WAKEUP_ULP:
    {
        printf("Wake up from ULP\n");
        int16_t diff_high = (int16_t)ulp_data_read(3);
        int16_t diff_low = (int16_t)ulp_data_read(4);
        if (diff_high < 0)
        {
            printf("High temperature alarm was triggered\n");
        }
        else if (diff_low < 0)
        {
            printf("Low temperature alarm was triggered\n");
        }
        else
        {
            assert(false && "temperature has stayed within limits, but got ULP wakeup\n");
        }
        break;
    }
#endif // CONFIG_IDF_TARGET_ESP32
#endif // CONFIG_EXAMPLE_ULP_TEMPERATURE_WAKEUP
    case ESP_SLEEP_WAKEUP_UNDEFINED:
    default:
        printf("Not a deep sleep reset\n");
    }

#ifdef CONFIG_EXAMPLE_ULP_TEMPERATURE_WAKEUP
#if CONFIG_IDF_TARGET_ESP32
    if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_UNDEFINED)
    {
        printf("ULP did %d temperature measurements in %d ms\n", ulp_data_read(1), sleep_time_ms);
        printf("Initial T=%d, latest T=%d\n", ulp_data_read(0), ulp_data_read(2));
    }
#endif // CONFIG_IDF_TARGET_ESP32
#endif // CONFIG_EXAMPLE_ULP_TEMPERATURE_WAKEUP

    vTaskDelay(1000 / portTICK_PERIOD_MS);

#ifdef CONFIG_EXAMPLE_ULP_TEMPERATURE_WAKEUP
#if CONFIG_IDF_TARGET_ESP32
    printf("Enabling ULP wakeup\n");
    esp_sleep_enable_ulp_wakeup();
#endif
#endif

#if CONFIG_IDF_TARGET_ESP32
    // Isolate GPIO12 pin from external circuits. This is needed for modules
    // which have an external pull-up resistor on GPIO12 (such as ESP32-WROVER)
    // to minimize current consumption.
    rtc_gpio_isolate(GPIO_NUM_12);
#endif

    printf("Entering deep sleep\n");
    gettimeofday(&sleep_enter_time, NULL);

    esp_deep_sleep_start();
}

#ifdef CONFIG_EXAMPLE_TOUCH_WAKEUP
#if CONFIG_IDF_TARGET_ESP32
static void calibrate_touch_pad(touch_pad_t pad)
{
    int avg = 0;
    const size_t calibration_count = 128;
    for (int i = 0; i < calibration_count; ++i)
    {
        uint16_t val;
        touch_pad_read(pad, &val);
        avg += val;
    }
    avg /= calibration_count;
    const int min_reading = 300;
    if (avg < min_reading)
    {
        printf("Touch pad #%d average reading is too low: %d (expecting at least %d). "
               "Not using for deep sleep wakeup.\n",
               pad, avg, min_reading);
        touch_pad_config(pad, 0);
    }
    else
    {
        int threshold = avg - 100;
        printf("Touch pad #%d average: %d, wakeup threshold set to %d.\n", pad, avg, threshold);
        touch_pad_config(pad, threshold);
    }
}
#endif
#endif // CONFIG_EXAMPLE_TOUCH_WAKEUP

#ifdef CONFIG_EXAMPLE_ULP_TEMPERATURE_WAKEUP
#if CONFIG_IDF_TARGET_ESP32
static void start_ulp_temperature_monitoring(void)
{
    /*
     * This ULP program monitors the on-chip temperature sensor and wakes the chip up when
     * the temperature goes outside of certain window.
     * When the program runs for the first time, it saves the temperature measurement,
     * it is considered initial temperature (T0).
     *
     * On each subsequent run, temperature measured and compared to T0.
     * If the measured value is higher than T0 + max_temp_diff or lower than T0 - max_temp_diff,
     * the chip is woken up from deep sleep.
     */

    /* Temperature difference threshold which causes wakeup
     * With settings here (TSENS_CLK_DIV=2, 8000 cycles),
     * TSENS measurement is done in units of 0.73 degrees Celsius.
     * Therefore, max_temp_diff below is equivalent to ~2.2 degrees Celsius.
     */
    const int16_t max_temp_diff = 3;

    // Number of measurements ULP should do per second
    const uint32_t measurements_per_sec = 5;

    // Allow TSENS to be controlled by the ULP
    SET_PERI_REG_BITS(SENS_SAR_TSENS_CTRL_REG, SENS_TSENS_CLK_DIV, 2, SENS_TSENS_CLK_DIV_S);
    SET_PERI_REG_BITS(SENS_SAR_MEAS_WAIT2_REG, SENS_FORCE_XPD_SAR, 3, SENS_FORCE_XPD_SAR_S);
    CLEAR_PERI_REG_MASK(SENS_SAR_TSENS_CTRL_REG, SENS_TSENS_POWER_UP);
    CLEAR_PERI_REG_MASK(SENS_SAR_TSENS_CTRL_REG, SENS_TSENS_DUMP_OUT);
    CLEAR_PERI_REG_MASK(SENS_SAR_TSENS_CTRL_REG, SENS_TSENS_POWER_UP_FORCE);

    // Clear the part of RTC_SLOW_MEM reserved for the ULP. Makes debugging easier.
    memset(RTC_SLOW_MEM, 0, CONFIG_ESP32_ULP_COPROC_RESERVE_MEM);

    // The first word of memory (at data offset) is used to store the initial temperature (T0)
    // Zero it out here, then ULP will update it on the first run.
    ulp_data_write(0, 0);
    // The second word is used to store measurement count, zero it out as well.
    ulp_data_write(1, 0);

    const ulp_insn_t program[] = {
        // load data offset into R2
        I_MOVI(R2, ULP_DATA_OFFSET),
        // load/increment/store measurement counter using R1
        I_LD(R1, R2, 1),
        I_ADDI(R1, R1, 1),
        I_ST(R1, R2, 1),
        // enable temperature sensor
        I_WR_REG(SENS_SAR_MEAS_WAIT2_REG, SENS_FORCE_XPD_SAR_S, SENS_FORCE_XPD_SAR_S + 1, 3),
        // do temperature measurement and store result in R3
        I_TSENS(R3, 8000),
        // disable temperature sensor
        I_WR_REG(SENS_SAR_MEAS_WAIT2_REG, SENS_FORCE_XPD_SAR_S, SENS_FORCE_XPD_SAR_S + 1, 0),
        // Save current measurement at offset+2
        I_ST(R3, R2, 2),
        // load initial value into R0
        I_LD(R0, R2, 0),
        // if threshold value >=1 (i.e. initialized), goto 1
        M_BGE(1, 1),
        // otherwise, save the current value as initial (T0)
        I_MOVR(R0, R3),
        I_ST(R0, R2, 0),
        M_LABEL(1),
        // check if the temperature is greater or equal (T0 + max_temp_diff)
        // uses R1 as scratch register, difference is saved at offset + 3
        I_ADDI(R1, R0, max_temp_diff - 1),
        I_SUBR(R1, R1, R3),
        I_ST(R1, R2, 3),
        M_BXF(2),
        // check if the temperature is less or equal (T0 - max_temp_diff)
        // uses R1 as scratch register, difference is saved at offset + 4
        I_SUBI(R1, R0, max_temp_diff - 1),
        I_SUBR(R1, R3, R1),
        I_ST(R1, R2, 4),
        M_BXF(2),
        // temperature is within (T0 - max_temp_diff; T0 + max_temp_diff)
        // stop ULP until the program timer starts it again
        I_HALT(),
        M_LABEL(2),
        // temperature is out of bounds
        // disable ULP program timer
        I_WR_REG_BIT(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN_S, 0),
        // initiate wakeup of the SoC
        I_WAKE(),
        // stop the ULP program
        I_HALT()};

    // Load ULP program into RTC_SLOW_MEM, at offset 0
    size_t size = sizeof(program) / sizeof(ulp_insn_t);
    ESP_ERROR_CHECK(ulp_process_macros_and_load(0, program, &size));
    assert(size < ULP_DATA_OFFSET && "ULP_DATA_OFFSET needs to be greater or equal to the program size");

    // Set ULP wakeup period
    const uint32_t sleep_cycles = rtc_clk_slow_freq_get_hz() / measurements_per_sec;
    REG_WRITE(SENS_ULP_CP_SLEEP_CYC0_REG, sleep_cycles);

    // Start ULP
    ESP_ERROR_CHECK(ulp_run(0));
}
#endif // CONFIG_IDF_TARGET_ESP32
#endif // CONFIG_EXAMPLE_ULP_TEMPERATURE_WAKEUP
