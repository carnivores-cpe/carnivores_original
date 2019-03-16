// EAX 1.0/2.0 audio driver 

#define INITGUID
#define _AUDIO3D_
#include <dsound.h>
#include "audio3d.h"
#include <eax.h>
#include <eax2.h> 
#include <d3dtypes.h>
#include <math.h>
//#include <stdio.h>

#define _AMBIENTS_

#define version 0x00010002
char logtt[1024];

void PrintLog(LPSTR l)
{       DWORD w;
        if (l[strlen(l)-1]==0x0A) {
                BYTE b = 0x0D;
                WriteFile(hlog, l, strlen(l)-1, &w, NULL);
                WriteFile(hlog, &b, 1, &w, NULL);
                b = 0x0A;
                WriteFile(hlog, &b, 1, &w, NULL);
        } else WriteFile(hlog, l, strlen(l), &w, NULL);

}

DSCAPS lpDSCAPS;                         // DSCAPS
LPDIRECTSOUND lpDS;                      // DIRECTSOUND OBJECT
LPDIRECTSOUNDBUFFER lpDSBPrimary;        // PRIMARY BUFFER
LPDIRECTSOUND3DLISTENER lpDSListener3D;  // LISTENER 3D

LPKSPROPERTYSET lpPropertySet;    //Reverb property set
LPDIRECTSOUNDBUFFER lpDummy;   //for EAX stuff;
//LPDIRECTSOUND3DBUFFER lpDummy3D;   //for EAX stuff;
float fReverbMix;

typedef struct tagEMITTER3D {
	bool drun;                       // need to run doublebuffer for delay
	LPDIRECTSOUNDBUFFER lpDSBuf;     // sound buffer
	LPDIRECTSOUND3DBUFFER lpDS3DBuf; // soundbuffer 3D
	DSBUFFERDESC DSBuffDesc;         // buffer description 
    LPKSPROPERTYSET pEmitterPropertySet;  // pReverb property for each 3d emitter
	D3DVECTOR pos;
} EMITTER3D;

typedef struct tagEMITTER2D {
	LPDIRECTSOUNDBUFFER lpDSBuf;     // sound buffer
	DSBUFFERDESC DSBuffDesc;         // buffer description 
} EMITTER2D;

EMITTER3D sounds[MAX_CHANNEL+2]; // 3D sounds
EMITTER2D asounds[3]; //Ambients 2 + 1 m ambient;//Must be used+1 !
/////////////////////////////////////////
HANDLE hAudioThread;
DWORD  AudioTId;

WAVEFORMATEX dsbufwf;  ///////// Do the Same For Ambients
HWND hwndApp;
int totalsounds = 0;

float ca,sa,cb,sb; //camera angles;
//Forwards
bool SetReverb(DSPROPERTY_EAX_LISTENERPROPERTY envID);
void Audio_MixAmbient();
void Audio_MixChannels();
bool LoadEmitter3DFromMem(short int* lpData,int dwSize,int num,bool make3d);
void Audio_Restore(){ return; }
void Audio_MixSound(int DestAddr, int SrcAddr, int MixLen, int LVolume, int RVolume) {}
int ProcessAudio(){ return 1;}
int Audio_GetVersion() { return version; }
BOOL CALLBACK EnumerateSoundDevice( GUID* lpGuid, LPSTR lpstrDescription, LPSTR lpstrModule, LPVOID lpContext){   return TRUE;}
void SetEmmiter(int num,float xx,float yy,float zz,int vol,bool play);

///////////////////////////////////////////////////////
LPSTR TranslateDSError( HRESULT hr ) 
{
    switch (hr) {
	 case DSERR_ALLOCATED:	       return "DSERR_ALLOCATED";
	 case DSERR_CONTROLUNAVAIL:	   return "DSERR_CONTROLUNAVAIL";
	 case DSERR_INVALIDPARAM:	   return "DSERR_INVALIDPARAM";
	 case DSERR_INVALIDCALL:	   return "DSERR_INVALIDCALL";
	 case DSERR_GENERIC:		   return "DSERR_GENERIC";
	 case DSERR_PRIOLEVELNEEDED:   return "DSERR_PRIOLEVELNEEDED";
	 case DSERR_OUTOFMEMORY:	   return "DSERR_OUTOFMEMORY";
	 case DSERR_BADFORMAT:		   return "DSERR_BADFORMAT";
	 case DSERR_UNSUPPORTED:	   return "DSERR_UNSUPPORTED";
	 case DSERR_NODRIVER:		   return "DSERR_NODRIVER";
	 case DSERR_ALREADYINITIALIZED:return "DSERR_ALREADYINITIALIZED";
	 case DSERR_NOAGGREGATION:	   return "DSERR_NOAGGREGATION";
	 case DSERR_BUFFERLOST:		   return "DSERR_BUFFERLOST";
	 case DSERR_OTHERAPPHASPRIO:   return "DSERR_OTHERAPPHASPRIO";
	 case DSERR_UNINITIALIZED:	   return "DSERR_UNINITIALIZED";
     case DSERR_NOINTERFACE:       return "DSERR_NOINTERFACE";
	 default:			           return "Unknown error";}
}
void ParseError(LPSTR em,HRESULT err)
{
 wsprintf(logtt, "EAX error: %s %s: %Xh\n",em,TranslateDSError(err),err); 
 PrintLog(logtt);
}


DWORD WINAPI ProcessAudioThread (LPVOID ptr)
{
   for (;;) {
    if (iSoundActive)
	  Audio_MixAmbient();
    Sleep(70);
   }
   return 0;
}


#define EAX_SETGET (KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET)

int QueryProperty(unsigned long propertyId)   
{  
  DWORD gulReturn;
  HRESULT hr = lpPropertySet->QuerySupport(DSPROPSETID_EAX_ListenerProperties,propertyId,&gulReturn);
 return (FAILED(lpPropertySet->QuerySupport(DSPROPSETID_EAX_ListenerProperties,propertyId,&gulReturn))
                 || ((gulReturn & EAX_SETGET) != EAX_SETGET) ? FALSE : TRUE);}

