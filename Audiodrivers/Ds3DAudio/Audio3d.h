#ifdef _AUDIO3D_
   #define _SOUND_H_
#else
   #define _SOUND_H_ extern
#endif

#define MAX_CHANNEL           22 //16

/*typedef struct tagSOUNDDEVICEDESC {
   GUID* lpGuid;
   char* lpstrDescription;
   char* lpstrModule;
   LPDSCAPS lpDSC;
   int status;
} SOUNDDEVICEDESC;*/

typedef struct tagCHANNEL {   
   int status;     // 0 means that channel is free
   int iLength;    // filled when appropriate voice is assigned to the channel 
   int iPosition;  // position of next portion to read
   short int* lpData;
   float x, y, z;    // coords of the voice
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


_SOUND_H_ int iSoundActive; 

_SOUND_H_ CHANNEL channel[ MAX_CHANNEL ];
_SOUND_H_ AMBIENT ambient, ambient2;
_SOUND_H_ MAMBIENT mambient;

_SOUND_H_ int xCamera, yCamera, zCamera;
_SOUND_H_ float alphaCamera, betaCamera, cosa, sina;

_SOUND_H_  HANDLE hlog;
