#include <stdio.h>
#include <windows.h>
#include <al.h>
#include <alc.h>
#include <iostream>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <typeinfo>




// www.codeproject.com/Tips/233484/Change-Master-Volume-in-Visual-Cplusplus
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




// www.dreamincode.net/forums/topic/184668-getting-microphone-input/
int main()
{

	ChangeVolume(0.2, 70);

	ALCdevice *dev[2];
	ALCcontext *ctx;
	ALuint source, buffers[3];
	char data[5000];
	ALuint buf;
	ALint val;

	dev[0] = alcOpenDevice(NULL);
	ctx = alcCreateContext(dev[0], NULL);
	alcMakeContextCurrent(ctx);

	alGenSources(1, &source);
	alGenBuffers(3, buffers);

	/* Setup some initial silent data to play out of the source */
	alBufferData(buffers[0], AL_FORMAT_MONO16, data, sizeof(data), 22050);
	alBufferData(buffers[1], AL_FORMAT_MONO16, data, sizeof(data), 22050);
	alBufferData(buffers[2], AL_FORMAT_MONO16, data, sizeof(data), 22050);
	alSourceQueueBuffers(source, 3, buffers);

	/* If you don't need 3D spatialization, this should help processing time */
	alDistanceModel(AL_NONE);

	dev[1] = alcCaptureOpenDevice(NULL, 22050, AL_FORMAT_MONO16, sizeof(data) / 2);

	/* Start playback and capture, and enter the audio loop */
	alSourcePlay(source);
	alcCaptureStart(dev[1]);

	while (1)
	{
		/* Check if any queued buffers are finished */
		alGetSourcei(source, AL_BUFFERS_PROCESSED, &val);
		if (val <= 0)
			continue;

		std::cout << "Val: " << val << std::endl;
		/* Check how much audio data has been captured (note that 'val' is the
		* number of frames, not bytes) */
		alcGetIntegerv(dev[1], ALC_CAPTURE_SAMPLES, 1, &val);

		/* Read the captured audio */
		alcCaptureSamples(dev[1], data, val);
		std::cout << "Datasize: " << int(data[300]) << std::endl;

		

		
		/* Pop the oldest finished buffer, fill it with the new capture data,
		then re-queue it to play on the source */
		alSourceUnqueueBuffers(source, 1, &buf);
		alBufferData(buf, AL_FORMAT_MONO16, data, val * 2 /* bytes here, not
														  frames */, 22050);
		alSourceQueueBuffers(source, 1, &buf);




		/* Make sure the source is still playing */
		alGetSourcei(source, AL_SOURCE_STATE, &val);
		if (val != AL_PLAYING)
		{
			alSourcePlay(source+20);
		}
	}

	/* Shutdown and cleanup */
	alcCaptureStop(dev[1]);
	alcCaptureCloseDevice(dev[1]);

	alSourceStop(source);
	alDeleteSources(1, &source);
	alDeleteBuffers(3, buffers);

	alcMakeContextCurrent(NULL);
	alcDestroyContext(ctx);
	alcCloseDevice(dev[0]);

	return 0;
}