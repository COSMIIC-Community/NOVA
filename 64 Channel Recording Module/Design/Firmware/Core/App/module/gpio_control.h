/// @file       gpio_control.h
/// @brief      Macros to control gpio pins.
/// @date       January 16, 2025
#pragma once

#include "main.h"
#include "stm32u5xx_hal.h"

#ifndef TEST
#    define blue_led_on()                                                                          \
        HAL_GPIO_WritePin( PF13_BLUE_LED_GPIO_Port, PF13_BLUE_LED_Pin, GPIO_PIN_RESET );
#    define blue_led_off()                                                                         \
        HAL_GPIO_WritePin( PF13_BLUE_LED_GPIO_Port, PF13_BLUE_LED_Pin, GPIO_PIN_SET );
#    define green_led_on()  HAL_GPIO_WritePin( PC4_LED_GPIO_Port, PC4_LED_Pin, GPIO_PIN_RESET );
#    define green_led_off() HAL_GPIO_WritePin( PC4_LED_GPIO_Port, PC4_LED_Pin, GPIO_PIN_SET );
#    define tp21_on()       HAL_GPIO_WritePin( PA2_TP21_GPIO_Port, PA2_TP21_Pin, GPIO_PIN_SET );
#    define tp21_off()      HAL_GPIO_WritePin( PA2_TP21_GPIO_Port, PA2_TP21_Pin, GPIO_PIN_RESET );
#    define tp24_on()       HAL_GPIO_WritePin( PA5_TP24_GPIO_Port, PA5_TP24_Pin, GPIO_PIN_SET );
#    define tp24_off()      HAL_GPIO_WritePin( PA5_TP24_GPIO_Port, PA5_TP24_Pin, GPIO_PIN_RESET );
#else
void blue_led_on( void );
void blue_led_off( void );
void green_led_on( void );
void green_led_off( void );
void tp21_on( void );
void tp21_off( void );
void tp24_on( void );
void tp24_off( void );
#endif
