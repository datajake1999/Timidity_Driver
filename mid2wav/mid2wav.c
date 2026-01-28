#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include "../common/registry.h"
#include "../driver/timidity/timid.h"
#include "wav_writer.h"

int main(int argc, char *argv[])
{
	Timid *synth = NULL;
	uint8 *buffer = NULL;
	WavWriter *wr = NULL;
	DriverConfig *cfg = NULL;
	char text[256];
	char szAnsi[MAX_PATH];
	int channels, bit_depth, audio_format, i, return_value;
	clock_t begin, end;
	double time_spent, realtime_rate;
	if (argc < 3)
	{
		printf("Usage: %s [input].mid [output].wav.\n", argv[0]);
		return_value = 1;
		goto cleanup;
	}
	memset(text, 0, sizeof(text));
	memset(szAnsi, 0, sizeof(szAnsi));
	cfg = (DriverConfig *)malloc(sizeof(DriverConfig));
	if (!cfg)
	{
		printf("Error allocating settings structure.\n");
		return_value = 1;
		goto cleanup;
	}
	memset(cfg, 0, sizeof(DriverConfig));
	cfg->nSampleRate = DEFAULT_RATE;
	cfg->nControlRate = CONTROLS_PER_SECOND;
	cfg->nVoices = DEFAULT_VOICES;
	cfg->nAmp = DEFAULT_AMPLIFICATION;
	cfg->fAdjustPanning = TRUE;
	cfg->fMono = FALSE;
	cfg->f8Bit = FALSE;
	cfg->fAntialiasing = TRUE;
	cfg->fPreResample = TRUE;
	cfg->fFastDecay = TRUE;
	cfg->fDynamicLoad = FALSE;
	cfg->nDefaultProgram = DEFAULT_PROGRAM;
	cfg->nDrumChannels = DEFAULT_DRUMCHANNELS;
	cfg->nQuietChannels = 0;
	ReadRegistry(cfg);
	if (cfg->nSampleRate > MAX_OUTPUT_RATE)
	{
		cfg->nSampleRate = MAX_OUTPUT_RATE;
	}
	else if (cfg->nSampleRate < MIN_OUTPUT_RATE)
	{
		cfg->nSampleRate = MIN_OUTPUT_RATE;
	}
	if (cfg->fMono)
	{
		channels = 1;
	}
	else
	{
		channels = 2;
	}
	if (cfg->f8Bit)
	{
		bit_depth = 8;
		audio_format = AU_CHAR;
	}
	else
	{
		bit_depth = 16;
		audio_format = AU_SHORT;
	}
	synth = (Timid *)malloc(sizeof(Timid));
	if (!synth)
	{
		printf("Error allocating synth.\n");
		return_value = 1;
		goto cleanup;
	}
	memset(synth, 0, sizeof(Timid));
	timid_init(synth);
	timid_set_sample_rate(synth, cfg->nSampleRate);
	timid_set_control_rate(synth, cfg->nControlRate);
	timid_set_max_voices(synth, cfg->nVoices);
	timid_set_amplification(synth, cfg->nAmp);
	timid_set_immediate_panning(synth, cfg->fAdjustPanning);
	timid_set_mono(synth, cfg->fMono);
	timid_set_antialiasing(synth, cfg->fAntialiasing);
	timid_set_pre_resample(synth, cfg->fPreResample);
	timid_set_fast_decay(synth, cfg->fFastDecay);
	timid_set_dynamic_instrument_load(synth, cfg->fDynamicLoad);
	timid_set_default_program(synth, cfg->nDefaultProgram);
	for (i = 0; i < 16; i++)
	{
		if (cfg->nDrumChannels & (1<<i))
		{
			timid_set_drum_channel(synth, i, 1);
		}
		else
		{
			timid_set_drum_channel(synth, i, 0);
		}
		if (cfg->nQuietChannels & (1<<i))
		{
			timid_set_quiet_channel(synth, i, 1);
		}
		else
		{
			timid_set_quiet_channel(synth, i, 0);
		}
	}
#ifdef _UNICODE
	WideCharToMultiByte(CP_ACP, 0, cfg->szConfigFile, -1, szAnsi, MAX_PATH, NULL, NULL);
#else
	strcpy(szAnsi, cfg->szConfigFile);
#endif
	if (!strlen(szAnsi))
	{
		strcpy(szAnsi, CONFIG_FILE);
	}
	if (!timid_load_config(synth, szAnsi))
	{
		printf("Failed to load Timidity configuration file at %s.\n", szAnsi);
		return_value = 1;
		goto cleanup;
	}
	if (!timid_load_smf(synth, argv[1]))
	{
		printf("Failed to play %s.\n", argv[1]);
		return_value = 1;
		goto cleanup;
	}
	wr = (WavWriter *)malloc(sizeof(WavWriter));
	if (!wr)
	{
		printf("Error allocating WAV writer.\n");
		return_value = 1;
		goto cleanup;
	}
	memset(wr, 0, sizeof(WavWriter));
	if (!WavFileOpen(wr, argv[2], cfg->nSampleRate, bit_depth, channels, 1))
	{
		printf("Failed to open %s.\n", argv[2]);
		return_value = 1;
		goto cleanup;
	}
	buffer = (uint8 *)malloc(AUDIO_BUFFER_SIZE * channels * (bit_depth / 8));
	if (!buffer)
	{
		printf("Error allocating buffer.\n");
		return_value = 1;
		goto cleanup;
	}
	memset(buffer, 0, AUDIO_BUFFER_SIZE * channels * (bit_depth / 8));
	if (timid_get_smf_name(synth, text, sizeof(text)))
	{
		printf("Playing %s.\n", text);
	}
	if (timid_get_song_title(synth, text, sizeof(text)))
	{
		printf("Title: %s.\n", text);
	}
	if (timid_get_song_copyright(synth, text, sizeof(text)))
	{
		printf("Copyright: %s.\n", text);
	}
	printf("Number of events: %d.\n", timid_get_event_count(synth));
	printf("MIDI duration: %d MS (%d samples).\n", timid_get_duration(synth), timid_get_sample_count(synth));
	printf("MIDI bitrate: %d KBPS.\n", timid_get_bitrate(synth));
	begin = clock();
	while (timid_play_smf(synth, audio_format, buffer, AUDIO_BUFFER_SIZE))
	{
		if (!WavFileWrite(wr, buffer, AUDIO_BUFFER_SIZE))
		{
			printf("Error writing data to %s.\n", argv[2]);
			return_value = 1;
			goto cleanup;
		}
	}
	end = clock();
	WavFileClose(wr);
	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	realtime_rate = ((double)timid_get_current_time(synth) / 1000.0) / time_spent;
	printf("Playback finished.\n");
	printf("Playback duration: %d MS (%d samples).\n", timid_get_current_time(synth), timid_get_current_sample_position(synth));
	printf("Estimated and playback duration difference: %d MS (%d samples).\n", timid_get_current_time(synth)-timid_get_duration(synth), timid_get_current_sample_position(synth)-timid_get_sample_count(synth));
	printf("Lost notes: %d.\n", timid_get_lost_notes(synth));
	printf("Cut notes: %d.\n", timid_get_cut_notes(synth));
	printf("Conversion time: %f seconds (%f realtime).\n", time_spent, realtime_rate);
	return_value = 0;
cleanup:
	if (buffer)
	{
		free(buffer);
		buffer = NULL;
	}
	if (wr)
	{
		free(wr);
		wr = NULL;
	}
	if (synth)
	{
		timid_close(synth);
		free(synth);
		synth = NULL;
	}
	if (cfg)
	{
		free(cfg);
		cfg = NULL;
	}
	return return_value;
}
