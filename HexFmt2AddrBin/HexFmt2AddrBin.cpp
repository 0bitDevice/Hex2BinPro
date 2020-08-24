// HexFmt2AddrBin.cpp : 定义 DLL 的导出函数。
//

#include "pch.h"
#include "framework.h"
#include "HexFmt2AddrBin.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>


#define CHAR_FILLED		0xFF
#define PACK_LENGTH		0x20a


static int hex_num(int c)
{
	c = tolower(c);
	if (c >= 'a' && c <= 'f')
	{
		return 0x0A + c - 'a';
	}
	if (c >= '0' && c <= '9')
	{
		return c - '0';
	}

	return -1;
}

static int get_hex_num(FILE *f, int bytes, int *checksum)
{
	int num = 0;
	int byte;
	int i;
	int j;

	for (i = bytes - 1; i >= 0; i--)
	{
		byte = 0x00;
		for (j = 1; j >= 0; j--)
		{
			int c = fgetc(f);
			if (c == EOF)
			{
				return -1;
			}

			c = hex_num(c);
			if (c == -1)
			{
				return -1;
			}

			byte += (c << (4 * j));
		}

		if (checksum != NULL)
		{
			*checksum += byte;
		}

		num += (byte << (8 * i));
	}

	return num;
}

static int get_nums(FILE *f, unsigned char *bytes, int elements, int *checksum)
{
	int i;

	for (i = 0; i < elements; i++)
	{
		int c = get_hex_num(f, 1, checksum);
		if (c < 0)
		{
			return c;
		}
		bytes[i] = c;
	}

	return EXIT_SUCCESS;
}

static void byteArrayCat(unsigned char* Dst, unsigned int len, const unsigned char* Src)
{
	for (unsigned int i = 0; i < len; ++i)
	{
		*Dst++ = *Src++;
	}
}

static void fillNullSpace(unsigned char *Dst, unsigned int len)
{
	for (unsigned int i = 0; i < len; ++i)
	{
		*Dst++ = (char)CHAR_FILLED;
	}
}

static unsigned int ReadHexFile(const char *fn, ihexfmt_Data *cbs, unsigned int length)
{
	ihexfmt_Data app_data;
	int computed_checksum;
	int upper_addr = 0x00;
	int rc = EXIT_FAILURE;
	int checksum;
	FILE *f;
	int c;
	unsigned int i = 0;

	memset(&app_data, 0x00, sizeof(app_data));

	fopen_s(&f, fn, "r");
	if (f == NULL)
	{
		return -EIO;
	}

	while (!feof(f))
	{
		computed_checksum = 0x00;

		c = fgetc(f);
		if (c != ':')
		{
			rc = -EINVAL;
			goto clean_exit;
		}

		app_data.byte_count = get_hex_num(f, 1, &computed_checksum);
		app_data.record_addr = get_hex_num(f, 2, &computed_checksum);
		app_data.record_type = get_hex_num(f, 1, &computed_checksum);
		rc = get_nums(f,
			app_data.data,
			app_data.byte_count,
			&computed_checksum);
		checksum = get_hex_num(f, 1, NULL);

		if (app_data.byte_count < 0 ||
			app_data.record_addr < 0 ||
			app_data.record_type <= IHEXFMT_RT_ERR_MIN ||
			app_data.record_type >= IHEXFMT_RT_ERR_MAX ||
			rc < 0)
		{
			rc = -EINVAL;
			goto clean_exit;
		}

		computed_checksum = (~computed_checksum + 1) & 0xFF;
		if (computed_checksum != checksum)
		{
			rc = -EINVAL;
			goto clean_exit;
		}

		switch (app_data.record_type)
		{
		case IHEXFMT_RT_EXTENDED_LIN_ADDR:
			upper_addr = (app_data.data[0] << 8) + app_data.data[1];
		case IHEXFMT_RT_EXTENDED_SEG_ADDR:
			if (app_data.byte_count != 2)
			{
				rc = -EINVAL;
				goto clean_exit;
			}
			break;
		case IHEXFMT_RT_DATA:
			app_data.upper_addr = upper_addr;
			memcpy(&cbs[i], &app_data, sizeof(app_data));
			++i;
			break;
		case IHEXFMT_RT_START_SEG_ADDR:
		case IHEXFMT_RT_START_LIN_ADDR:
			if (app_data.byte_count != 4)
			{
				rc = -EINVAL;
				goto clean_exit;
			}
			break;
		case IHEXFMT_RT_EOF:
			if (app_data.byte_count != 0)
			{
				rc = -EINVAL;
				goto clean_exit;
			}
			break;
		}

		if (i > length - 1)
		{
			rc = -EINVAL;
			goto clean_exit;
		}

		while (!feof(f))
		{
			c = fgetc(f);
			if (!isspace(c))
			{
				ungetc(c, f);
				break;
			}
		}

	}

clean_exit:
	fclose(f);

	return i;
}

