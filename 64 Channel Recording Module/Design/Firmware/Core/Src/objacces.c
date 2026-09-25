/**
 * @file objacces.c
 *
 * @date Created on: Oct 10, 2024
 * @author Jerry Ukwela (jeu6@case.edu)
 * @addtogroup CANFestival
 * @{
 * @defgroup od Object Dictionary
 * @}
 */


#include "data.h"

/**
 * @brief Print MSG_WAR (s) if error to the access to the object dictionary occurs.
 * @details
 * You must uncomment the lines in the file objaccess.c :\n
 * //#define DEBUG_CAN\n
 * //#define DEBUG_WAR_CONSOLE_ON\n
 * //#define DEBUG_ERR_CONSOLE_ON\n\n
 * Beware that sometimes, we force the sizeDataDict or sizeDataGiven to 0, when we wants to use
 * this function but we do not have the access to the right value. One example is
 * getSDOerror(). So do not take attention to these variables if they are null.
 * @param index
 * @param subIndex
 * @param sizeDataDict Size of the data defined in the dictionary
 * @param sizeDataGiven Size data given by the user.
 * @param code error code to print. (SDO abort code. See file def.h)
 * @return
 */
#ifdef DEBUG_WAR_CONSOLE_ON
UNS8 accessDictionaryError(UNS16 index, UNS8 subIndex,
                           UNS32 sizeDataDict, UNS32 sizeDataGiven, UNS32 code)
{
  MSG_WAR(0x2B09,"Dictionary index : ", index);
  MSG_WAR(0X2B10,"           subindex : ", subIndex);
  switch (code)
  {
  case  OD_NO_SUCH_OBJECT:
    MSG_WAR(0x2B11,"Index not found ", index);
    break;
  case OD_NO_SUCH_SUBINDEX :
    MSG_WAR(0x2B12,"SubIndex not found ", subIndex);
    break;
  case OD_WRITE_NOT_ALLOWED :
    MSG_WAR(0x2B13,"Write not allowed, data is read only ", index);
    break;
  case OD_LENGTH_DATA_INVALID :
    MSG_WAR(0x2B14,"Conflict size data. Should be (bytes)  : ", sizeDataDict);
    MSG_WAR(0x2B15,"But you have given the size  : ", sizeDataGiven);
    break;
  case OD_NOT_MAPPABLE :
    MSG_WAR(0x2B16,"Not mappable data in a PDO at index    : ", index);
    break;
  case OD_VALUE_TOO_LOW :
    MSG_WAR(0x2B17,"Value range error : value too low. SDOabort : ", code);
    break;
  case OD_VALUE_TOO_HIGH :
    MSG_WAR(0x2B18,"Value range error : value too high. SDOabort : ", code);
    break;
  default :
    MSG_WAR(0x2B20, "Unknown error code : ", code);
  }
  return 0;
}
#else
#define accessDictionaryError(index, subIndex, sizeDataDict, sizeDataGiven, code)
#endif

/**
 * @brief _getODentry() Reads an entry from the object dictionary.\n
 * @details
 *    use getODentry() macro to read from object and endianize
 *    use readLocalDict() macro to read from object and not endianize
 *
 * @code
 * // Example usage:
 * UNS8  *pbData;
 * UNS8 length;
 * UNS32 returnValue;
 *
 * returnValue = getODentry( (UNS16)0x100B, (UNS8)1,
 * (void * *)&pbData, (UNS8 *)&length );
 * if( returnValue != SUCCESSFUL )
 * {
 *     // error handling
 * }
 * @endcode
 * @param *d Pointer to a CAN object data structure
 * @param wIndex The index in the object dictionary where you want to read
 *               an entry
 * @param bSubindex The subindex of the Index. e.g. mostly subindex 0 is
 *                  used to tell you how many valid entries you can find
 *                  in this index. Look at the canopen standard for further
 *                  information
 * @param *pDestData Pointer to the pointer which points to the variable where
 *                   the value of this object dictionary entry should be copied
 * @param *pExpectedSize This function writes the size of the copied value (in Byte)
 *                      into this variable.
 * @param *pDataType Pointer to the type of the data. See objdictdef.h
 * @param CheckAccess if other than 0, do not read if the data is Write Only
 *                    [Not used today. Put always 0].
 * @param Endianize  When not 0, data is endianized into network byte order
 *                   when 0, data is not endianized and copied in machine native
 *                   endianness
 * @return
 * - OD_SUCCESSFUL is returned upon success.
 * - SDO abort code is returned if error occurs . (See file def.h)
 */
