#include <stdlib.h>
#include <stdio.h>

#include "iHex2Bin.h"

int main(void)
{
    unsigned int realCbsLength = 0;
	unsigned int realDataLength = 0;
	unsigned int realFilledCbsLength = 0;

	ihexfmt_Data mData[2000];
	ihexfmt_Data* pOriginHexFillData;
	ihexfmt_Data* pHexFillData;
	ihexfmt_Data arrayHexFillData[0xc8];

	char* pCharArray;
	char arrayChar[0xc8*(0x200+8+2)];
	const int PackLen = 0x20a;

	size_t lineCounter = 0;

	FILE* wt;
	fopen_s(&wt, "BMU_B21.boot", "w+");

	lineCounter = readHexFmtTagNum("BMU_B21.hex");

	pOriginHexFillData = (ihexfmt_Data*)malloc(lineCounter * sizeof(ihexfmt_Data));
	realCbsLength = ReadHexFile("BMU_B21.hex", pOriginHexFillData, lineCounter, &realDataLength);
	
	pHexFillData = (ihexfmt_Data*)malloc(realCbsLength * sizeof(ihexfmt_Data));
	
	realFilledCbsLength = FillHexTag(pHexFillData, realCbsLength, pOriginHexFillData, realCbsLength);
//	realFilledCbsLength = FillHexTag(arrayHexFillData, realCbsLength, mData, realCbsLength);
	
	pCharArray = (char*)malloc(realFilledCbsLength * PackLen);
	ConvertMap(pHexFillData, realFilledCbsLength, pCharArray, realFilledCbsLength * PackLen);
//	ConvertMap(arrayHexFillData, realFilledCbsLength, arrayChar, realFilledCbsLength * PackLen);

	fwrite(pCharArray, sizeof(char), realFilledCbsLength * PackLen, wt);
//	fwrite(arrayChar, sizeof(char), realFilledCbsLength * PackLen, wt);
	fclose(wt);
	free(pCharArray);
	free(pHexFillData);
	free(pOriginHexFillData);

    return 0;
}