unsigned int FillHexTag(ihexfmt_Data *DstCb, unsigned int Dst_Length, const ihexfmt_Data *SrcCb, unsigned int Src_Length)
{
	int tempAddr = 0, offsetAddr = 0, record_addr = 0, preByte_Counter = 0;
	unsigned int IndexDstCb = 0, IndexDstData = 0, byteCounterLeft = 0;

	memset(&DstCb[IndexDstCb], CHAR_FILLED, sizeof(ihexfmt_Data));
	DstCb[IndexDstCb].record_addr = SrcCb[0].record_addr;
	DstCb[IndexDstCb].upper_addr = SrcCb[0].upper_addr;
	byteArrayCat(&(DstCb[IndexDstCb].data[IndexDstData]), SrcCb[0].byte_count, SrcCb[0].data);
	IndexDstData += SrcCb[0].byte_count;

	for (size_t i = 1; i < Src_Length; ++i)
	{
		tempAddr = SrcCb[i].record_addr % 0x100;
		offsetAddr = ((SrcCb[i].upper_addr - SrcCb[i - 1].upper_addr) << 16)
			+ (SrcCb[i].record_addr - SrcCb[i - 1].record_addr);
		preByte_Counter = SrcCb[i - 1].byte_count;

		if (offsetAddr == preByte_Counter)
		{
			if (IndexDstData == 0x100)
			{
				++IndexDstCb;
				IndexDstData = 0;
				record_addr += 0x100;
				memset(&DstCb[IndexDstCb], CHAR_FILLED, sizeof(ihexfmt_Data));
				DstCb[IndexDstCb].record_addr = record_addr;
				DstCb[IndexDstCb].upper_addr = SrcCb[i].upper_addr;
				byteArrayCat(&(DstCb[IndexDstCb].data[IndexDstData]), SrcCb[i].byte_count, SrcCb[i].data);
				IndexDstData += SrcCb[i].byte_count;
			}
			else if ((IndexDstData + SrcCb[i].byte_count) > 0x100)
			{
				byteCounterLeft = 0x100 - IndexDstData;
				byteArrayCat(&(DstCb[IndexDstCb].data[IndexDstData]), byteCounterLeft, SrcCb[i].data);
				++IndexDstCb;
				IndexDstData = 0;
				record_addr += 0x100;
				memset(&DstCb[IndexDstCb], CHAR_FILLED, sizeof(ihexfmt_Data));
				DstCb[IndexDstCb].record_addr = record_addr;
				DstCb[IndexDstCb].upper_addr = SrcCb[i].upper_addr;
				byteArrayCat(&(DstCb[IndexDstCb].data[IndexDstData]), SrcCb[i].byte_count - byteCounterLeft, &(SrcCb[i].data[byteCounterLeft]));
				IndexDstData += SrcCb[i].byte_count - byteCounterLeft;
			}
			else
			{
				byteArrayCat(&(DstCb[IndexDstCb].data[IndexDstData]), SrcCb[i].byte_count, SrcCb[i].data);
				IndexDstData += SrcCb[i].byte_count;
			}
		}
		else
		{
			if ((offsetAddr - preByte_Counter) < 0x100)
			{
				byteCounterLeft = offsetAddr - preByte_Counter;
				if ((IndexDstData + byteCounterLeft) > 0x100)
				{
					fillNullSpace(&(DstCb[IndexDstCb].data[IndexDstData]), 0x100 - IndexDstData);
					byteCounterLeft -= 0x100 - IndexDstData;

					++IndexDstCb;
					IndexDstData = 0;
					record_addr += 0x100;
					memset(&DstCb[IndexDstCb], CHAR_FILLED, sizeof(ihexfmt_Data));
					DstCb[IndexDstCb].record_addr = record_addr;
					DstCb[IndexDstCb].upper_addr = SrcCb[i].upper_addr;
				}

				fillNullSpace(&(DstCb[IndexDstCb].data[IndexDstData]), byteCounterLeft);
				IndexDstData += byteCounterLeft;

				if ((IndexDstData + SrcCb[i].byte_count) > 0x100)
				{
					byteCounterLeft = 0x100 - IndexDstData;
					byteArrayCat(&(DstCb[IndexDstCb].data[IndexDstData]), byteCounterLeft, SrcCb[i].data);
					++IndexDstCb;
					IndexDstData = 0;
					record_addr += 0x100;
					memset(&DstCb[IndexDstCb], CHAR_FILLED, sizeof(ihexfmt_Data));
					DstCb[IndexDstCb].record_addr = record_addr;
					DstCb[IndexDstCb].upper_addr = SrcCb[i].upper_addr;
					byteArrayCat(&(DstCb[IndexDstCb].data[IndexDstData]), SrcCb[i].byte_count - byteCounterLeft, &(SrcCb[i].data[byteCounterLeft]));
					IndexDstData += SrcCb[i].byte_count;
				}
				else
				{
					byteArrayCat(&(DstCb[IndexDstCb].data[IndexDstData]), SrcCb[i].byte_count, SrcCb[i].data);
					IndexDstData += SrcCb[i].byte_count;
				}
			}
			else
			{
				++IndexDstCb;
				IndexDstData = 0;
				fillNullSpace(&(DstCb[IndexDstCb].data[IndexDstData]), tempAddr);
				IndexDstData += tempAddr;
				record_addr = SrcCb[i].record_addr - tempAddr;
				memset(&DstCb[IndexDstCb], CHAR_FILLED, sizeof(ihexfmt_Data));
				DstCb[IndexDstCb].record_addr = record_addr;
				DstCb[IndexDstCb].upper_addr = SrcCb[i].upper_addr;
				byteArrayCat(&(DstCb[IndexDstCb].data[IndexDstData]), SrcCb[i].byte_count, SrcCb[i].data);
				IndexDstData += SrcCb[i].byte_count;
			}
		}

	}
	return IndexDstCb + 1;
}