bool CheckEAX2(void)
{                                    
		if (   !QueryProperty(DSPROPERTY_EAXLISTENER_NONE)   
			|| !QueryProperty(DSPROPERTY_EAXLISTENER_ALLPARAMETERS)
			|| !QueryProperty(DSPROPERTY_EAXLISTENER_ROOM)
			|| !QueryProperty(DSPROPERTY_EAXLISTENER_ROOMHF)
			|| !QueryProperty(DSPROPERTY_EAXLISTENER_ROOMROLLOFFFACTOR)
			|| !QueryProperty(DSPROPERTY_EAXLISTENER_DECAYTIME)
			|| !QueryProperty(DSPROPERTY_EAXLISTENER_DECAYHFRATIO)
			|| !QueryProperty(DSPROPERTY_EAXLISTENER_REFLECTIONS)
			|| !QueryProperty(DSPROPERTY_EAXLISTENER_REFLECTIONSDELAY)
			|| !QueryProperty(DSPROPERTY_EAXLISTENER_REVERB)
			|| !QueryProperty(DSPROPERTY_EAXLISTENER_REVERBDELAY)
			|| !QueryProperty(DSPROPERTY_EAXLISTENER_ENVIRONMENT)
			|| !QueryProperty(DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE)
			|| !QueryProperty(DSPROPERTY_EAXLISTENER_ENVIRONMENTDIFFUSION)
			|| !QueryProperty(DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF)
			|| !QueryProperty(DSPROPERTY_EAXLISTENER_FLAGS)) return false;
		return true;
}

bool CheckEAX1(void)
{
   ULONG support = 0;
   
   HRESULT err = lpPropertySet->QuerySupport(DSPROPSETID_EAX_ReverbProperties,DSPROPERTY_EAX_ALL,&support);
   if( err != DS_OK ) { ParseError("EAXQuerySupport",err); return false;} 

   if ( (support & (EAX_SETGET))!= (EAX_SETGET)) 
   {
	   return false;
   }
return true;
}

bool EAX2=false;
bool EAX1=false;

bool EAXpresent(void)
{
  HRESULT err;
  WAVEFORMATEX wf;
  DSBUFFERDESC DSBuffDesc;

  ZeroMemory(&wf,sizeof(wf));
  wf.wFormatTag = 1; 
  wf.nChannels = 1;
  wf.nSamplesPerSec = 8000;
  wf.wBitsPerSample = 8;
  wf.nBlockAlign = wf.nChannels*wf.wBitsPerSample/8;
  wf.nAvgBytesPerSec = wf.nSamplesPerSec*wf.nBlockAlign;
  wf.cbSize = 0;
  
  ZeroMemory(&DSBuffDesc,sizeof(DSBUFFERDESC));
  DSBuffDesc.dwSize = sizeof(DSBUFFERDESC);
  DSBuffDesc.dwBufferBytes = 32+DSBSIZE_MIN*2; 
  DSBuffDesc.dwFlags =   DSBCAPS_STATIC|DSBCAPS_CTRL3D;
  DSBuffDesc.lpwfxFormat = (LPWAVEFORMATEX)&wf;
  
  //PrintLog("Creating lpDummy\n");
  err = lpDS->CreateSoundBuffer(&DSBuffDesc,&lpDummy,NULL);
  if( err != DS_OK ) {ParseError("CreateDummy",err); return false;}
  
//  err = lpDummy->QueryInterface(IID_IDirectSound3DBuffer, (void**)&lpDummy3D);
//  if( err != DS_OK ) { wsprintf(logtt,"Dummy3D-QueryInterface 3D (IID_IDirectSound3DBuffer)\n"); ParseError(logtt,err); return false;}
  
   err = lpDummy->QueryInterface(IID_IKsPropertySet,(void**)&lpPropertySet);
   if( err != DS_OK ) { ParseError("QueryInteface:IID_IKsPropertySet",err); return false;} 
//	   lpPropertySet->Release();
//	   lpPropertySet = NULL;
   
   EAX1=EAX2=false;
   if (CheckEAX2()) { PrintLog("Detect: EAX 2.0 \n"); EAX2=true; return true;}
   PrintLog("EAX 2.0 is not detected, attempt to detect EAX 1.0 \n");
   if (!EAX2) if (CheckEAX1()) { PrintLog("Detect: EAX 1.0 \n"); EAX1=true; return true;}

return false;	
}

LPDIRECTSOUND InitEAX(void)  ////Initialize Creative EAX
{
	LPDIRECTSOUND lpD;
    HRESULT err;
   
	err = EAXDirectSoundCreate(NULL,&lpD,NULL);
    if( err != DS_OK ) { 
	   ParseError("EAXDirectSoundCreate",err); 
  	   err = DirectSoundCreate(NULL,&lpD,NULL);
  	   if( err != DS_OK ) { ParseError("DirectSoundCreate",err); return false;}
   }
   return lpD;
}

