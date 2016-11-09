#include "fte_target.h"

 
void FTE_udelay
(
    FTE_UINT32  ulMicroSecs
)
{
    for(int j = ulMicroSecs*20 ; j > 0 ; j--);
}

FTE_RET FTE_strToIP
(    FTE_CHAR_PTR    pIPString, 
    FTE_UINT32_PTR  pIP
)
{
   FTE_UINT32      nNum[4] =  { 0 };
   FTE_UINT32      i, index = 0;
   FTE_UINT32      nTemp = 0;
   
   ASSERT(pIP != NULL);
   
   for (i=0; pIPString[i] && (i<16) && (index<4); i++)  
   {
      if (isdigit(pIPString[i]))  
      {
         nNum[index] = nNum[index]*10 + pIPString[i]-'0';
      } 
      else if (pIPString[i] == '.') 
      {
         index++; 
      } 
      else  
      {
         return FTE_RET_INVALID_ARGUMENT;
      }
   }
   
   if ((pIPString[i] == '\0') && (index==3))  
   {
      for (i=0;i<4;i++)  
      {
         if (nNum[i] > 255)  
         {
            return FTE_RET_INVALID_ARGUMENT;
         } 
         else  
         {
            nTemp = (nTemp << 8) | nNum[i];
         }
      } 
   } 

   *pIP = nTemp;
   
   return FTE_RET_OK;
}

FTE_RET     FTE_strToMAC
(
   FTE_CHAR_PTR     pString, 
   _enet_address    enet_address
)
{
    FTE_INT32    i,j=0;
   
    if (strlen(pString) != 17)  
    {
        return FTE_RET_INVALID_ARGUMENT;
    }
   
    for (i=0;i<16;i+=3)  
    {
        if ( isxdigit(pString[i]) && isxdigit(pString[i+1]) && ((pString[i+2]==':') || (pString[i+2]=='\0')) )  
        {
            enet_address[j++] = hexnum(pString[i]) * 16 + hexnum(pString[i+1]);
        } 
        else  
        {
            return FTE_RET_INVALID_ARGUMENT;
        }
    } 
    
   return FTE_RET_OK;   
}

FTE_RET FTE_strToFLOAT
(
    FTE_CHAR_PTR    pString, 
    FTE_FLOAT_PTR   pValue
)
{
    ASSERT((pString != NULL) && (pValue != NULL));
    
    FTE_INT32   i;
    FTE_FLOAT   dValue = 0;
    FTE_BOOL    bNagative = FALSE;
    FTE_BOOL    bInteger = TRUE;
    FTE_FLOAT   dDevider = 10;
    
    if (strlen(pString) == 0)
    {
        *pValue = 0;
        return  FTE_RET_OK;
    }

    for(i = 0 ; i < strlen(pString) ; i++)
    {
        if (!isspace(pString[i])) 
        {   
            break;
        }
    }
    
    if (pString[i] == '-')
    {
        bNagative = TRUE;
        i++;
    }
    else if (pString[i] == '+')
    {
        i++;
    }
    else if (!isdigit(pString[i]))
    {
        return  FTE_RET_INVALID_ARGUMENT;
    }
    
    for(; i < strlen(pString) ; i++)
    {
        if (isdigit(pString[i]))
        {
            if (bInteger)
            {
                dValue = dValue * 10 + (pString[i] - '0');
            }
            else
            {
                dValue += (pString[i] - '0') / dDevider;
                dDevider *= 10;
            }
        }
        else if (pString[i] == '.')
        {
            if (!bInteger)
            {
                return  FTE_RET_INVALID_ARGUMENT;
            }
            
            bInteger = FALSE;
        }
        else
        {
            return  FALSE;
        }
    }
    
    if (bNagative)
    {
        *pValue = 0 - dValue;
    }
    else
    {
        *pValue = dValue;
    }
    
    return  FTE_RET_OK;

}

