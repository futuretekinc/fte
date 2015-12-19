#ifndef __FTE_ASSERT_H__
#define __FTE_ASSERT_H__    1

#define ASSERT(x)   {   if (!(x)) {printf("%s[%d] : ASSERTED(%s)\n", __func__, __LINE__, #x); while(1);}}

#endif