/////////////////
int InitDirectSound( HWND hwnd)
{
   HRESULT err;

   wsprintf(logtt,"No Device Enumerating implemented.. \n");  PrintLog(logtt);   

   lpDS = InitEAX();
   if (lpDS) PrintLog("lpDS created..\n"); 
   else return false;

   if (!EAXpresent()) 
   {
	 PrintLog("EAX 1.0 is not detected. Running in muted mode. Please choose another sound output device\n"); 
   }
    
   lpDSCAPS.dwSize = sizeof(DSCAPS);
   err = lpDS->GetCaps(&lpDSCAPS);
   if( err != DS_OK ) { ParseError("Caps",err); return false;}

   PrintLog("DSCAPS:\n");
   wsprintf(logtt," dwFreeHw3DAllBuffers %d \n",lpDSCAPS.dwFreeHw3DAllBuffers);   PrintLog(logtt);
   wsprintf(logtt," dwFreeHw3DStaticBuffers %d \n",lpDSCAPS.dwFreeHw3DStaticBuffers);   PrintLog(logtt);
   wsprintf(logtt," dwFreeHw3DStreamingBuffers %d \n",lpDSCAPS.dwFreeHw3DStreamingBuffers);   PrintLog(logtt);
   wsprintf(logtt," dwFreeHwMemBytes %d \n",lpDSCAPS.dwFreeHwMemBytes);   PrintLog(logtt);
   wsprintf(logtt," dwTotalHwMemBytes %d \n",lpDSCAPS.dwTotalHwMemBytes);     PrintLog(logtt);
   wsprintf(logtt," dwFreeHwMixingAllBuffers %d \n",lpDSCAPS.dwFreeHwMixingAllBuffers);   PrintLog(logtt);
   wsprintf(logtt," dwFreeHwMixingStaticBuffers %d \n",lpDSCAPS.dwFreeHwMixingStaticBuffers);   PrintLog(logtt);
   wsprintf(logtt," dwFreeHwMixingStreamingBuffers %d \n",lpDSCAPS.dwFreeHwMixingStreamingBuffers);   PrintLog(logtt);
   wsprintf(logtt," dwMaxContigFreeHwMemBytes %d \n",lpDSCAPS.dwMaxContigFreeHwMemBytes);   PrintLog(logtt);
   wsprintf(logtt," dwMinSecondarySampleRate %d \n",lpDSCAPS.dwMinSecondarySampleRate);   PrintLog(logtt);
   wsprintf(logtt," dwMaxSecondarySampleRate %d \n",lpDSCAPS.dwMaxSecondarySampleRate);   PrintLog(logtt);
   wsprintf(logtt," dwMaxHw3DAllBuffers %d \n",lpDSCAPS.dwMaxHw3DAllBuffers);   PrintLog(logtt);
   wsprintf(logtt," dwMaxHw3DStaticBuffers %d \n",lpDSCAPS.dwMaxHw3DStaticBuffers);   PrintLog(logtt);
   wsprintf(logtt," dwMaxHw3DStreamingBuffers %d \n",lpDSCAPS.dwMaxHw3DStreamingBuffers);   PrintLog(logtt);
   wsprintf(logtt," dwMaxHwMixingAllBuffers %d \n",lpDSCAPS.dwMaxHwMixingAllBuffers);   PrintLog(logtt);
   wsprintf(logtt," dwMaxHwMixingStaticBuffers %d \n",lpDSCAPS.dwMaxHwMixingStaticBuffers);   PrintLog(logtt);
   wsprintf(logtt," dwMaxHwMixingStreamingBuffers %d \n",lpDSCAPS.dwMaxHwMixingStreamingBuffers);   PrintLog(logtt);

   err = lpDS->SetCooperativeLevel(hwnd, DSSCL_EXCLUSIVE);
   if( err != DS_OK ) { ParseError("SetCooperativeLevel()",err); return false;}

   WAVEFORMATEX pwf; //Primary wave format
   pwf.wFormatTag      = WAVE_FORMAT_PCM;
   pwf.nChannels       = 2;
   pwf.nSamplesPerSec  = 22050;
   pwf.nAvgBytesPerSec = 22050*2*2;
   pwf.nBlockAlign     = 2*2;
   pwf.wBitsPerSample  = 16;
   pwf.cbSize          = 0;
   
   DSBUFFERDESC dsbd;
   dsbd.dwSize = sizeof( DSBUFFERDESC );
   dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D; //| DSBCAPS_LOCHARDWARE; 
   dsbd.dwBufferBytes = 0; //must be zero for primary buffer
   dsbd.lpwfxFormat = NULL;
   dsbd.dwReserved = 0;
   
   err = lpDS->CreateSoundBuffer(&dsbd,&lpDSBPrimary,NULL);
    if( err != DS_OK ) { ParseError("Create Primary buffer ",err); return false;}
    PrintLog("CreatePrimaryBuffer Ok\n");
   
   err = lpDSBPrimary->SetFormat(&pwf);
    if( err != DS_OK ) { ParseError("Primary set format",err); return false;}   
    PrintLog("Primary SetFormat() Ok\n");
   ////////////////////////////////
   PrintLog("Create audio thread #1..");
    hAudioThread = CreateThread(NULL, 0, ProcessAudioThread, NULL, 0, &AudioTId);
    SetThreadPriority(hAudioThread, THREAD_PRIORITY_HIGHEST);
   PrintLog("ok\n");

   FillMemory( channel, sizeof( CHANNEL )*MAX_CHANNEL, 0 );
   FillMemory( sounds, sizeof( EMITTER3D )*(MAX_CHANNEL+2), 0 );
   iSoundActive = 1;

   ambient.iLength = 0;
   ambient.lpData = NULL;
   ambient.iPosition = 0;
   ambient.volume = 0;
   ambient.avolume = 0;
   ambient2.iLength = 0;
   ambient2.lpData = NULL;
   ambient2.iPosition = 0;
   ambient2.volume = 0;
   ambient2.avolume = 0;

   ZeroMemory(&dsbufwf,sizeof(dsbufwf)); //setting up buffer's WFX
   dsbufwf.wFormatTag = 1; 
   dsbufwf.nChannels = 1;
   dsbufwf.nAvgBytesPerSec = 22050*2*1;
   dsbufwf.nBlockAlign = 2*1;
   dsbufwf.nSamplesPerSec = 22050;
   dsbufwf.wBitsPerSample = 16;
   
   sounds[MAX_CHANNEL+1].lpDSBuf =  NULL;
   return true; 
}

#define MinDistance 20*20
#define MaxDistance 144*20

//#define MinDistance 64*34
//#define MaxDistance 64*400

