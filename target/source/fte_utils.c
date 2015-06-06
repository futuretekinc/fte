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

boolean fte_parse_enet_address( char_ptr arg, _enet_address enet_address)
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