// 这是导出变量的一个示例
HEXFMT2ADDRBIN_API int nHexFmt2AddrBin=0;

// 这是导出函数的一个示例。
HEXFMT2ADDRBIN_API int fnHexFmt2AddrBin(void)
{
    return 0;
}

// 这是已导出类的构造函数。
CHexFmt2AddrBin::CHexFmt2AddrBin()
{
    return;
}



HEXFMT2ADDRBIN_API size_t readHexFmtTagNum(const char* file_name)
{
	ihexfmt_Data app_data;
	int computed_checksum;
	size_t rc = 0;
	int checksum;
	FILE *f;
	int c;
	size_t tagCounter = 0;

	fopen_s(&f, file_name, "r");
	if (f == NULL)
	{
		return 0;
	}

	while (!feof(f))
	{
		computed_checksum = 0x00;

		c = fgetc(f);
		if (c != ':')
		{
			rc = 0;
			goto clean_exit;
		}

		app_data.byte_count = get_hex_num(f, 1, &computed_checksum);
		app_data.record_addr = get_hex_num(f, 2, &computed_checksum);
		app_data.record_type = get_hex_num(f, 1, &computed_checksum);
		rc = get_nums(f,
			app_data.data,
			app_data.byte_count,
			&computed_checksum);
		checksum = get_hex_num(f, 1, NULL);

		if (app_data.byte_count < 0 ||
			app_data.record_addr < 0 ||
			app_data.record_type <= IHEXFMT_RT_ERR_MIN ||
			app_data.record_type >= IHEXFMT_RT_ERR_MAX ||
			rc < 0)
		{
			rc = 0;
			goto clean_exit;
		}

		computed_checksum = (~computed_checksum + 1) & 0xFF;
		if (computed_checksum != checksum)
		{
			rc = 0;
			goto clean_exit;
		}

		switch (app_data.record_type)
		{
		case IHEXFMT_RT_EXTENDED_LIN_ADDR:
		case IHEXFMT_RT_EXTENDED_SEG_ADDR:
			if (app_data.byte_count != 2)
			{
				rc = 0;
				goto clean_exit;
			}
			break;
		case IHEXFMT_RT_DATA:
			++tagCounter;
			break;
		case IHEXFMT_RT_START_SEG_ADDR:
		case IHEXFMT_RT_START_LIN_ADDR:
			if (app_data.byte_count != 4)
			{
				rc = 0;
				goto clean_exit;
			}
			break;
		case IHEXFMT_RT_EOF:
			if (app_data.byte_count != 0)
			{
				rc = 0;
				goto clean_exit;
			}
			break;
		}

		while (!feof(f))
		{
			c = fgetc(f);
			if (!isspace(c))
			{
				ungetc(c, f);
				break;
			}
		}

	}

	fclose(f);

	return tagCounter;

clean_exit:
	fclose(f);

	return rc;
}

