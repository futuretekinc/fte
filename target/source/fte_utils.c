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
(   char_ptr         arg, 
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

boolean fte_parse_float
(    char_ptr    pString, 
    double _PTR_ pValue
)
{
    int     i;
    double  dValue = 0;
    boolean bNagative = FALSE;
    boolean bInteger = TRUE;
    double  dDevider = 10;
    
    if ((pString == NULL) && (strlen(pString) == 0))
    {
        return  FALSE;
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
        return  FALSE;
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
                return  FALSE;
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
    
    return  TRUE;

}
