#ifdef _AUDIO3D_
   #define _SOUND_H_
#else
   #define _SOUND_H_ extern
#endif

#define MAX_CHANNEL          26 /// 22 //16

typedef struct tagCHANNEL {   
   int status;     // 0 means that channel is free
   int iLength;    // filled when appropriate voice is assigned to the channel 
   int iPosition;  // position of next portion to read
   short int* lpData;
   int x, y, z;    // coords of the voice
   int volume;   // for aligning
} CHANNEL;

typedef struct tagAMBIENT {
   int iLength;
   short int* lpData;      // voice this channel is referenced to   
   int iPosition;  // position of next portion to read   
   int volume, avolume;
} AMBIENT;

typedef struct tagMAMBIENT {
   int iLength;
   short int* lpData;      // voice this channel is referenced to   
   int iPosition;  // position of next portion to read         
   float x,y,z;    
} MAMBIENT;


//_SOUND_H_ SOUNDDEVICEDESC* sdd;
//_SOUND_H_ int iTotalSoundDevices;
//_SOUND_H_ int iTotal16SD;
//_SOUND_H_ int iCurrentDriver;
//_SOUND_H_ int iBufferLength;
_SOUND_H_ int iSoundActive; // prevent playing voices on not ready device
//_SOUND_H_ int iNeedNextWritePosition; // is set to 1 when first voice is added
//_SOUND_H_ DWORD dwPlayPos;
//_SOUND_H_ DWORD dwWritePos;

//_SOUND_H_ char* lpSoundBuffer;

//_SOUND_H_ HANDLE hSoundHeap;
//_SOUND_H_ LPDIRECTSOUND lpDS;
//_SOUND_H_ WAVEFORMATEX WaveFormat;
//_SOUND_H_ LPDIRECTSOUNDBUFFER lpdsPrimary, lpdsSecondary, lpdsWork;
//_SOUND_H_ BOOL PrimaryMode;

_SOUND_H_ CHANNEL channel[ MAX_CHANNEL ];
_SOUND_H_ AMBIENT ambient, ambient2;
_SOUND_H_ MAMBIENT mambient;

_SOUND_H_ int xCamera, yCamera, zCamera;
_SOUND_H_ float alphaCamera, betaCamera, cosa, sina;

_SOUND_H_  HANDLE hlog;
