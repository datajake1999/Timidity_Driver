/* Copyright (C) 2011, 2012 Sergey V. Mikayev
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.h"

namespace Timidity {

static MidiSynth &midiSynth = MidiSynth::getInstance();

static class MidiStream {
private:
	static const unsigned int maxPos = 1024;
	unsigned int startpos;
	unsigned int endpos;
	DWORD stream[maxPos][2];

public:
	MidiStream() {
		startpos = 0;
		endpos = 0;
	}

	DWORD PutMessage(DWORD msg, DWORD timestamp) {
		unsigned int newEndpos = endpos;

		newEndpos++;
		if (newEndpos == maxPos) // check for buffer rolloff
			newEndpos = 0;
		if (startpos == newEndpos) // check for buffer full
			return -1;
		stream[endpos][0] = msg;	// ok to put data and update endpos
		stream[endpos][1] = timestamp;
		endpos = newEndpos;
		return 0;
	}

	DWORD GetMessage() {
		if (startpos == endpos) // check for buffer empty
			return -1;
		DWORD msg = stream[startpos][0];
		startpos++;
		if (startpos == maxPos) // check for buffer rolloff
			startpos = 0;
		return msg;
	}

	DWORD PeekMessageTime() {
		if (startpos == endpos) // check for buffer empty
			return (DWORD)-1;
		return stream[startpos][1];
	}

	DWORD PeekMessageTimeAt(unsigned int pos) {
		if (startpos == endpos) // check for buffer empty
			return -1;
		unsigned int peekPos = (startpos + pos) % maxPos;
		return stream[peekPos][1];
	}

	void Clean() {
		startpos = 0;
		endpos = 0;
		memset(stream, 0, sizeof(stream));
	}

} midiStream;

static class SynthEventWin32 {
private:
	HANDLE hEvent;

public:
	int Init() {
		hEvent = CreateEvent(NULL, false, true, NULL);
		if (hEvent == NULL) {
			MessageBoxW(NULL, L"Can't create sync object", L"Timidity", MB_OK | MB_ICONEXCLAMATION);
			return 1;
		}
		return 0;
	}

	void Close() {
		CloseHandle(hEvent);
	}

	void Wait() {
		WaitForSingleObject(hEvent, INFINITE);
	}

	void Release() {
		SetEvent(hEvent);
	}
} synthEvent;

static class WaveOutWin32 {
private:
	HWAVEOUT	hWaveOut;
	WAVEHDR		*WaveHdr;
	HANDLE		hEvent;
	DWORD		chunks;
	DWORD		numBytes;
	DWORD		prevPlayPos;
	DWORD		getPosWraps;
	bool		stopProcessing;

public:
	int Init(Bit8u *buffer, unsigned int bufferSize, unsigned int chunkSize, bool useRingBuffer, unsigned int sampleRate, unsigned int numChannels, unsigned int bitDepth) {
		DWORD callbackType = CALLBACK_NULL;
		DWORD_PTR callback = (DWORD_PTR)NULL;
		hEvent = NULL;
		if (!useRingBuffer) {
			hEvent = CreateEvent(NULL, false, true, NULL);
			callback = (DWORD_PTR)hEvent;
			callbackType = CALLBACK_EVENT;
		}

		if (numChannels > 2)
		{
			numChannels = 2;
		}
		else if (numChannels < 1)
		{
			numChannels = 1;
		}
		numBytes = (bitDepth / 8) * numChannels;

		PCMWAVEFORMAT wFormat = {WAVE_FORMAT_PCM, (WORD)numChannels, sampleRate, sampleRate * numBytes, (WORD)numBytes, bitDepth};

		// Open waveout device
		int wResult = waveOutOpen(&hWaveOut, WAVE_MAPPER, (LPWAVEFORMATEX)&wFormat, callback, (DWORD_PTR)&midiSynth, callbackType);
		if (wResult != MMSYSERR_NOERROR) {
			MessageBoxW(NULL, L"Failed to open waveform output device", L"Timidity", MB_OK | MB_ICONEXCLAMATION);
			return 2;
		}

		// Prepare headers
		chunks = useRingBuffer ? 1 : bufferSize / chunkSize;
		WaveHdr = new WAVEHDR[chunks];
		LPSTR chunkStart = (LPSTR)buffer;
		DWORD chunkBytes = numBytes * chunkSize;
		for (UINT i = 0; i < chunks; i++) {
			if (useRingBuffer) {
				WaveHdr[i].dwBufferLength = numBytes * bufferSize;
				WaveHdr[i].lpData = chunkStart;
				WaveHdr[i].dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
				WaveHdr[i].dwLoops = -1L;
			} else {
				WaveHdr[i].dwBufferLength = chunkBytes;
				WaveHdr[i].lpData = chunkStart;
				WaveHdr[i].dwFlags = 0L;
				WaveHdr[i].dwLoops = 0L;
				chunkStart += chunkBytes;
			}
			wResult = waveOutPrepareHeader(hWaveOut, &WaveHdr[i], sizeof(WAVEHDR));
			if (wResult != MMSYSERR_NOERROR) {
				MessageBoxW(NULL, L"Failed to Prepare Header", L"Timidity", MB_OK | MB_ICONEXCLAMATION);
				return 3;
			}
		}
		stopProcessing = false;
		return 0;
	}

	int Close() {
		stopProcessing = true;
		SetEvent(hEvent);
		int wResult = waveOutReset(hWaveOut);
		if (wResult != MMSYSERR_NOERROR) {
			MessageBoxW(NULL, L"Failed to Reset WaveOut", L"Timidity", MB_OK | MB_ICONEXCLAMATION);
			return 8;
		}

		for (UINT i = 0; i < chunks; i++) {
			wResult = waveOutUnprepareHeader(hWaveOut, &WaveHdr[i], sizeof(WAVEHDR));
			if (wResult != MMSYSERR_NOERROR) {
				MessageBoxW(NULL, L"Failed to Unprepare Wave Header", L"Timidity", MB_OK | MB_ICONEXCLAMATION);
				return 8;
			}
		}
		delete[] WaveHdr;
		WaveHdr = NULL;

		wResult = waveOutClose(hWaveOut);
		if (wResult != MMSYSERR_NOERROR) {
			MessageBoxW(NULL, L"Failed to Close WaveOut", L"Timidity", MB_OK | MB_ICONEXCLAMATION);
			return 8;
		}
		if (hEvent != NULL) {
			CloseHandle(hEvent);
			hEvent = NULL;
		}
		return 0;
	}

	int Start() {
		getPosWraps = 0;
		prevPlayPos = 0;
		for (UINT i = 0; i < chunks; i++) {
			if (waveOutWrite(hWaveOut, &WaveHdr[i], sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
				MessageBoxW(NULL, L"Failed to write block to device", L"Timidity", MB_OK | MB_ICONEXCLAMATION);
				return 4;
			}
		}
		_beginthread(RenderingThread, 16384, this);
		return 0;
	}

	int Pause() {
		if (waveOutPause(hWaveOut) != MMSYSERR_NOERROR) {
			MessageBoxW(NULL, L"Failed to Pause wave playback", L"Timidity", MB_OK | MB_ICONEXCLAMATION);
			return 9;
		}
		return 0;
	}

	int Resume() {
		if (waveOutRestart(hWaveOut) != MMSYSERR_NOERROR) {
			MessageBoxW(NULL, L"Failed to Resume wave playback", L"Timidity", MB_OK | MB_ICONEXCLAMATION);
			return 9;
		}
		return 0;
	}

	UINT64 GetPos() {
		MMTIME mmTime;
		mmTime.wType = TIME_SAMPLES;

		if (waveOutGetPosition(hWaveOut, &mmTime, sizeof(MMTIME)) != MMSYSERR_NOERROR) {
			MessageBoxW(NULL, L"Failed to get current playback position", L"Timidity", MB_OK | MB_ICONEXCLAMATION);
			return 10;
		}
		if (mmTime.wType != TIME_SAMPLES) {
			MessageBoxW(NULL, L"Failed to get # of samples played", L"Timidity", MB_OK | MB_ICONEXCLAMATION);
			return 10;
		}

		// Deal with waveOutGetPosition() wraparound. For 16-bit stereo output, it equals 2^27,
		// presumably caused by the internal 32-bit counter of bits played.
		// The output of that nasty waveOutGetPosition() isn't monotonically increasing
		// even during 2^27 samples playback, so we have to ensure the difference is big enough...
		int delta = mmTime.u.sample - prevPlayPos;
		if (delta < -(1 << 26)) {
			++getPosWraps;
		}
		prevPlayPos = mmTime.u.sample;
		return mmTime.u.sample + getPosWraps * (1 << 27);
	}

	static void RenderingThread(void *);
} s_waveOut;

void WaveOutWin32::RenderingThread(void *) {
	if (s_waveOut.chunks == 1) {
		// Rendering using single looped ring buffer
		while (!s_waveOut.stopProcessing) {
			midiSynth.RenderAvailableSpace();
		}
	} else {
		while (!s_waveOut.stopProcessing) {
			bool allBuffersRendered = true;
			for (UINT i = 0; i < s_waveOut.chunks; i++) {
				if (s_waveOut.WaveHdr[i].dwFlags & WHDR_DONE) {
					allBuffersRendered = false;
					midiSynth.Render((Bit8u *)s_waveOut.WaveHdr[i].lpData, s_waveOut.WaveHdr[i].dwBufferLength / s_waveOut.numBytes);
					if (waveOutWrite(s_waveOut.hWaveOut, &s_waveOut.WaveHdr[i], sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
						MessageBoxW(NULL, L"Failed to write block to device", L"Timidity", MB_OK | MB_ICONEXCLAMATION);
					}
				}
			}
			if (allBuffersRendered) {
				WaitForSingleObject(s_waveOut.hEvent, INFINITE);
			}
		}
	}
}

MidiSynth::MidiSynth() {}

MidiSynth &MidiSynth::getInstance() {
	static MidiSynth *instance = new MidiSynth;
	return *instance;
}

// Renders all the available space in the single looped ring buffer
void MidiSynth::RenderAvailableSpace() {
	DWORD playPos = s_waveOut.GetPos() % bufferSize;
	DWORD framesToRender;

	if (playPos < framesRendered) {
		// Buffer wrap, render 'till the end of the buffer
		framesToRender = bufferSize - framesRendered;
	} else {
		framesToRender = playPos - framesRendered;
		if (framesToRender < chunkSize) {
			Sleep(1 + (chunkSize - framesToRender) * 1000 / sampleRate);
			return;
		}
	}
	midiSynth.Render(buffer + numChannels * (bitDepth / 8) * framesRendered, framesToRender);
}

// Renders totalFrames frames starting from bufpos
// The number of frames rendered is added to the global counter framesRendered
void MidiSynth::Render(Bit8u *bufpos, DWORD totalFrames) {
	while (totalFrames > 0) {
		DWORD timeStamp;
		// Incoming MIDI messages timestamped with the current audio playback position + midiLatency
		while ((timeStamp = midiStream.PeekMessageTime()) == framesRendered) {
			DWORD msg = midiStream.GetMessage();
			synthEvent.Wait();
			timid_write_midi_packed(&synth, msg);
			synthEvent.Release();
		}

		// Find out how many frames to render. The value of timeStamp == -1 indicates the MIDI buffer is empty
		DWORD framesToRender = timeStamp - framesRendered;
		if (framesToRender > totalFrames) {
			// MIDI message is too far - render the rest of frames
			framesToRender = totalFrames;
		}
		synthEvent.Wait();
		if (bitDepth == 16)
		{
			timid_render_short(&synth, (Bit16s *)bufpos, framesToRender);
		}
		else if (bitDepth == 8)
		{
			timid_render_char(&synth, (Bit8u *)bufpos, framesToRender);
		}
		synthEvent.Release();
		framesRendered += framesToRender;
		bufpos += numChannels * (bitDepth / 8) * framesToRender; // each frame consists of two samples for both the Left and Right channels
		totalFrames -= framesToRender;
	}

	// Wrap framesRendered counter
	if (framesRendered >= bufferSize) {
		framesRendered -= bufferSize;
	}
}

unsigned int MidiSynth::MillisToFrames(unsigned int millis) {
	return UINT(sampleRate * millis / 1000.f);
}

void MidiSynth::LoadSettings() {
	sampleRate = cfg.nSampleRate;
	if (sampleRate > MAX_OUTPUT_RATE)
	{
		sampleRate = MAX_OUTPUT_RATE;
	}
	else if (sampleRate < MIN_OUTPUT_RATE)
	{
		sampleRate = MIN_OUTPUT_RATE;
	}
	if (cfg.fMono)
	{
		numChannels = 1;
	}
	else
	{
		numChannels = 2;
	}
	if (cfg.f8Bit)
	{
		bitDepth = 8;
	}
	else
	{
		bitDepth = 16;
	}
	bufferSize = MillisToFrames(100);
	chunkSize = MillisToFrames(10);
	midiLatency = MillisToFrames(0);
	useRingBuffer = false;
	if (!useRingBuffer) {
		// Number of chunks should be ceil(bufferSize / chunkSize)
		DWORD chunks = (bufferSize + chunkSize - 1) / chunkSize;
		// Refine bufferSize as chunkSize * number of chunks, no less then the specified value
		bufferSize = chunks * chunkSize;
	}
}

int MidiSynth::Init() {
	char szAnsi[MAX_PATH];
	memset(szAnsi, 0, sizeof(szAnsi));
	memset(&cfg, 0, sizeof(cfg));
	cfg.nSampleRate = DEFAULT_RATE;
	cfg.nControlRate = CONTROLS_PER_SECOND;
	cfg.nVoices = DEFAULT_VOICES;
	cfg.nAmp = DEFAULT_AMPLIFICATION;
	cfg.fAdjustPanning = TRUE;
	cfg.fMono = FALSE;
	cfg.f8Bit = FALSE;
	cfg.fAntialiasing = TRUE;
	cfg.fPreResample = TRUE;
	cfg.fFastDecay = TRUE;
	cfg.fDynamicLoad = FALSE;
	ReadRegistry(&cfg);
	LoadSettings();
	buffer = new Bit8u[numChannels * (bitDepth / 8) * bufferSize]; // each frame consists of two samples for both the Left and Right channels

	// Init synth
	if (synthEvent.Init()) {
		return 1;
	}
	memset(&synth, 0, sizeof(synth));
	timid_init(&synth);
	timid_set_sample_rate(&synth, cfg.nSampleRate);
	timid_set_control_rate(&synth, cfg.nControlRate);
	timid_set_max_voices(&synth, cfg.nVoices);
	timid_set_amplification(&synth, cfg.nAmp);
	timid_set_immediate_panning(&synth, cfg.fAdjustPanning);
	timid_set_mono(&synth, cfg.fMono);
	timid_set_antialiasing(&synth, cfg.fAntialiasing);
	timid_set_pre_resample(&synth, cfg.fPreResample);
	timid_set_fast_decay(&synth, cfg.fFastDecay);
	timid_set_dynamic_instrument_load(&synth, cfg.fDynamicLoad);
#ifdef _UNICODE
	WideCharToMultiByte(CP_ACP, 0, cfg.szConfigFile, -1, szAnsi, MAX_PATH, NULL, NULL);
#else
	strcpy(szAnsi, cfg.szConfigFile);
#endif
	if (!timid_load_config(&synth, szAnsi)) {
		MessageBoxW(NULL, L"Can't open Synth", L"Timidity", MB_OK | MB_ICONEXCLAMATION);
		return 1;
	}

	UINT wResult = s_waveOut.Init(buffer, bufferSize, chunkSize, useRingBuffer, sampleRate, numChannels, bitDepth);
	if (wResult) return wResult;

	// Start playing stream
	if (bitDepth == 16)
	{
		timid_render_short(&synth, (Bit16s *)buffer, bufferSize);
	}
	else if (bitDepth == 8)
	{
		timid_render_char(&synth, (Bit8u *)buffer, bufferSize);
	}
	framesRendered = 0;

	wResult = s_waveOut.Start();
	return wResult;
}

void MidiSynth::ResetSynth()
{
	synthEvent.Wait();
	timid_reset(&synth);
	midiStream.Clean();
	synthEvent.Release();
}

void MidiSynth::PanicSynth()
{
	synthEvent.Wait();
	timid_panic(&synth);
	synthEvent.Release();
}

void MidiSynth::PushMIDI(DWORD msg) {
	midiStream.PutMessage(msg, (s_waveOut.GetPos() + midiLatency) % bufferSize);
}

void MidiSynth::PlaySysex(Bit8u *bufpos, DWORD len) {
	synthEvent.Wait();
	timid_write_sysex(&synth, bufpos, len);
	synthEvent.Release();
}

void MidiSynth::Close() {
	s_waveOut.Pause();
	s_waveOut.Close();
	synthEvent.Wait();
	timid_close(&synth);

	// Cleanup memory
	delete buffer;

	synthEvent.Close();
}

}
