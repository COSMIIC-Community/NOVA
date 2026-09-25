/**
 *  @file eedata.c
 *
 *  @date Created on: Oct 14, 2024
 *  @author: jenej
 *  @defgroup EEPROM Emulated EEPROM
 */

//#include "applicfg.h"
#include "ObjDict.h"
#include "objdictdef.h"
#include "flash_config.h"
#include "eedata.h"
#include <stdlib.h>

//============================
//    GLOBAL CODE
//============================

uint8_t page[FLASH_PAGE_SIZE * EEPROM_PAGE_COUNT];
uint8_t memOpen = 0;

/**
 * @ingroup EEPROM
 * @brief checks whether or not RestoreValues() should be run.
 * @return 1=RestoreValues() should be run, 0=ResetToODDefault() was run, so don't restore from EEPROM
 */
UNS8 CheckRestoreFlag(void)
{
  UNS8 data[2];

  EEPROM_read(0x00, data, 2);
  if ((data[0] == 0xFF && data[1] == 0xFF) || (data[0] == 0x00 && data[1] == 0x00))
    return 0;
  else
    return 1;
}

/**
 * @ingroup EEPROM
 * @brief Saves the values of custom OD entries (specified in RestoreList OD index 0x2900)
 *        to EEPROM from the OD.  Companion function to RestoreValues().  Note that if the RestoreList is changed,
 *        RestoreValues() will not restore values to the correct subindices in the OD unless the EEPROM data
 *        has been updated via SaveValues(). I.e., SaveValues() only stores the data bytes contained within the
 *        OD subindices in the order specified by RestoreList; the indices and subindices themselves are not
 *        stored in EEPROM.
 * @details Called from NMT_Do_Restore_Cmd (only when in the Waiting Mode) and from the startup sequence.
 *          The first two bytes in EEPROM specify the amount EEPROM used by this function.  All OD indices used by the
 *          RestoreList MUST have more than one subindex, where the first subindex specifies the number of subindices.
 *          Takes ~9ms per byte that needs to be saved.
 */
void SaveValues( void )
{
  UNS8 nSubIndices;
  UNS32 size = 0;
  UNS8  type = 0;
  UNS8 data[MAX_BYTES_PER_SUBINDEX];
  UNS32 abortCode = 0;
  UNS16 counter = 2; //NOTE: counter starts at 2 (0 and 1 used to store size later)
  UNS8 i = 0;

  EEPROM_open();
  //loop through each index in RestoreList
  for (i = 0; i < sizeof(RestoreList); i++)
  {
    //don't let user save/restore below 0x1018 in OD, currently to make sure 1st subindex specifies nSubIndices
    if (RestoreList[i] < 0x1018)
      continue;

    //need to reset the size before the next "read"
    size = 0;
    type = 0;
    //read the 0th subindex (nSubIndices) at the current index
    abortCode = readLocalDict( &ObjDict_Data, RestoreList[i], 0, &nSubIndices, &size, &type, 0);
    if ( abortCode != OD_SUCCESSFUL )
	   break;

    if (type == uint8) // First subindex must be a UNS8 see objdictdef.h for datatypes
    {
        //loop through each subindex and write bytes to EEPROM
        for (int k = 1; k <= nSubIndices; k++) //the first element of subIndexSize must contain the number of subindices
        {
          size = 0;
          //?what happens here if subindex has more bytes than MAX_BYTES_PER_SUBINDEX?
          abortCode = readLocalDict( &ObjDict_Data, RestoreList[i], k, data, &size, &type, 0);
          if ( abortCode != OD_SUCCESSFUL )
			 break;

          //write all the subindex bytes to EEPROM
          if (counter+size < EEPROM_MEMORY_SIZE )
          {
            EEPROM_write(counter, data, size );
            counter += size;
          }
        }
      }
  }
  //write the number of bytes used (current counter value) in the first 2 bytes of EEPROM
  data[0] = (UNS8) counter;
  data[1] = (UNS8)(counter >> 8);
  EEPROM_write(0, data, 2);
  EEPROM_commit();
  //NOTE: disabling/enabling interrupts is handled in EEPROM_commit routine!
}

