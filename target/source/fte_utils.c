#include "fte_target.h"

 
void fte_udelay(uint_32 usec)
{
    for(int j = usec*20 ; j > 0 ; j--);
}

boolean fte_parse_ip_address( char_ptr pIPString, uint_32 _PTR_ pIP)
{
   uint_32      nNum[4] =  { 0 };
   uint_32      i, index = 0;
   uint_32      nTemp = 0;
   
   if (pIP == NULL) 
   {
       return FALSE;
   }
   
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
         return FALSE;
      }
   }
   
   if ((pIPString[i] == '\0') && (index==3))  
   {
      for (i=0;i<4;i++)  
      {
         if (nNum[i] > 255)  
         {
            return FALSE;
         } 
         else  
         {
            nTemp = (nTemp << 8) | nNum[i];
         }
      } 
   } 

   *pIP = nTemp;
   
   return TRUE;
}

boolean fte_parse_enet_address
(
   char_ptr         arg, 
   _enet_address    enet_address
)
{
   int i,j=0;
   
   if (strlen(arg) != 17)  {
      return FALSE;
   }
   
   for (i=0;i<16;i+=3)  {
      if ( isxdigit(arg[i]) && isxdigit(arg[i+1]) && ((arg[i+2]==':') || (arg[i+2]=='\0')) )  {
         enet_address[j++] = hexnum(arg[i]) * 16 + hexnum(arg[i+1]);
      } else  {
         return FALSE;
      }
   } 
   return TRUE;   
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

uint_32 fte_parse_hex_string(char_ptr pString, uint_8 *pBuff, uint_32 ulBuffLen)
{
    uint_32 ulStringLen = strlen(pString);
    
    if (((ulStringLen % 2) == 1) || ((ulStringLen / 2) > ulBuffLen))
    {
        return  0;
    }
    
    for(int i = 0 ; i < ulStringLen ;i++)
    {
        if (!(('0' <= pString[i] && pString[i] <= '9') || 
              ('A' <= pString[i] && pString[i] <= 'F') ||
              ('a' <= pString[i] && pString[i] <= 'f')))
        {
            return  0;
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
   
   return   ulStringLen / 2;
}

FTE_RET FTE_strToUINT32
(
    FTE_CHAR_PTR     pString,
    FTE_UINT32_PTR   pValue
)
{
    if ((pString == NULL) || (pValue == NULL))
    {
        return  FTE_RET_ASSERT;
    }
    
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
