#ifndef __FTE_JSON_H__
#define __FTE_JSON_H__

typedef enum
{
    FTE_JSON_TYPE_STRING = 0,
    FTE_JSON_TYPE_NUMBER,
    FTE_JSON_TYPE_FLOAT,
    FTE_JSON_TYPE_PAIR,   
    FTE_JSON_TYPE_OBJECT,   
    FTE_JSON_TYPE_ARRAY,
    FTE_JSON_TYPE_TRUE,
    FTE_JSON_TYPE_FALSE,
    FTE_JSON_TYPE_HEX,
    FTE_JSON_TYPE_NULL
}   FTE_JSON_TYPE, _PTR_ FTE_JSON_TYPE_PTR;

typedef struct  
{
    FTE_JSON_TYPE   xType;
}   FTE_JSON_VALUE, _PTR_ FTE_JSON_VALUE_PTR;

typedef struct  
{
    FTE_JSON_TYPE   xType;
    FTE_CHAR_PTR    pValue;
}   FTE_JSON_STRING, _PTR_ FTE_JSON_STRING_PTR;

typedef struct  
{
    FTE_JSON_TYPE   xType;
    long            nValue;
}   FTE_JSON_NUMBER, _PTR_ FTE_JSON_NUMBER_PTR;

typedef struct  
{
    FTE_JSON_TYPE   xType;
    long            nValue;
}   FTE_JSON_HEX, _PTR_ FTE_JSON_HEX_PTR;

typedef struct  
{
    FTE_JSON_TYPE   xType;
    long            nValue;
}   FTE_JSON_FLOAT, _PTR_ FTE_JSON_FLOAT_PTR;

typedef struct _FTE_JSON_PAIR
{
    FTE_JSON_TYPE       xType;
    FTE_CHAR_PTR        pString;
    FTE_JSON_VALUE_PTR  pValue;
}   FTE_JSON_PAIR, _PTR_ FTE_JSON_PAIR_PTR;

typedef struct  
{
    FTE_JSON_TYPE   xType;
    FTE_UINT32      nMaxCount;
    FTE_UINT32      nCount;
    FTE_JSON_PAIR_PTR   pPairs[];
}   FTE_JSON_OBJECT, _PTR_ FTE_JSON_OBJECT_PTR;

typedef struct  
{
    FTE_JSON_TYPE   xType;
    FTE_UINT32      nMaxCount;
    FTE_UINT32      nCount;
    FTE_JSON_VALUE_PTR  pElements[];
}   FTE_JSON_ARRAY, _PTR_ FTE_JSON_ARRAY_PTR;

typedef struct
{
    FTE_JSON_OBJECT_PTR pObject;
}   FTE_JSON, _PTR_ FTE_JSON_PTR;


FTE_RET FTE_JSON_init(FTE_JSON_PTR pJSON);
FTE_RET FTE_JSON_final(FTE_JSON_PTR pJSON);

FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createString(FTE_CHAR_PTR pString);
FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createNumber(long nValue);
FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createHex(long nValue);
FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createFloat(long nValue);
FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createPair(FTE_CHAR_PTR pString, FTE_JSON_VALUE_PTR pValue);
FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createObject(int nMaxCount);
FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createArray(int nMaxCount);
FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createTrue(void);
FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createFalse(void);
FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createNull(void);
FTE_JSON_VALUE_PTR  FTE_JSON_VALUE_createValue(FTE_VALUE_PTR pValue);

FTE_RET FTE_JSON_OBJECT_setPair(FTE_JSON_OBJECT_PTR pObject, FTE_CHAR_PTR pString, FTE_JSON_VALUE_PTR pValue);
FTE_RET FTE_JSON_ARRAY_setElement(FTE_JSON_ARRAY_PTR pArray, FTE_JSON_VALUE_PTR pElement);

FTE_RET FTE_JSON_OBJECT_addStringPair(FTE_JSON_OBJECT_PTR pObject, FTE_CHAR_PTR pName, FTE_CHAR_PTR pString);
FTE_RET FTE_JSON_OBJECT_addNumberPair(FTE_JSON_OBJECT_PTR pObject, FTE_CHAR_PTR pName, long nValue);

FTE_RET FTE_JSON_VALUE_destroy(FTE_JSON_VALUE_PTR pValue);
FTE_INT32   FTE_JSON_VALUE_snprint(FTE_CHAR_PTR pBuff, FTE_INT32 nBuffLen, FTE_JSON_VALUE_PTR pValue);
FTE_INT32   FTE_JSON_VALUE_buffSize(FTE_JSON_VALUE_PTR pValue);
#endif