HEXFMT2ADDRBIN_API size_t readHexFmtTagContent(const char* file_name, ihexfmt_Data* pHexFmt_data, size_t tag_Num)
{
	ihexfmt_Data* pOriginHexFillData;
	unsigned int realFilledCbsLength = 0;

	pOriginHexFillData = (ihexfmt_Data*)malloc(tag_Num * sizeof(ihexfmt_Data));
	ReadHexFile(file_name, pOriginHexFillData, tag_Num);
	realFilledCbsLength = FillHexTag(pHexFmt_data, tag_Num, pOriginHexFillData, tag_Num);

	free(pOriginHexFillData);

	return realFilledCbsLength;
}

HEXFMT2ADDRBIN_API size_t convertHex2Bin(char* data_Array, size_t data_Length, ihexfmt_Data* pHexFmt_data, size_t tag_Num)
{
	unsigned int i = 0, indexArray = 0;
	int dataAddr = 0, preDataAddr = 0;
	char hexData[2] = { 0 };

	while (tag_Num--)
	{
		//计算地址
		dataAddr = (pHexFmt_data[i].upper_addr << 16) + pHexFmt_data[i].record_addr;
		sprintf_s(&data_Array[indexArray], 9, "%x", dataAddr);
		indexArray += 8;
		data_Array[indexArray++] = '\n';

		for (int countChar = 0; countChar < 0x100; ++countChar)
		{
			sprintf_s(hexData, 2, "%X", (pHexFmt_data[i].data[countChar] & 0xF0) >> 4);
			data_Array[indexArray++] = hexData[0];
			sprintf_s(hexData, 2, "%X", (pHexFmt_data[i].data[countChar] & 0x0F));
			data_Array[indexArray++] = hexData[0];
		}

		data_Array[indexArray++] = '\n';
		++i;
	}

	return EXIT_SUCCESS;
}

int HexFmt2AddrBin(const char* file_name, char * pData)
{
	size_t tagNum = readHexFmtTagNum(file_name);
	ihexfmt_Data* pihexfmt_Data = (ihexfmt_Data*)malloc(tagNum * sizeof(ihexfmt_Data));
	size_t tagFilledNum = readHexFmtTagContent("BMU_B21.hex", pihexfmt_Data, tagNum);
	char* pContentData = (char*)malloc(tagFilledNum * PACK_LENGTH);
	convertHex2Bin(pContentData, tagFilledNum * PACK_LENGTH, pihexfmt_Data, tagFilledNum);

	memcpy_s(pData, tagFilledNum * PACK_LENGTH, pContentData, tagFilledNum * PACK_LENGTH);
	pData[tagFilledNum * PACK_LENGTH] = '\0';

	free(pihexfmt_Data);
	free(pContentData);

	return EXIT_SUCCESS;
}

