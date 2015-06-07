#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "crc32.h"

char *pVendor = "FutureTek,Inc.";
char *pModel = "FTE Series";

typedef	struct
{
	char		pModel[32];
	char		pVendor[32];
	uint32_t	ulVersion;
	uint32_t	ulLen;
	uint32_t	ulCheckSum;
}	MKIMAGE_HEADER;


/***************************************************************************/

unsigned int char_to_uint(char c)
{
	int res = 0;

	if (c >= '0' && c <= '9')
		res = (c - '0');
	else if (c >= 'A' && c <= 'F')
		res = (c - 'A' + 10);
	else if (c >= 'a' && c <= 'f')
		res = (c - 'a' + 10);

	return(res);
}

/***************************************************************************/

uint32_t str_to_uint32(char *s)
{
	int i;
	char c;
	uint32_t res = 0;

	for (i = 0; (i < 8) && (s[i] != '\0'); i++)
	{
		c = s[i];
		res <<= 4;
		res += char_to_uint(c);
	}

	return(res);
}


uint32_t file_size(FILE *f)
{
	struct stat info;

	if (!fstat(fileno(f), &info))
		return((uint32_t)info.st_size);
	else
		return(0);
}

int main(int nArgc, char *pArgv[])
{
	FILE 			*pInFile;
	FILE 			*pOutFile;
	int				i;
	int				nInFileSize;	
	char			*pBuff;
	char			*pInFileName = NULL;
	char			*pOutFileName = NULL;
	MKIMAGE_HEADER	*pHeader;
	time_t		xTime;

	for(i = 1; i < nArgc ; i++)
	{
		if (!strcmp(pArgv[i], "-V"))
		{
			pVendor = pArgv[++i];
		}
		else if (!strcmp(pArgv[i], "-m"))
		{
			pModel = pArgv[++i];
		}
		else if (!strcmp(pArgv[i], "-o"))
		{
			pOutFileName = pArgv[++i];
		}
		else
		{
			pInFileName = pArgv[i];
		}
	}

	pInFile = fopen(pInFileName, "rb");
	if (pInFile == NULL)
	{
		fprintf(stderr, "Can't open file[%s]\n", pInFileName);
		return (3);
	}

	nInFileSize = file_size(pInFile) - 4;
	pBuff = (char *)malloc(4096 + nInFileSize);
	if (pBuff == NULL)
	{
		fprintf(stderr, "Not enough memory\n");
		fclose(pInFile);
		return (3);
	}

	memset(pBuff, 0, 4096 + nInFileSize);

	pHeader = (MKIMAGE_HEADER *)pBuff;
	pBuff += 4096;

	if (nInFileSize != fread(pBuff, 1, nInFileSize, pInFile))
	{
		fclose(pInFile);
		free(pBuff);	
		fprintf(stderr, "File size mismatch!\n");
		return (3);
	}

	fclose(pInFile);

	strcpy(pHeader->pVendor, pVendor);
	strcpy(pHeader->pModel, pModel);
	pHeader->ulVersion = time(&xTime);
	pHeader->ulLen = nInFileSize;
	pHeader->ulCheckSum = crc32(0, pBuff, nInFileSize);

	printf("%16s : %s\n", 	"Vendor", 		pHeader->pVendor);
	printf("%16s : %s\n", 	"Model", 		pHeader->pModel);
	xTime = (time_t)pHeader->ulVersion;
	printf("%16s : %s\n",	"Version",		ctime(&xTime));
	printf("%16s : %d\n", 	"Image Size", 	pHeader->ulLen);
	printf("%16s : %08x\n", "Check Sum", 	pHeader->ulCheckSum);

	if (pOutFileName != NULL)
	{
		pOutFile = fopen(pOutFileName, "wb");
		if (pOutFile != NULL)
		{
			fwrite(pHeader, 1, nInFileSize + 4096, pOutFile);
			fclose(pOutFile);
		}
	}

	free(pHeader);

	return	0;	
}