UNS32 _getODentry( CO_Data* d,
                   UNS16 wIndex,
                   UNS8 bSubindex,
                   void * pDestData,
                   UNS32 * pExpectedSize,
                   UNS8 * pDataType,
                   UNS8 checkAccess,
                   UNS8 endianize)
{ /* DO NOT USE MSG_ERR because the macro may send a PDO -> infinite
    loop if it fails. */
  UNS32 errorCode;
  UNS32 szData;
  const indextable * ptrTable;
  ODCallback_t *Callback;

  ptrTable = (*d->scanIndexOD)(wIndex, &errorCode, &Callback);

  if (errorCode != OD_SUCCESSFUL)
    return errorCode;
  if( ptrTable->bSubCount <= bSubindex )
  {
    /* Subindex not found */
    accessDictionaryError(wIndex, bSubindex, 0, 0, OD_NO_SUCH_SUBINDEX);
    return OD_NO_SUCH_SUBINDEX;
  }

  if (checkAccess && (ptrTable->pSubindex[bSubindex].bAccessType & WO))
  {
    MSG_WAR(0x2B30, "Access Type : ", ptrTable->pSubindex[bSubindex].bAccessType);
    accessDictionaryError(wIndex, bSubindex, 0, 0, OD_READ_NOT_ALLOWED);
    return OD_READ_NOT_ALLOWED;
  }


  *pDataType = ptrTable->pSubindex[bSubindex].bDataType;
  szData = ptrTable->pSubindex[bSubindex].size;

  if(*pExpectedSize == 0 ||
     *pExpectedSize == szData ||
     /* allow to fetch a shorter string than expected */
     (*pDataType >= visible_string && *pExpectedSize < szData))
  {

#  ifdef CANOPEN_BIG_ENDIAN
     if(endianize && *pDataType > boolean && !(
            *pDataType >= visible_string &&
            *pDataType <= domain))
     {
      /* data must be transmited with low byte first */
      UNS8 i, j = 0;
      MSG_WAR(boolean, "data type ", *pDataType);
      MSG_WAR(visible_string, "data type ", *pDataType);
      for ( i = szData ; i > 0 ; i--)
      {
        MSG_WAR(i," ", j);
        ((UNS8*)pDestData)[j++] =
          ((UNS8*)ptrTable->pSubindex[bSubindex].pObject)[i-1];
      }

      *pExpectedSize = szData;
  }
    else /* no endianisation change */
#  endif
    if(*pDataType != visible_string)
    {
        memcpy(pDestData, ptrTable->pSubindex[bSubindex].pObject, szData);
        *pExpectedSize = szData;
    }

    else
    {

        /* Copy null terminated string to user, and return discovered size */
        UNS8 *ptr = (UNS8*)ptrTable->pSubindex[bSubindex].pObject;
        UNS8 *ptr_start = ptr;

        /* *pExpectedSize IS < szData . if null, use szData */
        UNS8 *ptr_end = ptr + (*pExpectedSize ? *pExpectedSize : szData) ;
        UNS8 *ptr_dest = (UNS8*)pDestData;

        while( *ptr && ptr < ptr_end)
        {
            *(ptr_dest++) = *(ptr++);
        }

        *pExpectedSize = (UNS32)(ptr - ptr_start);
        /* terminate string if not maximum length */
        if (*pExpectedSize < szData)
            *(ptr) = 0;

    }

    return OD_SUCCESSFUL;
  }
  else
  { /* Error ! */
    *pExpectedSize = szData;
    accessDictionaryError(wIndex, bSubindex, szData,
                          *pExpectedSize, OD_LENGTH_DATA_INVALID);

    return OD_LENGTH_DATA_INVALID;
  }
}

/**
 * @brief By this function you can write an entry into the object dictionary
 * @param *d Pointer to a CAN object data structure
 * @param wIndex The index in the object dictionary where you want to write
 *               an entry
 * @param bSubindex The subindex of the Index. e.g. mostly subindex 0 is
 *                  used to tell you how many valid entries you can find
 *                  in this index. Look at the canopen standard for further
 *                  information
 * @param *pSourceData Pointer to the variable that holds the value that should
 *                     be copied into the object dictionary
 * @param *pExpectedSize The size of the value (in Byte).
 * @param checkAccess Flag that indicate if a check rights must be perfomed (0 : no , other than 0 : yes)
 * @param endianize When not 0, data is endianized into network byte order
 *                  when 0, data is not endianized and copied in machine native
 *                  endianness
 * @return
 * - OD_SUCCESSFUL is returned upon success.
 * - SDO abort code is returned if error occurs . (See file def.h)
 */
