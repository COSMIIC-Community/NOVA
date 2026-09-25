/**
 *  @file eedata.h
 *
 *  @date Created on: Oct 14, 2024
 *  @author: jenej
 */

#ifndef APP_MODULE_EEDATA_H_
#define APP_MODULE_EEDATA_H_

// -------- DEFINITIONS ----------
#define MAX_BYTES_PER_SUBINDEX  32    //TODO: Not sure what happens if subindex exceeds this
#define EEPROM_RECORD_SIZE      32
#define EEPROM_ERASE_SIZE       0x80 ///< must be divisible into 8192 (8KB) by factor of 2, defines size of chunks erased from EEPROM

//#define MAX_EEPROM_MEMORY       0x1800 ///< (6KB)
//#define EEPROM_PAGE_SIZE		3	  ///< XXX: must match the page size in bootloader config.h
//#define EEPROM_START_ADDRESS	((127 - EEPROM_PAGE_SIZE) * FLASH_PAGE_SIZE)   ///< Loation of the start of the EEPROM in flash

#define MAX_FLASH_MEMORY        FLASH_SIZE ///< (2MB)
#define FLASH_RECORD_SIZE       32

// --------   DATA   ------------


// -------- PROTOTYPES ----------

UNS8 CheckRestoreFlag(void);
void SaveValues( void );
void RestoreValues( void );
void ResetToODDefault(void);
void ResetModule(void);
void EEPROM_erase(UNS8 space);
UNS8 ReadLocalFlashData( UNS32 nvAddress, UNS8 * data, UNS8 numData );
UNS8 EEPROM_open();
UNS8 EEPROM_commit();
void EEPROM_read(UNS16 address, UNS8 * data, UNS16 length);
void EEPROM_write(UNS16 address, UNS8 * data, UNS16 length);

#endif /* APP_MODULE_EEDATA_H_ */
