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
	BOOL fAntialiasing;
	BOOL fPreResample;
	BOOL fFastDecay;
	BOOL fDynamicLoad;
} DriverConfig;

#define REG_KEY					_T("Software\\Datajake\\TimidityDriver")
#define REG_NAME_CONFIGFILE		_T("ConfigFile")
#define REG_NAME_SAMPLERATE		_T("SampleRate")
#define REG_NAME_CTRLRATE		_T("ControlRate")
#define REG_NAME_VOICES			_T("Voices")
#define REG_NAME_AMP			_T("Amp")
#define REG_NAME_ADJUSTPAN		_T("AdjustPanning")
#define REG_NAME_MONO			_T("Mono")
#define REG_NAME_ANTIALIAS		_T("Antialiasing")
#define REG_NAME_PRERESAMPLE		_T("PreResample")
#define REG_NAME_FASTDEC		_T("FastDecay")
#define REG_NAME_DYNALOAD		_T("DynamicLoad")

#ifdef __cplusplus
extern "C" {
#endif
void ReadRegistry(DriverConfig *cfg);
void WriteRegistry(DriverConfig *cfg);
#ifdef __cplusplus
}
#endif

#endif