/**
 * @ingroup EEPROM
 * @brief Restores the values of custom OD entries (specified in RestoreList OD index 0x2900)
 *        to the OD from EEPROM.  Companion function to SaveValues().  Note that if the RestoreList is changed,
 *        RestoreValues() will not restore values to the correct subindices in the OD unless the EEPROM data
 *        has been updated via SaveValues(). I.e., SaveValues() only stores the data bytes contained within the
 *        OD subindices in the order specified by RestoreList; the indices and subindices themselves are not
 *        stored in EEPROM.
 * @details Called from NMT_Do_Restore_Cmd (only when in the Waiting Mode) and from the startup sequence.
 *          All OD indices used by the RestoreList MUST have more than one subindex, where the first subindex
 *          specifies the number of subindices.
*/
void RestoreValues ( void )
{

  UNS8 nSubIndices;
  UNS32 size = 0;
  UNS8  type = 0;
  UNS8 data[MAX_BYTES_PER_SUBINDEX];
  UNS32 abortCode = 0;;
  UNS16 counter = 2;

    for (int i = 0; i < sizeof(RestoreList); i++)
    {
      //don't let user save/restore below 0x1018 in OD, currently to make sure 1st subindex specifies nSubIndices
      if (RestoreList[i] < 0x1018)
        continue;

      size = 0;
      type = 0;

      //read the 0th subindex (nSubIndices) at the current index
      abortCode = readLocalDict( &ObjDict_Data, RestoreList[i], 0, &nSubIndices, &size, &type, 0);
      if ( abortCode != OD_SUCCESSFUL )
		   break;

      if ( type == uint8 )
      {
        //loop through each subindex and read bytes from EEPROM
        for (int k = 1; k <=  nSubIndices; k++)
        {
          size = 0;
          //read current values from OD to get length of subindex
          abortCode = readLocalDict( &ObjDict_Data, RestoreList[i], k, data, &size, &type, 0);
          if ( abortCode != OD_SUCCESSFUL )
			 break;

          //read all subindex bytes from EEPROM
          if( counter+size < EEPROM_MEMORY_SIZE )
          {
              EEPROM_read( counter, data, size );
              counter += size;
          }
          //write data to OD
          abortCode = writeLocalDict( &ObjDict_Data, RestoreList[i], k, data, &size, 0);
          if ( abortCode != OD_SUCCESSFUL )
			 break;
        }
      }
    }


}

/**
 * @ingroup EEPROM
 * @brief  invoked by nmt_master -- writes 0's to size causing bypass of OD restore, then causes reset
 */
void ResetToODDefault(void)
{

  UNS8 data[2];
  data[0] = 0x00;
  data[1] = 0x00;

  EEPROM_open();
  EEPROM_write(0x00, data, 0x02);
  EEPROM_commit();

  ResetModule();
}

/**
 * @ingroup EEPROM
 * @brief resets module, called by ResetToODDefault()
 */
void ResetModule(void)
{
  __disable_irq();

  NVIC_SystemReset();
}

/**
 * @ingroup EEPROM
 * @brief reads CPU based flash data
 * @param nvAddress
 * @param pointer to data
 * @param size of data requested
 */
UNS8 ReadLocalFlashData( UNS32 nvAddress, UNS8 * data, UNS8 numData )
{
  if (nvAddress > 0x5000 && nvAddress < (0x3FFFF))
  {
	memcpy( data, (UNS8 *)nvAddress , numData );
  }
  else if ( nvAddress <= 0x5000 ) //Bootloader region
  {
    memcpy( data, (UNS8 *)nvAddress , numData );
  }
  else
    return 2;


  return 0;
}


/**
 * @ingroup EEPROM
 * @brief writes 0xFF to entire EEPROM (6KB).
 * @param space: can be used to specify region of EEPROM (as long as aligned with EEPROM_ERASE_SIZE)
 */
