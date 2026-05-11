#ifndef REGISTRY_H
#define REGISTRY_H

typedef struct {
	TCHAR szConfigFile[MAX_PATH];
	int nSampleRate;
	int nControlRate;
	int nVoices;
	int nAmp;
	BOOL fAdjustPanning;
	BOOL fMono;
	BOOL f8Bit;
	BOOL fAntialiasing;
	BOOL fPreResample;
	BOOL fFastDecay;
	BOOL fDynamicLoad;
	TCHAR szDefaultInstrument[MAX_PATH];
	int nDefaultProgram;
	int nDrumChannels;
	int nQuietChannels;
} DriverConfig;

#ifdef __cplusplus
extern "C" {
#endif
void ReadRegistry(DriverConfig *cfg);
void WriteRegistry(DriverConfig *cfg);
#ifdef __cplusplus
}
#endif

#endif
