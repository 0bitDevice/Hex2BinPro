#ifndef _IHEX2BIN_H_
#define _IHEX2BIN_H_

enum ihexfmt_rtype
{
    IHEXFMT_RT_ERR_MIN = -1,
    IHEXFMT_RT_DATA = 0,
    IHEXFMT_RT_EOF,
    IHEXFMT_RT_EXTENDED_SEG_ADDR,
    IHEXFMT_RT_START_SEG_ADDR,
    IHEXFMT_RT_EXTENDED_LIN_ADDR,
    IHEXFMT_RT_START_LIN_ADDR,
    IHEXFMT_RT_SIZE = 6,
    IHEXFMT_RT_ERR_MAX = 6
};

typedef struct ihexfmt_data
{
    int record_type;
    int record_addr;
    int upper_addr;
    unsigned char data[256];
    int byte_count;
}ihexfmt_Data;

unsigned int ReadHexFile(const char *fn, ihexfmt_Data *cb, unsigned int length, unsigned int* realData_Length);
unsigned int FillHexTag(ihexfmt_Data *DstCb, unsigned int Dst_Length, const ihexfmt_Data *SrcCb, unsigned int Src_Length);
int ConvertMap(ihexfmt_Data *cb, unsigned int length, char* dataArray, unsigned int arrlength);
#endif