bool CreateListener3D()
{
   HRESULT err = lpDSBPrimary->QueryInterface(IID_IDirectSound3DListener,(void**)&lpDSListener3D);
   if( err != DS_OK ) { ParseError("QueryInterface Listener 3D (IID_IDirectSound3DListener)",err); return false;} 

   if (lpDSListener3D == NULL) PrintLog("lpDSListener3D = NULL\n");
      
   //if (EAX1) 
   {
    lpDSListener3D->SetRolloffFactor((D3DVALUE)0.95,DS3D_DEFERRED);
	lpDSListener3D->SetDistanceFactor((D3DVALUE)0.0500,DS3D_DEFERRED);
   }

   lpDSListener3D->SetDopplerFactor(0.5,DS3D_DEFERRED);
   lpDSListener3D->SetVelocity(0,0,0,DS3D_DEFERRED);
   lpDSListener3D->SetPosition(0,0,0,DS3D_DEFERRED);
   lpDSListener3D->SetOrientation( 0,0,1, 0,1,0, DS3D_DEFERRED);
   err = lpDSListener3D->CommitDeferredSettings();


   return true;
}

void InitAudioSystem (HWND hw,HANDLE hlogg)
{   
   hwndApp = hw;
   hlog = hlogg;
   wsprintf(logtt,"\nCreative EAX 1.0/2.0 audio driver (beta 4) %d.%d\n", version>>16, version & 0xFFFF);
   PrintLog(logtt);
   if( !InitDirectSound( hwndApp ) ) PrintLog("EAX init failed\n");
   if( !CreateListener3D() ) PrintLog("Create Listener3D failed\n");
   PrintLog("InitAudioSystem ok\n\n");
}

void Audio_Shutdown(void)
{
 PrintLog(" Releasing lpPropertySet\n"); 
 if (lpPropertySet) { lpPropertySet->Release(); lpPropertySet = NULL;}
 PrintLog(" Releasing lpDummy\n"); 
 if (lpDummy) {lpDummy->Release(); lpDummy=NULL;}
 PrintLog(" Releasing Listener3D\n"); 
 if (lpDSListener3D) { lpDSListener3D->Release(); lpDSListener3D = NULL;}
 PrintLog(" Releasing PrimaryBuffer\n");
 if (lpDSBPrimary) { lpDSBPrimary->Release(); lpDSBPrimary = NULL; }
 PrintLog(" Releasing DirectSoundObject\n");
 if (lpDS) { lpDS->Release(); lpDS = NULL; }
  
 PrintLog(" Terminate thread 1\n");
 TerminateThread(hAudioThread,0);
 PrintLog(" AudioShutdown()\n");
 iSoundActive = 0;
}

bool LoadEmitter3DFromMem(short int* lpData,int dwSize,int num,bool make3d) {
  HRESULT err;
  LPVOID Buffer1 = NULL, Buffer2 = NULL;
  DWORD BufSize1 = 0, BufSize2 = 0;
  DWORD dwFlags;

  ZeroMemory(&sounds[num].DSBuffDesc,sizeof(DSBUFFERDESC));

  if (make3d) dwFlags = DSBCAPS_STATIC|DSBCAPS_CTRLVOLUME|DSBCAPS_CTRL3D|DSBCAPS_MUTE3DATMAXDISTANCE;// | DSBCAPS_LOCHARDWARE; //DSBCAPS_LOCSOFTWARE  
  else dwFlags = DSBCAPS_STATIC|DSBCAPS_CTRLVOLUME|DSBCAPS_LOCSOFTWARE;
                      
  sounds[num].DSBuffDesc.dwSize = sizeof(DSBUFFERDESC);
  sounds[num].DSBuffDesc.dwBufferBytes = dwSize; 
  sounds[num].DSBuffDesc.dwFlags =  dwFlags; 
  sounds[num].DSBuffDesc.lpwfxFormat = (LPWAVEFORMATEX)&dsbufwf;

  err = lpDS->CreateSoundBuffer(&sounds[num].DSBuffDesc,&sounds[num].lpDSBuf,NULL);
  if( err != DS_OK ) { ParseError("Create SoundBuffer ",err); return false;}
  
  if (make3d) err = sounds[num].lpDSBuf->QueryInterface(IID_IDirectSound3DBuffer, (void**)&sounds[num].lpDS3DBuf);
  if( err != DS_OK ) { ParseError("Query IID_IDirectSound3DBuffer ",err); return false;} 

  if (make3d) {
  
   err = sounds[num].lpDS3DBuf->QueryInterface(IID_IKsPropertySet,(void**)&sounds[num].pEmitterPropertySet);
   if( err != DS_OK ) { ParseError("QueryInteface:IID_IKsPropertySet",err); 
   
   /******
   lpDSCAPS.dwSize = sizeof(DSCAPS);
   err = lpDS->GetCaps(&lpDSCAPS);
   if( err != DS_OK ) { ParseError("Caps",err); return false;}
  
   wsprintf(logtt,"Error in QueryInterface #%i Free3Dbuf %i",num,lpDSCAPS.dwFreeHw3DAllBuffers);
   MessageBox(hwndApp,logtt,"Bla!",MB_OK);
   return false;/******/
   }
  }
   
  err = sounds[num].lpDSBuf->Lock(0,dwSize,&Buffer1,&BufSize1,&Buffer2,&BufSize2,0);
  if( err != DS_OK ) { ParseError("Lock ",err); return false;}  
  
  CopyMemory(Buffer1,lpData,BufSize1);
  if ( BufSize2 > 0) {
   CopyMemory(Buffer2,lpData+BufSize1,BufSize2);
  }
  err = sounds[num].lpDSBuf->Unlock(Buffer1,BufSize1,Buffer2,BufSize2);
  if( err != DS_OK ) { ParseError("UnLock ",err); return false;}  

  /******
  lpDSCAPS.dwSize = sizeof(DSCAPS);
  err = lpDS->GetCaps(&lpDSCAPS);
  if( err != DS_OK ) { ParseError("Caps",err); return false;}
  totalsounds++;
  wsprintf(logtt,"Created %i  Fre3Dbuf %i \n",totalsounds,lpDSCAPS.dwFreeHw3DAllBuffers);  PrintLog(logtt);
  /******/
  return true;
} 
//////////////////////////////////////////////-\\\\\\ Load Ambients
bool LoadAmbientFromMem(short int* lpData,int dwSize,int num) {
  LPVOID Buffer1 = NULL, Buffer2 = NULL;
  DWORD BufSize1 = 0, BufSize2 = 0;

  ZeroMemory(&asounds[num].DSBuffDesc,sizeof(DSBUFFERDESC));

  asounds[num].DSBuffDesc.dwSize = sizeof(DSBUFFERDESC);
  asounds[num].DSBuffDesc.dwBufferBytes = dwSize; 
  asounds[num].DSBuffDesc.dwFlags =   DSBCAPS_STATIC|DSBCAPS_CTRLVOLUME|DSBCAPS_CTRLPAN|DSBCAPS_LOCSOFTWARE;
  asounds[num].DSBuffDesc.lpwfxFormat = (LPWAVEFORMATEX)&dsbufwf;

  lpDS->CreateSoundBuffer(&asounds[num].DSBuffDesc,&asounds[num].lpDSBuf,NULL);
  
  asounds[num].lpDSBuf->Lock(0,dwSize,&Buffer1,&BufSize1,&Buffer2,&BufSize2,0);
   CopyMemory(Buffer1,lpData,BufSize1);
  if ( BufSize2 > 0) {
   CopyMemory(Buffer2,lpData+BufSize1,BufSize2);
  }
  asounds[num].lpDSBuf->Unlock(Buffer1,BufSize1,Buffer2,BufSize2);
  return true;
}


