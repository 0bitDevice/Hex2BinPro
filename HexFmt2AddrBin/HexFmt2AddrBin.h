// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 HEXFMT2ADDRBIN_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// HEXFMT2ADDRBIN_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef HEXFMT2ADDRBIN_EXPORTS
#define HEXFMT2ADDRBIN_API __declspec(dllexport)
#else
#define HEXFMT2ADDRBIN_API __declspec(dllimport)
#endif

// 此类是从 dll 导出的
class HEXFMT2ADDRBIN_API CHexFmt2AddrBin {
public:
	CHexFmt2AddrBin(void);
	// TODO: 在此处添加方法。
};

extern HEXFMT2ADDRBIN_API int nHexFmt2AddrBin;

HEXFMT2ADDRBIN_API int fnHexFmt2AddrBin(void);

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

HEXFMT2ADDRBIN_API size_t readHexFmtTagNum(const char* file_name);
HEXFMT2ADDRBIN_API size_t readHexFmtTagContent(const char* file_name, ihexfmt_Data* pHexFmt_data, size_t tag_Num);
HEXFMT2ADDRBIN_API size_t convertHex2Bin(char* data_Array, size_t data_Length, ihexfmt_Data* pHexFmt_data, size_t tag_Num);
HEXFMT2ADDRBIN_API int HexFmt2AddrBin(const char* file_name, char * pData);
HEXFMT2ADDRBIN_API int testStrPtr(char * pData)
{
	char testArray[] = "It's me,testStrPtr!!";
	strcpy_s(pData, sizeof(testArray), testArray);

	return 0;
}