#include <iostream>
#include <stdlib.h>   // system("pause");
#include <iomanip>    //format cout,setw，setprecision，dec， hex， oct，setfill
# include <windows.h>   // BITMAPFILEHEADER


using namespace std;


int64 t[40] = { 0 };

// just according theoretic formula,use double multiplication
// min time = 2680 ~ 3000 us
void convert2Gray0(uchar* in,uchar* out,int size)
{
	for (int i = 0; i < size/3; i++)
	{
		out[i] = 0.299 * (in[i * 3 + 2]) + 0.587 * (in[i * 3 + 1]) + 0.114 * (in[i * 3]) + 0.5;
	}
}

// use integer multiplication and one division
// min time = 950 ~ 1100 us
void convert2Gray1(uchar* in, uchar* out, int size)
{
	for (int i = 0; i < size / 3; i++)
	{
		out[i] = (299 * (in[i * 3 + 2]) + 587 * (in[i * 3 + 1]) + 114 * (in[i * 3]) + 500) / 1000;
	}
}

// use integer multiplication and right-shift(equal to division)
// min time = 620 ~ 750us
void convert2Gray2(uchar* in, uchar* out, int size)
{
	for (int i = 0; i < size / 3; i++)
	{
		out[i] = (1224 * (in[i * 3 + 2]) + 2404 * (in[i * 3 + 1]) + 467 * (in[i * 3]) + 2048) >> 12;
	}
}

// use table array and right-shift(equal to division)
int tblRed[256], tblGreen[256], tblBlue[256];     // min time=630 ~ 680 us
//uint tblRed[256], tblGreen[256], tblBlue[256];    // min time= 610 ~ 660 us
//uchar tblRed[256], tblGreen[256], tblBlue[256];     // min time = 600 ~ 650 us
void convert2Gray_initTable()
{
	for (int i = 0; i < 256; i++)
	{
		tblRed[i] = (i * 1224) >> 12;
		tblGreen[i] = (i * 2404) >> 12;
		tblBlue[i] = (i * 467) >> 12;
	}
}

//use search table,min time = 700 ~ 850us
void convert2Gray3(uchar* in, uchar* out, int size)
{
	for (int i = 0; i < size / 3; i++)
	{
		out[i] = tblRed[in[i * 3 + 2]] + tblGreen[in[i * 3 + 1]] + tblBlue[in[i * 3]];
	}
}

// use search table,parallel execute,min time = 650 ~ 750us
void convert2Gray4(uchar* in, uchar* out, int size)
{
	int j = 0;
	for (int i = 0; i < size / 3; i+=2)
	{
		j = i * 3;
		out[i] = tblRed[in[j + 2]] + tblGreen[in[j + 1]] + tblBlue[in[j]];
		out[i + 1] = tblRed[in[j + 5]] + tblGreen[in[j + 4]] + tblBlue[in[j + 3]];
		//out[i + 2] = tblRed[in[j + 8]] + tblGreen[in[j + 7]] + tblBlue[in[j + 6]];
		//out[i + 3] = tblRed[in[j + 11]] + tblGreen[in[j + 10]] + tblBlue[in[j + 9]];
		//out[i + 4] = tblRed[in[j + 14]] + tblGreen[in[j + 13]] + tblBlue[in[j + 12]];
		//out[i + 5] = tblRed[in[j + 17]] + tblGreen[in[j + 16]] + tblBlue[in[j + 15]];
		//out[i + 6] = tblRed[in[j + 20]] + tblGreen[in[j + 19]] + tblBlue[in[j + 18]];
		//out[i + 7] = tblRed[in[j + 23]] + tblGreen[in[j + 22]] + tblBlue[in[j + 21]];
	}
}

// uchar type table time ~ 1.2us,about 0.2us less than integer table
uchar utblRed[256], utblGreen[256], utblBlue[256];     
void convert2Gray_initTable1()
{
	for (uint i = 0; i < 256; i++)
	{
		utblRed[i] = (i * 1224) >> 12;
		utblGreen[i] = (i * 2404) >> 12;
		utblBlue[i] = (i * 467) >> 12;
	}
}