D3DVECTOR RotateVector(D3DVECTOR& v)
{
  D3DVECTOR vv;

  float vx = v.x * ca + v.z * sa;
  float vz = v.z * ca - v.x * sa;
  float vy = v.y;
  vv.x = vx;
  vv.y = vy * cb - vz * sb;
  vv.z = vz * cb + vy * sb;
  return vv;
}

void AudioSetCameraPos(float cx, float cy, float cz, float ca_, float cb_)
{
  //D3DVECTOR v1,v2;

  xCamera = (int) cx;
  yCamera = (int) cy;
  zCamera = (int) cz;
  alphaCamera = ca_;
  betaCamera = cb_;

   ca = (float)cos(alphaCamera);
   sa = (float)sin(alphaCamera);

   cb = (float)cos(betaCamera);
   sb = (float)sin(betaCamera);

  //Listener 3D position update
  //AnglesToVector(alphaCamera,betaCamera,v1,v2);

   for(int i = 0; i < MAX_CHANNEL+1; i++ ) 
     if( channel[i].status ) {      
		 if (sounds[i].lpDSBuf) {

			  D3DVECTOR t,t2;
		if ((sounds[i].pos.x == 0)&&(sounds[i].pos.y == 0)&&(sounds[i].pos.z == 0))
		{
            SetEmmiter(i,0,0,0,0,false); 
		} else {

			  t.x = sounds[i].pos.x-xCamera;
			  t.y = sounds[i].pos.y-yCamera;
			  t.z = sounds[i].pos.z-zCamera;
			  t2 = RotateVector(t);

			  float _x = t2.x;
              float _y = t2.y;
              float _z = -t2.z;
            SetEmmiter(i,_x,_y,_z,0,false); 
		}
		  } 

	 }		 

  Audio_MixChannels();  
  //SetAmbient3D();
//  UpdateListener3D(cx,cy,cz,v1,v2);
    lpDSListener3D->SetOrientation( 0,0,1,0,1,0, DS3D_IMMEDIATE); //DS3D_IMMEDIATE

}

int int2db(unsigned int lv) //for 3D sounds
{
 lv = lv*3;
 if (lv<=0) return -10000; 
 if (lv>=65535) return 0; 
 return (int)((1667/2)*log((double)lv/65535));  // /255
}
int intA2db(unsigned int lv)  //for Ambients
{
 lv=lv/2;
 if (lv<=0) return -10000; 
 if (lv>=65535) return 0; 
 return (int)((1667/2)*log((double)lv/65535));  // /255
}


ULONG dwFlags =  EAXBUFFERFLAGS_ROOMAUTO | EAXBUFFERFLAGS_DIRECTHFAUTO | EAXBUFFERFLAGS_ROOMHFAUTO;
EAXBUFFERPROPERTIES EAXbuffer = {-30, 0, 0, 0, 0.05f,  0,0,0,0,0, -150, 1.8f, dwFlags};


void SetEmmiter(int num,float xx,float yy,float zz,int vol,bool play)
{ 
int volu;

if (EAX1){
   fReverbMix = EAX_REVERBMIX_USEDISTANCE;
   if FAILED(sounds[num].pEmitterPropertySet->Set(DSPROPSETID_EAXBUFFER_ReverbProperties,DSPROPERTY_EAXBUFFER_REVERBMIX,NULL,0,&fReverbMix,sizeof(float))) PrintLog("SetReverbMix error\n");
}

if (EAX2) if FAILED(sounds[num].pEmitterPropertySet->Set(DSPROPSETID_EAX_BufferProperties,DSPROPERTY_EAXBUFFER_ALLPARAMETERS ,NULL,0, &EAXbuffer,sizeof(EAXBUFFERPROPERTIES)))   PrintLog("Set() error\n");
   

	if (sounds[num].lpDS3DBuf) {
    if (!play) sounds[num].lpDS3DBuf->SetMinDistance(MinDistance,DS3D_IMMEDIATE);
    if (!play) sounds[num].lpDS3DBuf->SetMaxDistance(MaxDistance,DS3D_IMMEDIATE);
    sounds[num].lpDS3DBuf->SetPosition(xx,yy,zz,DS3D_IMMEDIATE);
    }   

if (play) {
  volu = (int)vol/3;
  sounds[num].lpDSBuf->SetVolume(volu);
  sounds[num].lpDSBuf->Play(0,0,0);
}

}
//////////////////////////////////////
//////////////////////////////////////
//////////////////////////////////////


