#ifdef _AUDIO3D_
   #define _SOUND_H_
#else
   #define _SOUND_H_ extern
#endif

#define MAX_CHANNEL 22

typedef struct tagEnvironment
{
   float RefGain,RefDelay;
   float MatRefGain,MatRefHF;
   float MatTransGain,MatTransHF;
   float MaxRefDelay;
} A3DEnvironment;

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


_SOUND_H_ int iSoundActive; 

_SOUND_H_ CHANNEL channel[ MAX_CHANNEL ];
_SOUND_H_ AMBIENT ambient, ambient2;
_SOUND_H_ MAMBIENT mambient;

_SOUND_H_ float xCamera, yCamera, zCamera;
_SOUND_H_ float alphaCamera, betaCamera;

_SOUND_H_  HANDLE hlog;


typedef struct tagAudioQuad
{
  float x1,y1,z1;	
  float x2,y2,z2;	
  float x3,y3,z3;	
  float x4,y4,z4;	
} AudioQuad;