// parallel execute and inline, min time = 610 ~ 650 us, about 1~7us less than integer table
inline void convert2Gray5(uchar* in, uchar* out, uint size)  
{
	for (uint i = 0; i < size / 3; i += 2)
	{
		out[i] = utblRed[in[i * 3 + 2]] + utblGreen[in[i * 3  + 1]] + utblBlue[in[i * 3 ]];
		out[i + 1] = utblRed[in[i * 3  + 5]] + utblGreen[in[i * 3  + 4]] + utblBlue[in[i * 3  + 3]];
	}
}


int main(int argc, const char* argv[])
{

	BITMAPFILEHEADER fh;
	BITMAPINFOHEADER ih;
	
	ifstream src("../lena.bmp", ifstream::binary); 
	if (!src)
	{
		cerr << "Can't open the file.";
		return -1;
	}
	src.read((char *)&fh, sizeof(BITMAPFILEHEADER));
	if (fh.bfType != 'MB')
	{
		cerr << "type error:not bmp file!";
		return -1;
	}
	src.read((char *)&ih, sizeof(BITMAPINFOHEADER));
	if (ih.biBitCount != 24) {
		cout << "invalid bmp color bits!" << endl;
		return -1;
	}
		
	t[1] = getTickCount();
	//move read pointer to bmp data begin position
	src.seekg(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), ios::beg);	
	//start to read all data to array
	uchar* pData = new uchar[ih.biSizeImage];
	src.read((char *)pData, ih.biSizeImage);
	src.close();
	cout << hex << (unsigned int)(unsigned char)pData[0] << endl
		  << (unsigned int)(unsigned char)pData[ih.biSizeImage - 1] << endl;
	t[3] = getTickCount();
	t[4] = t[3] - t[1];

	uchar* grayData = new uchar[ih.biSizeImage];
	for (i = 0; i < 100; i++)
	{	
		convert2Gray0(pData, grayData, ih.biSizeImage);
	}
	t[5] = getTickCount();
	t[6] = t[5] - t[3];

	uchar* grayData1 = new uchar[ih.biSizeImage];
	for (i = 0; i < 100; i++)
	{
		convert2Gray1(pData, grayData1, ih.biSizeImage);
	}
	t[7] = getTickCount();
	t[8] = t[7] - t[5];

	uchar* grayData2 = new uchar[ih.biSizeImage];
	for (i = 0; i < 100; i++)
	{		
		convert2Gray2(pData, grayData2, ih.biSizeImage);
	}
	t[9] = getTickCount();
	t[10] = t[9] - t[7];

	convert2Gray_initTable();
	t[11] = getTickCount();
	t[12] = t[11] - t[9];

	uchar* grayData3 = new uchar[ih.biSizeImage];
	for (i = 0; i < 100; i++)
	{		
		convert2Gray3(pData, grayData3, ih.biSizeImage);
	}
	t[13] = getTickCount();
	t[14] = t[13] - t[11];

	uchar* grayData4 = new uchar[ih.biSizeImage];
	for (i = 0; i < 100; i++)
	{		
		convert2Gray4(pData, grayData4, ih.biSizeImage);
	}
	t[15] = getTickCount();
	t[16] = t[15] - t[13];

	convert2Gray_initTable1();
	t[17] = getTickCount();
	t[18] = t[17] - t[15];

	uchar* grayData5 = new uchar[ih.biSizeImage];
	for (i = 0; i < 100; i++)
	{
		convert2Gray5(pData, grayData5, ih.biSizeImage);
	}
	t[19] = getTickCount();
	t[20] = t[19] - t[17];

	cout << dec
		//<< " read file into rgb time = " << t[2] / 10000 << " ms;   " << endl
		//<< "  read file into array time = " << t[4] / 10 << " us;   " << endl
		<< " convert to gray0 time = " << t[6]/100 << " .1us;    " << endl
		<< " convert to gray1 time = " << t[8]/100 << " .1us;    " << endl
		<< " convert to gray2 time = " << t[10]/100 << " .1us;    " << endl
		<< " init integer color table time = " << t[12] << " .1us;    " << endl
	    << " convert to gray3 time = " << t[14]/100 << " .1us;    " << endl
	    << " convert to gray4 time = " << t[16]/100 << " .1us;    " << endl
		<< " init uchar color table time = " << t[18] << " .1us;    " << endl
		<< " convert to gray5 time = " << t[20] / 100 << " .1us;    " << endl;

	system("Pause");
	return 0;
}