int AddVoice3dv(int length, short int* lpdata, float cx, float cy, float cz, int vol)
{
   float _x,_y,_z;
   int i;
   if (!iSoundActive) return 0;
   if (lpdata == 0) return 0;

//  if ((!EAX2)&&(EAX1)) return 0;
   
   if ((cy==0)&&(vol==256)) goto AddVoice;   
   if ((cx==0)&&(cy==0)) goto AddVoicev;
   //if ((cx==0)&&(cy==0)&&(vol==256)) goto AddVoice;

   for(i = 0; i < MAX_CHANNEL; i++ ) 
     if( !channel[i].status ) {      
		 
		 if (!LoadEmitter3DFromMem(lpdata,length,i,TRUE)) {
			 PrintLog("Load Failed \n");
			 continue; 
		 }

		 if (sounds[i].lpDSBuf) {
			 if ((cx)&&(cy)&&(cz)) {
        
              sounds[i].pos.x = cx;
			  sounds[i].pos.y = cy;
			  sounds[i].pos.z = cz;
			  D3DVECTOR t,t2;
			  t.x = cx-xCamera;
			  t.y = cy-yCamera;
			  t.z = cz-zCamera;
			  t2 = RotateVector(t);

			  _x = t2.x;
              _y = t2.y;
              _z = -t2.z;
			 }else {
			  sounds[i].pos.x = cx;
			  sounds[i].pos.y = cy;
			  sounds[i].pos.z = cz;
				 _x = cx;
				 _y = cy;
				 _z = cz;
			 }

		  SetEmmiter(i,_x,_y,_z,int2db(vol*540),true); 
		  } 
         channel[i].status = 1;
		 //if (i==MAX_CHANNEL) PrintLog("MAX_CHANNELS");
	     return 1;
      }
AddVoicev:  //return AddVoice3dv(length, lpdata, 0, 0, 0, v);   
   for( i = 0; i < MAX_CHANNEL; i++ ) 
   if( !channel[i].status ) {      
   if (!LoadEmitter3DFromMem(lpdata,length,i,true)) {continue;}
    if (sounds[i].lpDSBuf) {
          //sounds[i].lpDS3DBuf =  NULL; 
		  sounds[i].lpDSBuf->SetVolume(int2db(vol*150)); //footsteps
		  sounds[i].lpDSBuf->Play(0,0,0);
		 } 
         channel[i].status = 1;
//		 if (i==MAX_CHANNEL) PrintLog("MAX_CHANNELS");
	     return 1;
      }
AddVoice:  //AddVoice3dv(length, lpdata, 0,0,0, 256);
   for( i = 0; i < MAX_CHANNEL; i++ ) 
   if( !channel[i].status ) {      
		 if (!LoadEmitter3DFromMem(lpdata,length,i,TRUE)) {continue;}

		 if (sounds[i].lpDSBuf) {
          //sounds[i].lpDS3DBuf =  NULL; 
		  
		  if ((cx==0)&&(cy==0)&&(vol==256)) { //Sounds shots,calls etc
			  sounds[i].pos.x = 0;
			  sounds[i].pos.y = 0;
			  sounds[i].pos.z = 0;

          SetEmmiter(i,0,0,0,-350,true); 

		  } 
		 } 
         channel[i].status = 1;
	     return 1;
      }
 return 0;
}


void Audio_MixChannels()
{
    DWORD dwStatus;

    for( int i = 0; i < MAX_CHANNEL; i++ )        
      if( channel[ i ].status ) {

   	    sounds[i].lpDSBuf->GetStatus(&dwStatus);
        if (!(dwStatus & DSBSTATUS_PLAYING)/* != DSBSTATUS_PLAYING*/) {
		  //  PrintLog("Released\n");
			if (sounds[i].lpDSBuf) sounds[i].lpDSBuf->Stop();

            if (sounds[i].pEmitterPropertySet) sounds[i].pEmitterPropertySet->Release();
            sounds[i].pEmitterPropertySet = NULL;

            if (sounds[i].lpDS3DBuf) sounds[i].lpDS3DBuf->Release();
			sounds[i].lpDS3DBuf = NULL;
			
			if (sounds[i].lpDSBuf) sounds[i].lpDSBuf->Release();
            sounds[i].lpDSBuf = NULL;
			channel[i].status = 0;

 			}

      }
}
////////////////////////////////////////////////////////////////////////

void AudioStop()
{
#ifdef _AMBIENTS_
if (asounds[0].lpDSBuf) { 
    asounds[0].lpDSBuf->Stop();
    asounds[0].lpDSBuf->Release();
	asounds[0].lpDSBuf = NULL;   }
if (asounds[1].lpDSBuf) { 
    asounds[1].lpDSBuf->Stop();
    asounds[1].lpDSBuf->Release();
 	asounds[1].lpDSBuf = NULL;   }
if (asounds[2].lpDSBuf) { 
    asounds[2].lpDSBuf->Stop();
    asounds[2].lpDSBuf->Release();
    asounds[2].lpDSBuf = NULL;   }
#endif

   for( int i = 0; i < MAX_CHANNEL; i++ ) 
     if(channel[i].status ) {      
		 
		 if (sounds[i].lpDSBuf) {
			if (sounds[i].lpDSBuf) sounds[i].lpDSBuf->Stop();

            if (sounds[i].pEmitterPropertySet) sounds[i].pEmitterPropertySet->Release();
            sounds[i].pEmitterPropertySet = NULL;
    
			if (sounds[i].lpDS3DBuf) sounds[i].lpDS3DBuf->Release();
			sounds[i].lpDS3DBuf = NULL;
			if (sounds[i].lpDSBuf) sounds[i].lpDSBuf->Release();
			sounds[i].lpDSBuf = NULL;
			channel[ i ].status = 0;
			 }
 		 } 
PrintLog("  AudioStop()\n");
}
/////////////////////////