UNS32 _setODentry( CO_Data* d,
                   UNS16 wIndex,
                   UNS8 bSubindex,
                   void * pSourceData,
                   UNS32 * pExpectedSize,
                   UNS8 checkAccess,
                   UNS8 endianize)
{
  UNS32 szData;
  UNS8 dataType;
  UNS32 errorCode;
  const indextable *ptrTable;
  ODCallback_t *Callback;

  ptrTable =(*d->scanIndexOD)(wIndex, &errorCode, &Callback);
  if (errorCode != OD_SUCCESSFUL)
    return errorCode;

  if( ptrTable->bSubCount <= bSubindex )
  {
    /* Subindex not found */
    accessDictionaryError(wIndex, bSubindex, 0, *pExpectedSize, OD_NO_SUCH_SUBINDEX);
    return OD_NO_SUCH_SUBINDEX;
  }
  if (checkAccess && (ptrTable->pSubindex[bSubindex].bAccessType == RO))
  {
    MSG_WAR(0x2B25, "Access Type : ", ptrTable->pSubindex[bSubindex].bAccessType);
    accessDictionaryError(wIndex, bSubindex, 0, *pExpectedSize, OD_WRITE_NOT_ALLOWED);
    return OD_WRITE_NOT_ALLOWED;
  }


  dataType = ptrTable->pSubindex[bSubindex].bDataType;
  szData = ptrTable->pSubindex[bSubindex].size;

  if( *pExpectedSize == 0 ||
      *pExpectedSize == szData ||
      /* allow to store a shorter string than entry size */
      (dataType == visible_string && *pExpectedSize < szData)) //look for else at bottom
    {
#ifdef CANOPEN_BIG_ENDIAN
      /* re-endianize do not occur for bool, strings time and domains */
      if(endianize && dataType > boolean && !(
            dataType >= visible_string &&
            dataType <= domain))
        {
          /* we invert the data source directly. This let us do range
            testing without */
          /* additional temp variable */
          UNS8 i;
          for ( i = 0 ; i < ( ptrTable->pSubindex[bSubindex].size >> 1)  ; i++)
            {
              UNS8 tmp =((UNS8 *)pSourceData) [(ptrTable->pSubindex[bSubindex].size - 1) - i];
              ((UNS8 *)pSourceData) [(ptrTable->pSubindex[bSubindex].size - 1) - i] = ((UNS8 *)pSourceData)[i];
              ((UNS8 *)pSourceData)[i] = tmp;
            }
        }
#endif
      errorCode = (*d->valueRangeTest)(dataType, pSourceData);
      if (errorCode)
      {
        accessDictionaryError(wIndex, bSubindex, szData, *pExpectedSize, errorCode);
        return errorCode;
      }
      memcpy(ptrTable->pSubindex[bSubindex].pObject,pSourceData, szData);
     /* TODO : CONFORM TO DS-301 :
      *  - stop using NULL terminated strings
      *  - store string size in td_subindex
      * */
      /* terminate visible_string with '\0' */
      if(dataType == visible_string && *pExpectedSize < szData)
        ((UNS8*)ptrTable->pSubindex[bSubindex].pObject)[*pExpectedSize] = 0;

      *pExpectedSize = szData;

      /* Callbacks */
      if(Callback && Callback[bSubindex])
      {
        errorCode = (Callback[bSubindex])(d, ptrTable, bSubindex);
        if(errorCode != OD_SUCCESSFUL)
        {
            return errorCode;
        }
       }

      /* TODO : Store dans NVRAM */
      if (ptrTable->pSubindex[bSubindex].bAccessType & TO_BE_SAVE)
      {
        (*d->storeODSubIndex)(d, wIndex, bSubindex);
      }
      return OD_SUCCESSFUL;
    }  // end if (outer IF -- 3rd from top)
  else
  {
      *pExpectedSize = szData;
      accessDictionaryError(wIndex, bSubindex, szData, *pExpectedSize, OD_LENGTH_DATA_INVALID);
      return OD_LENGTH_DATA_INVALID;
    }
}

/**
 * @brief Scan the index of object dictionary. Used only by setODentry and getODentry.
 * @param *d Pointer to a CAN object data structure
 * @param wIndex
 * @param *errorCode :  OD_SUCCESSFUL if index foundor SDO abort code. (See file def.h)
 * @param **Callback
 * @return NULL if index not found. Else : return the table part of the object dictionary.
 */
const indextable * scanIndexOD (CO_Data* d, UNS16 wIndex, UNS32 *errorCode, ODCallback_t **Callback)
{
  return (*d->scanIndexOD)(wIndex, errorCode, Callback);
}

/**
 * @ingroup od
 * @brief  <BRIEF>
 * @param *d Pointer to a CAN object data structure
 * @param wIndex
 * @param bSubindex
 * @param Callback
 * @return errorCode
 */
UNS32 RegisterSetODentryCallBack(CO_Data* d, UNS16 wIndex, UNS8 bSubindex, ODCallback_t Callback)
{
UNS32 errorCode;
ODCallback_t *CallbackList;
const indextable *odentry;

  odentry = scanIndexOD (d, wIndex, &errorCode, &CallbackList);
  if(errorCode == OD_SUCCESSFUL  &&  CallbackList  &&  bSubindex < odentry->bSubCount)
    CallbackList[bSubindex] = Callback;
  return errorCode;
}

/**
 * @ingroup od
 * @brief <BRIEF>
 * @param *d Pointer to a CAN object data structure
 * @param wIndex
 * @param bSubindex
 */
void _storeODSubIndex (CO_Data* d, UNS16 wIndex, UNS8 bSubindex){}
