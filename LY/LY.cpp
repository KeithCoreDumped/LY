// LY.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
// LY V2.0

#include "pch.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <string>
#include <Windows.h>
#include <io.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"") //隐藏窗口

HWAVEIN hWaveIn;                                           //输入设备
WAVEFORMATEX waveform;                                     //采集音频的格式，结构体
BYTE *pBuffer1, *pBuffer2;                                 //采集音频时的数据缓存
WAVEHDR wHdr1;                                             //采集音频时包含数据缓存的结构体
FILE *pf;
std::string fp("./"), cp("./"), lp("./"), cp1, cp2;
volatile unsigned int m = 1, n = 0;

DWORD WINAPI f0(LPVOID lpParam);                           //判断是否继续
DWORD WINAPI f1(LPVOID lpParam);                           //写入文件
std::string gettimec();
bool  exist(std::string filename);
bool  exist(char* filename);

int main(int argc, char* argv[])
{
	size_t t;
	std::stringstream ss;
	std::string cfgp = std::string(argv[0]).substr(0, std::string(argv[0]).find_last_of('\\') + 1) + "LY.cfg";
	std::ifstream cfg(cfgp.data());
	std::string ls;
	//读取配置文件
	while (std::getline(cfg, ls))
	{
		if (ls.substr(0, 6) == "FPath=")//FilePath
			fp = ls.substr(6);
		if (ls.substr(0, 6) == "CPath=")//CtrlPath
			cp = ls.substr(6);
		if (ls.substr(0, 6) == "LPath=")//LogPath
			lp = ls.substr(6);
	}
	cfg.close();
	//使路径格式统一
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

	std::ofstream flog(lp, std::ios::app);
	if (!flog.is_open())
	{
		std::ofstream errflog("err.log");
		errflog << "error opening " << lp << std::endl;
		errflog.close();
		flog = std::ofstream("LY.log", std::ios::app);
		
	}
	flog << gettimec() << "\t\t\t\tCtrlPath:" << cp << "\tLogPath:" << lp << "\tFilePath:" << fp << std::endl;
	if (exist(cp) && !exist(cp1))
		rename(cp.data(), cp1.data());
	DWORD  threadid;
	HANDLE h0 =               CreateThread(NULL, 0, f0, 0, 0, &threadid);
	HANDLE wait =             CreateEvent(NULL, 0, 0, NULL);
	DWORD  bufsize =          200 * 1024;                  //100KB缓存
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
	//获取系统时间用于文件名
	fp += gettimec();
	t = 0;
	while (exist(fp + ".pcm"))
	{
		ss.clear();
		t += 1;
		ss << t;
		fp += " (" + ss.str() + ")";
	}
	fp += ".pcm";
	flog << gettimec() << "\t\t\t\tCtrlPath:" << cp << "\tLogPath:" << lp << "\tFilePath:" << fp << std::endl;
	fopen_s(&pf, fp.data(), "wb");
	//开启音频采集
	waveInOpen(&hWaveIn, WAVE_MAPPER, &waveform, (DWORD_PTR)wait, 0L, CALLBACK_EVENT);
	n = 2;
	__int64 t0 = time(NULL);
	flog << gettimec() << "\tstarted.\ttime=" << t0 << std::endl;
	//循环录音
	while (m == 1)
	{
		if (n == 1)	n = 2;
		else if (n == 2) n = 1;
		wHdr1.lpData  = (LPSTR)(n == 1 ? pBuffer1 : pBuffer2);
		wHdr1.dwBufferLength  = bufsize;
		wHdr1.dwBytesRecorded = 0;
		wHdr1.dwUser  = 0;
		wHdr1.dwFlags = 0;
		wHdr1.dwLoops = 1;
		waveInPrepareHeader (hWaveIn, &wHdr1, sizeof(WAVEHDR));       //波形数据块头
		waveInAddBuffer     (hWaveIn, &wHdr1, sizeof(WAVEHDR));           //波形数据块输入缓存
		waveInStart         (hWaveIn);
		Sleep(1000);
		//f1((LPVOID)wHdr1.dwBytesRecorded);
		CreateThread(NULL, 0, f1,(LPVOID)wHdr1.dwBytesRecorded , 0, &threadid);
		waveInReset(hWaveIn);
	}

	Sleep(1000);
	n = 0;
	waveInClose(hWaveIn);
	fclose(pf);
	__int64 t1 = time(NULL);
	flog << gettimec() << "\tfinished.\ttime=" << t1 << "\t\t\t\t" << t1 - t0 << " s in total\n" << "===========\
========================================================================================" << std::endl;

	flog.close();
	rename(cp2.data(), cp.data());
	return 0;
}

DWORD WINAPI f0(LPVOID lpParam)//判断是否继续
{
	while (1)
	{
		if (exist(cp1))
			m = 1;
		else if (exist(cp))
			m = 0;
		else if (exist(cp2))
			m = 2;
		Sleep(1000);
	}
	return 0;
}

DWORD WINAPI f1(LPVOID lpParam)//写入文件
{
	fwrite((int)n == 1 ? pBuffer1 : pBuffer2, 1, (DWORD)lpParam, pf);
	return 0;
}

std::string gettimec()
{
	time_t rt;
	struct tm * timeinfo;
	time(&rt);
	timeinfo = localtime(&rt);
	char ctm[21];
	strftime(ctm, sizeof(tm), "%Y.%m.%d.%H.%M.%S", timeinfo);     //YYYY.MM.DD.HH.MM.SS
	return std::string(ctm);
}

bool exist(std::string filename)
{
	return (!_access(filename.data(), 0));
}

bool exist(char* filename)
{
	return (!_access(filename, 0));
}