void SetAmbient(int length, short int* lpdata, int av){
#ifdef _AMBIENTS_
   if (!iSoundActive) return;

   //  if ((!EAX2)&&(EAX1)) return;

   if (ambient.lpData == lpdata) return;
   ambient2 = ambient;
   
   if (ambient.lpData) {
   if (asounds[1].lpDSBuf) { 
    asounds[1].lpDSBuf->Stop();
    asounds[1].lpDSBuf->Release();
   }
     LoadAmbientFromMem(ambient.lpData,ambient.iLength,1);
	   if (asounds[1].lpDSBuf) {
        asounds[1].lpDSBuf->SetVolume(intA2db(ambient2.volume*ambient2.avolume));
		asounds[1].lpDSBuf->Play(0,0,DSBPLAY_LOOPING );
   }}

   ambient.iLength = length;
   ambient.lpData = lpdata;
   ambient.iPosition = 0;
   ambient.volume = 0;
   ambient.avolume = av;
   
   if (asounds[0].lpDSBuf) { 
   asounds[0].lpDSBuf->Stop();
   asounds[0].lpDSBuf->Release();
   }
   LoadAmbientFromMem(ambient.lpData,ambient.iLength,0);
	   if (asounds[0].lpDSBuf) {
        asounds[0].lpDSBuf->SetVolume(intA2db(ambient.volume*ambient.avolume));
		asounds[0].lpDSBuf->Play(0,0,DSBPLAY_LOOPING );
   }
#endif
}

void SetAmbient3d(int length, short int* lpdata, float cx, float cy, float cz)
{
#ifdef _AMBIENTS_
if (!iSoundActive) return;

if ((length==0)&&(lpdata==NULL)) 
{   //ShipStop
	if (sounds[MAX_CHANNEL+1].lpDSBuf){
      sounds[MAX_CHANNEL+1].lpDSBuf->SetVolume(-10000); 
      sounds[MAX_CHANNEL+1].lpDSBuf->Stop();
	  sounds[MAX_CHANNEL+1].lpDS3DBuf->Release();
      sounds[MAX_CHANNEL+1].lpDS3DBuf= NULL;
	  sounds[MAX_CHANNEL+1].lpDSBuf->Release();
      sounds[MAX_CHANNEL+1].lpDSBuf = NULL;
	} 
	return;
}

if (cy==0) //Process Menu
  if (!asounds[2].lpDSBuf)
  {	    LoadAmbientFromMem(lpdata,length,2);
        asounds[2].lpDSBuf->SetVolume(-150);  
		asounds[2].lpDSBuf->Play(0,0,DSBPLAY_LOOPING );
  }
  if (asounds[2].lpDSBuf)  {

  long pan = ((long)(cx*10)-2500);
  (long)pan/=2;
  pan-=4000;

  asounds[2].lpDSBuf->SetPan(pan);
  Audio_MixChannels();  
  }
  else { //Process Ship
 
	 if (!sounds[MAX_CHANNEL+1].lpDSBuf) 
  	   LoadEmitter3DFromMem(lpdata,length,MAX_CHANNEL+1,TRUE);
  
	if (sounds[MAX_CHANNEL+1].lpDSBuf) {
     sounds[MAX_CHANNEL+1].lpDS3DBuf->SetMinDistance(64*1,DS3D_IMMEDIATE);
     sounds[MAX_CHANNEL+1].lpDS3DBuf->SetMaxDistance(64*20*10,DS3D_IMMEDIATE);
   float _x,_y,_z;
D3DVECTOR t,t2;
	          sounds[MAX_CHANNEL+1].pos.x = cx;
			  sounds[MAX_CHANNEL+1].pos.y = cy;
			  sounds[MAX_CHANNEL+1].pos.z = cz;
			  
			  t.x = cx-xCamera;
			  t.y = cy-yCamera;
			  t.z = cz-zCamera;
			  t2 = RotateVector(t);

			  _x = t2.x;
              _y = t2.y;
              _z = -t2.z;

     sounds[MAX_CHANNEL+1].lpDS3DBuf->SetPosition(_x,_y,_z,DS3D_IMMEDIATE);
     sounds[MAX_CHANNEL+1].lpDSBuf->SetVolume(-200);
     sounds[MAX_CHANNEL+1].lpDSBuf->Play(0,0,DSBPLAY_LOOPING );
	}
	  return;  
  }
#endif
}

void Audio_MixAmbient() {
#ifdef _AMBIENTS_
 	
 if (ambient.volume<256) 
  {
	  ambient.volume = min(ambient.volume+10, 256);
	  if (asounds[0].lpDSBuf) asounds[0].lpDSBuf->SetVolume(intA2db(ambient.volume*ambient.avolume));
  }
  if (ambient2.volume) {
	  ambient2.volume = max(ambient2.volume-10, 0);
	  if (asounds[1].lpDSBuf) asounds[1].lpDSBuf->SetVolume(intA2db(ambient2.volume*ambient2.avolume));
  }
#endif
}

//////////////////////////////////////////

EAX_REVERBPROPERTIES forest1       = {EAX_ENVIRONMENT_FOREST,0.12f,2.2145f,0.44f};
EAX_REVERBPROPERTIES generic1      = {EAX_ENVIRONMENT_FOREST,0.42f,1.75f,0.35f}; //EAX_REVERBPROPERTIES generic1      = {EAX_ENVIRONMENT_GENERIC,0.0f,0.0f,0.0f};
EAX_REVERBPROPERTIES mountains1    = {EAX_ENVIRONMENT_MOUNTAINS,1.194f,2.841f,0.4992f};
EAX_REVERBPROPERTIES canyone1      = {EAX_ENVIRONMENT_MOUNTAINS,0.6f,1.3f,0.58f};
EAX_REVERBPROPERTIES plate1        = {EAX_ENVIRONMENT_PLAIN,0.35f,0.67f,0.16f};
EAX_REVERBPROPERTIES special21     = {EAX_ENVIRONMENT_PLAIN,0.5f,0.4f,0.1f};
EAX_REVERBPROPERTIES special1cave1 = {EAX_ENVIRONMENT_STONECORRIDOR,0.444f,1.697f,0.238f};
EAX_REVERBPROPERTIES underwater1   = {EAX_ENVIRONMENT_UNDERWATER,1.0f,1.499f,0.0f};


