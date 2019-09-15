// LY.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
// LY V2.0

// TODO:
// CFG
// LOG

#include "pch.h"
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <string>
#include <Windows.h>
#include <io.h>

#pragma comment(lib, "winmm.lib")
#pragma   comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

#define TRACE printf
#define exist(x) !_access(x, 0)
HWAVEIN hWaveIn;  //输入设备
WAVEFORMATEX waveform; //采集音频的格式，结构体
BYTE *pBuffer1, *pBuffer2;//采集音频时的数据缓存
WAVEHDR wHdr1; //采集音频时包含数据缓存的结构体
FILE *pf;
volatile unsigned int m = 1, n = 0;

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
int main(int argc, TCHAR* argv[])
{
	if (exist("E:/Temp/0") && !exist("E:/Temp/1"))
		rename("E:/Temp/0", "E:/Temp/1");
	DWORD threadid;
	HANDLE h0 = CreateThread(NULL, 0, f0, 0, 0, &threadid);
	//HANDLE h1 = CreateThread(NULL, 0, threadfunc1, 0, 0, &threadid);
	HANDLE wait;
	waveform.wFormatTag = WAVE_FORMAT_PCM;//声音格式为PCM
	waveform.nSamplesPerSec = 44100;//采样率，44100次/秒
	waveform.wBitsPerSample = 16;//采样比特，16bits/次
	waveform.nChannels = 1;//采样声道数，2声道
	waveform.nAvgBytesPerSec = ((waveform.nSamplesPerSec*waveform.wBitsPerSample*waveform.nChannels) >> 3);//每秒的数据率，就是每秒能采集多少字节的数据
	waveform.nBlockAlign = 2;//一个块的大小，采样bit的字节数乘以声道数
	waveform.cbSize = 0;//一般为0

	wait = CreateEvent(NULL, 0, 0, NULL);

	waveInOpen(&hWaveIn, WAVE_MAPPER, &waveform, (DWORD_PTR)wait, 0L, CALLBACK_EVENT);//使用waveInOpen函数开启音频采集

	DWORD bufsize = 100 * 1024;//每次开辟100k的缓存存储录音数据
	pBuffer1 = new BYTE[bufsize];
	pBuffer2 = new BYTE[bufsize];
	{
		time_t rt;
		struct tm * timeinfo;
		time(&rt);
		timeinfo = localtime(&rt);
		char tm[35];
		strftime(tm, sizeof(tm), "E:/Temp/%Y.%m.%d.%H.%M.%S.pcm", timeinfo);
		fopen_s(&pf, tm, "wb");
	}
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
			waveInPrepareHeader(hWaveIn, &wHdr1, sizeof(WAVEHDR));//准备一个波形数据块头用于录音
			waveInAddBuffer(hWaveIn, &wHdr1, sizeof(WAVEHDR));//指定波形数据块为录音输入缓存
			waveInStart(hWaveIn);
			Sleep(1000);
			//CreateThread(NULL, 0, threadfunc1, (LPVOID)n, 0, &threadid);
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
	rename("E:/Temp/2", "E:/Temp/0");
	return 0;
}