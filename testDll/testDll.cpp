// testDll.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <Windows.h>

typedef struct ihexfmt_data
{
	int record_type;
	int record_addr;
	int upper_addr;
	unsigned char data[256];
	int byte_count;
}ihexfmt_Data;

int main()
{
	typedef size_t(*_readHexFmtTagNum)(const char* file_name);
	typedef size_t(*_readHexFmtTagContent)(const char* file_name, ihexfmt_Data* pHexFmt_data, size_t tag_Num);
	typedef size_t(*_HexFmt2AddrBin)(const char* file_name, char * pData);

	_readHexFmtTagNum		readHexFmtTagNum;
	_readHexFmtTagContent	readHexFmtTagContent;
	_HexFmt2AddrBin			HexFmt2AddrBin;
	HINSTANCE hDll = LoadLibrary(L"HEXFMT2ADDRBIN.dll");

	readHexFmtTagNum = (_readHexFmtTagNum)GetProcAddress(hDll, "readHexFmtTagNum");
	readHexFmtTagContent = (_readHexFmtTagContent)GetProcAddress(hDll, "readHexFmtTagContent");
	HexFmt2AddrBin= (_HexFmt2AddrBin)GetProcAddress(hDll, "HexFmt2AddrBin");


	size_t tagNum = readHexFmtTagNum("BMU_B21.hex");

	ihexfmt_Data* pihexfmt_Data = (ihexfmt_Data*)malloc(tagNum * sizeof(ihexfmt_Data));

	size_t charNum = readHexFmtTagContent("BMU_B21.hex", pihexfmt_Data, tagNum);
	free(pihexfmt_Data);
	char* pData = (char*)malloc(charNum * 0x20a);
	HexFmt2AddrBin("BMU_B21.hex", pData);

	FILE* wt;
	fopen_s(&wt, "BMU_B21.boot", "w+");
	fwrite(pData, sizeof(char), charNum * 0x20a, wt);
	fclose(wt);

	free(pData);
	FreeLibrary(hDll);
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