bool SetReverb(EAX_REVERBPROPERTIES envID)
{
  HRESULT err;

  if ( lpPropertySet ) {
	err = lpPropertySet->Set(DSPROPSETID_EAX_ReverbProperties,DSPROPERTY_EAX_ALL,NULL,0,&envID,sizeof(EAX_REVERBPROPERTIES));
    if( err != DS_OK ) { ParseError("lpPropertySet->Set()",err); return false;} 
  }
  return true;
}

LONG dwLFlags =                  ( EAXLISTENERFLAGS_DECAYTIMESCALE |        
                                   EAXLISTENERFLAGS_REFLECTIONSSCALE |      
                                   EAXLISTENERFLAGS_REFLECTIONSDELAYSCALE | 
                                   EAXLISTENERFLAGS_REVERBSCALE |           
                                   EAXLISTENERFLAGS_REVERBDELAYSCALE |      
                                   EAXLISTENERFLAGS_DECAYHFLIMIT);
typedef struct tagEAX2ENV
{
 ULONG envID;
 ULONG Room;
 float Decay;
 float DecayHF;
 float Diffusion;
 ULONG Reverb;
} EAX2ENV;

#define  LP DSPROPSETID_EAX_ListenerProperties 

EAX2ENV forest2       = {EAX_ENVIRONMENT_FOREST, -400, 2.4f, 0.18f, 0.3f, -700};
EAX2ENV generic2      = {EAX_ENVIRONMENT_FOREST, -400, 2.5f, 0.3f, 0.4f, -620};
EAX2ENV mountains2    = {EAX_ENVIRONMENT_MOUNTAINS, -200, 2.6f, 0.2f, 0.326f, -500};
EAX2ENV canyone2      = {EAX_ENVIRONMENT_MOUNTAINS, -400, 1.1f, 0.6f, 0.275f, -700};
EAX2ENV plate2        = {EAX_ENVIRONMENT_PLAIN, -400, 1.2f, 0.2f, 0.2f, -600};
EAX2ENV special22     = {EAX_ENVIRONMENT_PLAIN, -400, 1.8f, 0.4f, 0.4f, -400};
EAX2ENV special1cave2 = {EAX_ENVIRONMENT_STONECORRIDOR, -300, 3.2f, 0.6f, 0.9f, -200};
EAX2ENV underwater2 =   {EAX_ENVIRONMENT_UNDERWATER, -400, 1.5f, 0.1f, 0.1f, -200};

void SetEAX2Env(EAX2ENV ee)
{
    if FAILED(lpPropertySet->Set(LP,DSPROPERTY_EAXLISTENER_ENVIRONMENT,NULL,0, &ee.envID, sizeof(long))) PrintLog("EnvSet fail\n");
    if FAILED(lpPropertySet->Set(LP,DSPROPERTY_EAXLISTENER_FLAGS,NULL,0, &dwLFlags, sizeof(long))) PrintLog("FlagSet fail\n");
	if FAILED(lpPropertySet->Set(LP,DSPROPERTY_EAXLISTENER_ROOM,NULL,0, &ee.Room, sizeof(long))) PrintLog("RoomSet fail\n");
	if FAILED(lpPropertySet->Set(LP,DSPROPERTY_EAXLISTENER_DECAYTIME,NULL,0, &ee.Decay, sizeof(float))) PrintLog("DecaySet fail\n");
	if FAILED(lpPropertySet->Set(LP,DSPROPERTY_EAXLISTENER_DECAYHFRATIO,NULL,0, &ee.DecayHF, sizeof(ULONG))) PrintLog("DecatHFSet fail\n");
    if FAILED(lpPropertySet->Set(LP,DSPROPERTY_EAXLISTENER_ENVIRONMENTDIFFUSION,NULL,0, &ee.Diffusion, sizeof(float))) PrintLog("DiffSet fail\n");
	if FAILED(lpPropertySet->Set(LP,DSPROPERTY_EAXLISTENER_REVERB,NULL,0, &ee.Reverb, sizeof(ULONG))) PrintLog("VerbSet fail\n");
}

int oldE = 0xFF;
void Audio_SetEnvironment(int e, float fadel) 
{
	if (e==oldE) return;
    oldE = e;
	wsprintf(logtt,"SetEnv %d ",e);  PrintLog(logtt);	

if (EAX2) {
    switch (e) {
	 case 0: SetEAX2Env(generic2);     PrintLog("Generic\n"); break; 
	 case 1: SetEAX2Env(plate2);       PrintLog("Plate\n"); break; 
	 case 2: SetEAX2Env(forest2);      PrintLog("Forest\n"); break; 
	 case 3: SetEAX2Env(mountains2);   PrintLog("Mountain\n"); break;
	 case 4: SetEAX2Env(canyone2);     PrintLog("Canyone\n"); break; 
	 case 5: SetEAX2Env(special1cave2);PrintLog("Special 1 - Cave\n"); break;
	 case 6: SetEAX2Env(special22);    PrintLog("Special 2\n"); break; 
	 case 7: PrintLog("Special 3\n"); break; 
     case 8: SetEAX2Env(underwater2);  PrintLog("Underwater \n"); break; 
	 default: break;	 
	}
}    

	if (EAX1) {
    switch (e) {
	 case 0: SetReverb(generic1);     PrintLog("Generic\n"); break; 
	 case 1: SetReverb(plate1);       PrintLog("Plate\n"); break; 
	 case 2: SetReverb(forest1);      PrintLog("Forest\n"); break; 
	 case 3: SetReverb(mountains1);   PrintLog("Mountain\n"); break;
	 case 4: SetReverb(canyone1);     PrintLog("Canyone\n"); break; 
	 case 5: SetReverb(special1cave1);PrintLog("Special 1 - Cave\n"); break;
	 case 6: SetReverb(special21);    PrintLog("Special 2\n"); break; 
	 case 7: PrintLog("Special 3\n"); break; 
     case 8: SetReverb(underwater1);  PrintLog("Underwater \n"); break; 
	 default: break;	 
	}
	}

}

void Audio_UploadGeometry(int nF, short int* data) { }
