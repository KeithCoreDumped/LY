// LY.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
// LY V2.0

// TODO:
// CFG
// LOG

#include "pch.h"
#include <iostream>
#include <fstream>
#include <time.h>
#include <stdio.h>
#include <string>
#include <Windows.h>
#include <io.h>

#pragma comment(lib, "winmm.lib")
//#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"") //隐藏窗口

#define TRACE printf
#define exist(x) (!_access(x, 0))

HWAVEIN hWaveIn;                                           //输入设备
WAVEFORMATEX waveform;                                     //采集音频的格式，结构体
BYTE *pBuffer1, *pBuffer2;                                 //采集音频时的数据缓存
WAVEHDR wHdr1;                                             //采集音频时包含数据缓存的结构体
FILE *pf;
volatile unsigned int m = 1, n = 0;

DWORD WINAPI f0(LPVOID lpParam);                           //判断是否继续
DWORD WINAPI f1(LPVOID lpParam);                           //写入文件
char* gettimec();

int main(int argc, TCHAR* argv[])
{
	std::cout << gettimec();
	system("pause");
	int t;
	std::ifstream cfg("LY.cfg");
	std::string ls, fp(""), cp(""), lp(""), cp1, cp2;
	//读取配置文件
	while (std::getline(cfg, ls))
	{
		if (ls.substr(0, 6) == "FPath=")//FilePath
			fp = ls.substr(6);
		if (ls.substr(0, 6) == "CPath=")//CtrlPath
			cp = ls.substr(6);
		if (ls.substr(0, 6) == "LPath=")//LogPath
			lp = ls.substr(6);
		std::cout << lp << fp << cp;
	}
	cfg.close();
	//使路径格式统一
	{
		//fp
		t = fp.find("\\");
		while (t != -1)
		{
			fp.replace(t, 1, "/");
			t = fp.find("\\");
		}
		if (fp[fp.length() - 1] != '/')
			fp += '/';
		//cp
		t = cp.find("\\");
		while (t != -1)
		{
			cp.replace(t, 1, "/");
			t = cp.find("\\");
		}
		if (cp[cp.length() - 1] != '/')
			cp += '/';
		cp1 = cp + '1';
		cp2 = cp + '2';
		cp += '0';
		//lp
		t = lp.find("\\");
		while (t != -1)
		{
			lp.replace(t, 1, "/");
			t = lp.find("\\");
		}
		if (lp[lp.length() - 1] != '/')
			lp += '/';
		lp += "LY.log";
	}
	std::ofstream log(lp, std::ios::ate);
	if (!log.is_open())
	{
		std::ofstream errlog("err.log");
		errlog << "error opening " << lp << std::endl;
		log = std::ofstream("LY.log", std::ios::ate);
	}

	if (exist(cp.data()) && !exist(cp1.data()))
		rename(cp.data(), cp1.data());
	DWORD  threadid;
	HANDLE h0 =               CreateThread(NULL, 0, f0, 0, 0, &threadid);
	HANDLE wait =             CreateEvent(NULL, 0, 0, NULL);;
	DWORD  bufsize =          100 * 1024;                  //100KB缓存
	waveform.wFormatTag =     WAVE_FORMAT_PCM;             //声音格式 PCM
	waveform.nSamplesPerSec = 44100;                       //采样率(Hz)
	waveform.wBitsPerSample = 16;                          //采样比特(bits)
	waveform.nChannels =      1;                           //采样声道数
	waveform.nAvgBytesPerSec =                             //每秒的数据率(byte)
		                      ((waveform.nSamplesPerSec * waveform.wBitsPerSample * waveform.nChannels) >> 3);
	waveform.nBlockAlign =    2;                           //块大小
	waveform.cbSize =         0;
	pBuffer1 =                new BYTE[bufsize];           //分配缓存
	pBuffer2 =                new BYTE[bufsize];
	//开启音频采集
	//获取系统时间用于文件名
	fp += gettimec();
	fp += ".pcm";
	fopen_s(&pf, fp.data(), "wb");
	waveInOpen(&hWaveIn, WAVE_MAPPER, &waveform, (DWORD_PTR)wait, 0L, CALLBACK_EVENT);

	n = 1;
	//循环录音
	while (1)
	{
		if (m == 1)
		{
			if (n == 1)	n = 2;
			else if (n == 2) n = 1;
			wHdr1.lpData = (LPSTR)(n == 1 ? pBuffer1 : pBuffer2);
			wHdr1.dwBufferLength = bufsize;
			wHdr1.dwBytesRecorded = 0;
			wHdr1.dwUser = 0;
			wHdr1.dwFlags = 0;
			wHdr1.dwLoops = 1;
			waveInPrepareHeader(hWaveIn, &wHdr1, sizeof(WAVEHDR));   //准备一个波形数据块头用于录音
			waveInAddBuffer(hWaveIn, &wHdr1, sizeof(WAVEHDR));       //指定波形数据块为录音输入缓存
			waveInStart(hWaveIn);
			Sleep(2000);
			f1((LPVOID)n);
			waveInReset(hWaveIn);
		}
		else if (m == 0)
			Sleep(1000);
		else if (m == 2)
			break;
	}
	waveInClose(hWaveIn);
	fclose(pf);
	rename(cp2.data(), cp.data());
	return 0;
}

DWORD WINAPI f0(LPVOID lpParam)//判断是否继续
{
	while (n == 0)
		Sleep(100);
	while (1)
	{
		if (exist("E:/Temp/1")) {
			m = 1;
		}
		if (exist("E:/Temp/2")) {
			m = 2;
		}
		if (exist("E:/Temp/0")) {
			m = 0;
		}
		Sleep(1000);
	}
	return 0;
}
DWORD WINAPI f1(LPVOID lpParam)//写入文件
{
	fwrite((int)lpParam == 1 ? pBuffer1 : pBuffer2, 1, wHdr1.dwBytesRecorded, pf);
	return 0;
}
char* gettimec()
{
	time_t rt;
	struct tm * timeinfo;
	time(&rt);
	timeinfo = localtime(&rt);
	char ctm[21];
	strftime(ctm, sizeof(tm), "%Y.%m.%d.%H.%M.%S", timeinfo);     //YYYY.MM.DD.HH.MM.SS
	return ctm;
}