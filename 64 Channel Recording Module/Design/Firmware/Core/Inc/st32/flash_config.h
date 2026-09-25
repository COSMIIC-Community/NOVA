/**
 * @file flash_config.h
 *
 *  Created on: @date Sept 23, 2025
 *      @author Dale Walter (djwalter.software@gmail.com)
 *      @brief 	Defines memory spaces in Flash
 *  @ingroup bootloader
 *  @note Order of parameters in flash is NNB, Checksum, 3x App Size, Checksum Clk
 *  @note SN and modType are stored in last 3 bytes of Boot Flash address space ();
 */

#pragma once

#define BOOT_SIZE 0x00008000           ///< 32kB
#define BOOTLOADER_PAGE_COUNT                                                                      \
    ( BOOT_SIZE / FLASH_PAGE_SIZE )    ///< The number of flash pages allocated for the bootloader

#define EEPROM_PAGE_COUNT 1            ///< Number of pages of the emulated EEPROM
#define EEPROM_MEMORY_SIZE                                                                         \
    ( FLASH_PAGE_SIZE * EEPROM_PAGE_COUNT )       ///< Size of the EEPROM in bytes

#define PARAMETER_PAGE_COUNT 1                    ///< Number of pages for Parameter Memory
#define PARAMETER_MEMORY_SIZE                                                                      \
    ( FLASH_PAGE_SIZE * PARAMETER_PAGE_COUNT )    ///< Size of the Parameter Memory in bytes


#define MAX_APP_SIZE                                                                               \
    ( FLASH_BANK_SIZE - BOOT_SIZE - EEPROM_MEMORY_SIZE - PARAMETER_MEMORY_SIZE )

#define BOOT_START_ADDR                                                                            \
    FLASH_BASE    ///< Start address of bootloader in flash
                  ///< @note Flash addressing starts at 0x08000000
#define BOOT_END_ADDR     ( BOOT_START_ADDR + BOOT_SIZE - 1 )    ///< End address of bootloader in flash
#define APP_START_ADDR    ( BOOT_END_ADDR + 1 )
#define APP_END_ADDR      ( APP_START_ADDR + MAX_APP_SIZE - 1 )
#define EEPROM_START_ADDR ( APP_END_ADDR + 1 )        ///< Start address of EEPROM in flash
#define EEPROM_END_ADDR                                                                            \
    ( EEPROM_START_ADDR + EEPROM_MEMORY_SIZE - 1 )    ///< End address of EEPROM in flash
#define PARAM_START_ADDR ( EEPROM_END_ADDR + 1 )    ///< Start address of module parameters in flash
#define PARAM_END_ADDR                                                                             \
    ( PARAM_START_ADDR + PARAMETER_MEMORY_SIZE                                                     \
      - 1 )    ///< End address of module parameters in flash

#define BOOTLOADER_PAGE_NB 0X00
#define APP_PAGE_NB        ( BOOTLOADER_PAGE_NUM + BOOTLOADER_PAGE_COUNT )
#define EEPROM_PAGE_NB     ( FLASH_PAGE_NB - EEPROM_PAGE_COUNT - PARAMETER_PAGE_COUNT )
#define PARAMETER_PAGE_NB  ( FLASH_PAGE_NB - PARAMETER_PAGE_COUNT )

#define NODE_NB_ADDR      PARAM_START_ADDR         ///< Address in flash where the node number lives
#define CHECKSUM_ADDR     ( NODE_NB_ADDR + 1 )     ///< Address in flash where the checksum lives
#define TEST_LOCK_ADDR    ( BOOT_END_ADDR - 3 )    ///< Address to check for write protection
#define SERIAL_NB_ADDR    ( BOOT_END_ADDR - 2 )    ///< Address oF the serial number (2 bytes)
#define PRODUCT_CODE_ADDR ( BOOT_END_ADDR )        ///< Address of the product code
///< (4 = PulseGenerator, 5=BioPotential, 6 = SensorHost).
///< The NOVA can use product code 7 for now,