void EEPROM_erase(UNS8 space)
{
	// Space variable can be unused, or used to define a region of the EEPROM if set up that way

	UNS8 byteErase[EEPROM_ERASE_SIZE];
	memset(byteErase, 0xFF, EEPROM_ERASE_SIZE);

	EEPROM_open();

	for (int i = 0; i < EEPROM_MEMORY_SIZE; i += EEPROM_ERASE_SIZE){
	  EEPROM_write(i, byteErase, EEPROM_ERASE_SIZE);
	}

	EEPROM_commit();
}

//============================
//    LOCAL CODE
//============================

/**
 * @ingroup EEPROM
 * @brief Copies the EEPROM into ram for modification by EEPROM_write()
 */
UNS8 EEPROM_open(){
	if(memOpen == 0){
		memcpy(page, (void*) EEPROM_START_ADDR, (FLASH_PAGE_SIZE * EEPROM_PAGE_COUNT));	// Copy the EEPROM page into memory
		memOpen = 1;
		return 1;
	}
	return 0;
}


/**
 * @ingroup EEPROM
 * @brief Commits edits to EEPROM from ram to flash
 */
UNS8 EEPROM_commit(){
	FLASH_EraseInitTypeDef eraseInitStruct;
	uint32_t PageError = 0;

	if (memOpen == 0)
		return 0;

	__disable_irq();
	HAL_FLASH_Unlock();

	//Erase the page
	eraseInitStruct.TypeErase 	= TYPEERASE_PAGES;		// Just erase pages
	eraseInitStruct.Banks 		= FLASH_BANK_1;
	eraseInitStruct.Page 		= EEPROM_PAGE_NB; 		// Erase the EEPROM pages, leaving parameters untouched
	eraseInitStruct.NbPages 	= EEPROM_PAGE_COUNT;

	if (HAL_FLASHEx_Erase(&eraseInitStruct, &PageError) != HAL_OK) {
		HAL_FLASH_Lock(); // Re-lock the flash afterwards'
		memset(page, 0, sizeof(page));
		memOpen = 0;
		__enable_irq();
		return 0;
	}

	//Rewrite the page in flash
	for (int i = 0; i < EEPROM_PAGE_COUNT * FLASH_PAGE_SIZE; i += 16) // Program into flash in quad word increments (16 bytes)
	{
	  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, EEPROM_START_ADDR + i, (uint32_t)&page[i]) != HAL_OK)
	  {
		  return 0;
	  }
	}

	HAL_FLASH_Lock(); // Re-lock the flash afterwards
	__enable_irq();

	memOpen = 0;
	return 1;
}

/**
 * @ingroup EEPROM
 * @brief Discards current edits to EEPROM without committing to flash
 */
UNS8 EEPROM_discard(){
	if (memOpen == 0)
		return 0;

	memOpen = 0;

	return 1;
}

/**
 * @ingroup EEPROM
 * @brief Writes bytes to a specified location in EEPROM
 * @details EEPROM_open() must be called before this to copy EEPROM into ram, which this function edits
 * @details EEPROM_commit() is called afterwards to commit EEPROM edits to flash
 * @param address The address, relative to the start of the EEPROM where the data will be written.
 * @param *data pointer to the data (or array) to be written
 * @param length length of data to be written
 */
void EEPROM_write(UNS16 address, UNS8 * data, UNS16 length)
{
	if (memOpen == 0)
		return;

	memcpy((page + address), data, length);		// Copy updated data into location in the memory copy
}


/**
 * @ingroup EEPROM
 * @brief Reads bytes from a specified location in EEPROM
 * @param address The address, relative to the start of the EEPROM where the data will be written.
 * @param *data pointer to the data (or array) to read to
 * @param length length of data to be read
 */
void EEPROM_read(UNS16 address, UNS8 * data, UNS16 length)
{
	memcpy(data, (void *)(address + EEPROM_START_ADDR), length);
}


//============================
//    INTERRUPT SERVICE ROUTINES
//============================


//============================
//    HARDWARE SPECIFIC CODE
//============================