FTE_RET FTE_strToHexArray
(
    FTE_CHAR_PTR    pString, 
    FTE_UINT8_PTR   pBuff, 
    FTE_UINT32      ulBuffLen,
    FTE_UINT32_PTR  pLen
)
{
    ASSERT((pString != NULL) && (pBuff != NULL) && (pLen != NULL));
    
    FTE_UINT32 ulStringLen = strlen(pString);
    
    if (((ulStringLen % 2) == 1) || ((ulStringLen / 2) > ulBuffLen))
    {
        *pLen = 0;
        return  FTE_RET_OK;
    }
    
    for(int i = 0 ; i < ulStringLen ;i++)
    {
        if (!(('0' <= pString[i] && pString[i] <= '9') || 
              ('A' <= pString[i] && pString[i] <= 'F') ||
              ('a' <= pString[i] && pString[i] <= 'f')))
        {
            *pLen = 0;
            return  FTE_RET_INVALID_ARGUMENT;
        }
    }

   for(int i = 0 ; i <  ulStringLen  / 2; i++)
   {
        if ('0' <= pString[i*2] && pString[i*2] <= '9')
        {
           pBuff[i] = pString[i*2] - '0'; 
        }
        else if ('A' <= pString[i*2] && pString[i*2] <= 'F')
        {
           pBuff[i] = pString[i*2] - 'A' + 10; 
        }
        else if ('a' <= pString[i*2] && pString[i*2] <= 'f')
        {
           pBuff[i] = pString[i*2] - 'a' + 10; 
        }

        if ('0' <= pString[i*2 + 1] && pString[i*2 + 1] <= '9')
        {
           pBuff[i] = (pBuff[i] << 4) + pString[i*2 + 1] - '0'; 
        }
        else if ('A' <= pString[i*2 + 1] && pString[i*2 + 1] <= 'F')
        {
           pBuff[i] = (pBuff[i] << 4) + pString[i*2 + 1] - 'A' + 10; 
        }
        else if ('a' <= pString[i*2 + 1] && pString[i*2 + 1] <= 'f')
        {
           pBuff[i] = (pBuff[i] << 4) + pString[i*2 + 1] - 'a' + 10; 
        }
   }
   
   *pLen = ulStringLen / 2;
   
   return   FTE_RET_OK;
}

FTE_RET FTE_strToUINT16
(   FTE_CHAR_PTR     pString, 
   FTE_UINT16_PTR   pValue
)
{
   FTE_UINT32   i=0;
   FTE_UINT16   usValue=0;

   ASSERT((pString != NULL) && (pValue != NULL));
   
   while (isdigit(pString[i]))  
   {
      usValue = usValue*10 + (pString[i++]-'0');
   }
   
   if (pString[i] != '\0')
   {
      return FTE_RET_INVALID_ARGUMENT;
   }
   
    *pValue = usValue;
   
   return FTE_RET_OK;
}

FTE_RET FTE_strToUINT32
(
    FTE_CHAR_PTR     pString,
    FTE_UINT32_PTR   pValue
)
{
    ASSERT((pString != NULL) && (pValue != NULL));
    
    FTE_UINT32  i = 0;
    FTE_UINT32  ulValue = 0;
   
   while (isdigit(pString[i]))  
   {
        ulValue = ulValue * 10 + (pString[i++]-'0');
   }
   
   if (pString[i]!='\0')
   {
       return   FTE_RET_INVALID_ARGUMENT;
   }
   
   *pValue = ulValue;
   
   return FTE_RET_OK;
}

FTE_RET FTE_strToHex
(
    FTE_CHAR_PTR     pString,
    FTE_UINT32_PTR   pValue
)
{
    if ((pString == NULL) || (pValue == NULL))
    {
        return  FTE_RET_ASSERT;
    }
    
    FTE_UINT32  ulValue = 0;
    FTE_UINT32  ulStringLen = strlen(pString);
    
    for(FTE_INT32   i = 0 ; i < ulStringLen ;i++)
    {
        if ('0' <= pString[i] && pString[i] <= '9')
        {
            ulValue = ulValue * 16 + (pString[i] - '0');
        }
        else if ('A' <= pString[i] && pString[i] <= 'F')
        {
            ulValue = ulValue * 16 + (pString[i] - 'A') + 10;
        }
        else if ('a' <= pString[i] && pString[i] <= 'f')
        {
            ulValue = ulValue * 16 + (pString[i] - 'a') + 10;
        }
        else {
            return   FTE_RET_INVALID_ARGUMENT;
        }
    }

    
   *pValue = ulValue;
   
   return FTE_RET_OK;
}
