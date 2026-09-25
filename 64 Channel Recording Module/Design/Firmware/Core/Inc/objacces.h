/**
 * @file objacces.h
 *
 *  Created on: Oct 9, 2024
 *      Author: jenej
 */

#ifndef INC_OBJACCES_H_
#define INC_OBJACCES_H_

#include <applicfg.h>

typedef UNS32 (*valueRangeTest_t)(UNS8 typeValue, void *Value);
typedef void (* storeODSubIndex_t)(CO_Data* d, UNS16 wIndex, UNS8 bSubindex);
void _storeODSubIndex (CO_Data* d, UNS16 wIndex, UNS8 bSubindex);

UNS8 accessDictionaryError(UNS16 index, UNS8 subIndex,
			   UNS32 sizeDataDict, UNS32 sizeDataGiven, UNS32 code);



UNS32 _getODentry( CO_Data* d,
		  UNS16 wIndex,
		  UNS8 bSubindex,
		  void * pDestData,
		  UNS32 * pExpectedSize,
		  UNS8 * pDataType,
		  UNS8 checkAccess,
		  UNS8 endianize);


#define getODentry( OD, wIndex, bSubindex, pDestData, pExpectedSize, \
		          pDataType,  checkAccess)                         \
       _getODentry( OD, wIndex, bSubindex, pDestData, pExpectedSize, \
		          pDataType,  checkAccess, 1)

#define readLocalDict( OD, wIndex, bSubindex, pDestData, pExpectedSize, \
		          pDataType,  checkAccess)                         \
       _getODentry( OD, wIndex, bSubindex, pDestData, pExpectedSize, \
		          pDataType,  checkAccess, 0)


UNS32 _setODentry( CO_Data* d,
                   UNS16 wIndex,
                   UNS8 bSubindex,
                   void * pSourceData,
                   UNS32 * pExpectedSize,
                   UNS8 checkAccess,
                   UNS8 endianize);


#define setODentry( d, wIndex, bSubindex, pSourceData, pExpectedSize, \
                  checkAccess) \
       _setODentry( d, wIndex, bSubindex, pSourceData, pExpectedSize, \
                  checkAccess, 1)

#define writeLocalDict( d, wIndex, bSubindex, pSourceData, pExpectedSize, checkAccess) \
       _setODentry( d, wIndex, bSubindex, pSourceData, pExpectedSize, checkAccess, 0)




 const indextable * scanIndexOD (CO_Data* d, UNS16 wIndex, UNS32 *errorCode, ODCallback_t **Callback);

UNS32 RegisterSetODentryCallBack(CO_Data* d, UNS16 wIndex, UNS8 bSubindex, ODCallback_t Callback);

#endif /* INC_OBJACCES_H_ */
