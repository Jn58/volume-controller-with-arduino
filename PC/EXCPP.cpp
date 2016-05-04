#include <stdio.h>
#include <tchar.h>
#include "SerialClass.h"	// Library described above
#include <string>
#include <Endpointvolume.h>
#include <mmdeviceapi.h>
#include <iostream>



// application reads from the specified serial port and reports the collected data

bool ChangeVolume(double nVolume, bool bScalar)
{

	HRESULT hr = NULL;
	bool decibels = false;
	bool scalar = false;
	double newVolume = nVolume;

	CoInitialize(NULL);
	IMMDeviceEnumerator *deviceEnumerator = NULL;
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER,
		__uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);
	IMMDevice *defaultDevice = NULL;

	hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice);
	deviceEnumerator->Release();
	deviceEnumerator = NULL;

	IAudioEndpointVolume *endpointVolume = NULL;
	hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume),
		CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&endpointVolume);
	defaultDevice->Release();
	defaultDevice = NULL;

	// -------------------------
	float currentVolume = 0;
	endpointVolume->GetMasterVolumeLevel(&currentVolume);
	//printf("Current volume in dB is: %f\n", currentVolume);

	hr = endpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
	//CString strCur=L"";
	//strCur.Format(L"%f",currentVolume);
	//AfxMessageBox(strCur);

	// printf("Current volume as a scalar is: %f\n", currentVolume);
	if (bScalar == false)
	{
		hr = endpointVolume->SetMasterVolumeLevel((float)newVolume, NULL);
	}
	else if (bScalar == true)
	{
		hr = endpointVolume->SetMasterVolumeLevelScalar((float)newVolume, NULL);
	}
	endpointVolume->Release();

	CoUninitialize();

	return FALSE;
}
int cur, pre,set;

// application reads from the specified serial port and reports the collected data

int _tmain(int argc, _TCHAR* argv[])
{
	
	printf("Welcome to the volume controllor\n\n");
	Serial* SP;
	while(1)
	{
		char port[15] = "\\\\.\\COM";
		char porttemp[10];
		if (argc == 1)
		{
			printf("Enter your Port : ");
			scanf("%s", porttemp);

		}
		else
		{
			WideCharToMultiByte(CP_ACP, 0, argv[1], -1, porttemp, WideCharToMultiByte(CP_ACP, 0, argv[1], -1, NULL, 0, NULL, NULL), NULL, NULL);
			printf("Your port is %s\n", porttemp);
			argc = 1;
		}
		strcat(port, porttemp);

		SP = new Serial(port);    // adjust as needed

		if (SP->IsConnected())
		{
			printf("We're connected\n\n");
			break;
		}
			
		else
		{
			printf("Try another prot\n");
		}
	}
	

	char incomingData[256] = "";			// don't forget to pre-allocate memory
											//printf("%s\n",incomingData);
	int dataLength = 255;
	int readResult = 0;
	char data[768] = { 0, };
	int data_size = 0;
	char temp[10];
	int i;
	//while (!readResult) readResult = SP->ReadData(incomingData, dataLength);
	//set=pre=cur= std::stoi(incomingData);
	readResult = SP->ReadData(incomingData, dataLength);
	memcpy(data, incomingData, readResult);
	do
	{
		for (i = 0; i < readResult; i++)
			if (data[i] == 'a') break;
		if (i == 0)
			data_size = readResult;
		else if (i != readResult)
		{
			memcpy(data, data + i, readResult - i);
			data_size = readResult - i;
		}
	} while (data[0] != 'a');
	printf("Current value is %4d",cur);
	while (SP->IsConnected())
	{
		readResult = SP->ReadData(incomingData, dataLength);
		if (readResult)
		{
			memcpy(data + data_size, incomingData, readResult);
			data_size += readResult;
		}
		while (1)
		{
			for (i = 1; i < data_size; i++) if (data[i] == 'a') break;
			if (i == data_size) break;
			memcpy(temp, data + 1, i - 1);
			temp[i - 1] = 0;
			
			data_size -= i;
			memcpy(data, data + i, data_size);
			cur = 0;
			for (int j = 0; j < i - 1; j++)
			{
				cur *= 10;
				cur += temp[j]-'0';
			}
			if (abs(cur - pre)>1)
			{
				printf("\b\b\b\b    \b\b\b\b");
				printf("%4d", cur);
				ChangeVolume((double)cur / 1023, true);
			}
			pre = cur;
			
		}
		Sleep(50);
		
		
	}
	return 0;
}