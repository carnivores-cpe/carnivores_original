// Direct Sound 3D native audio driver 

#define INITGUID
#define _AUDIO3D_
#include <dsound.h>
#include "audio3d.h"
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


typedef struct tagEMITTER3D {
	LPDIRECTSOUNDBUFFER lpDSBuf;     // sound buffer
	LPDIRECTSOUND3DBUFFER lpDS3DBuf; // soundbuffer 3D
	DSBUFFERDESC DSBuffDesc;         // buffer description 
} EMITTER3D;

typedef struct tagEMITTER2D {
	LPDIRECTSOUNDBUFFER lpDSBuf;     // sound buffer
	DSBUFFERDESC DSBuffDesc;         // buffer description 
} EMITTER2D;

EMITTER3D sounds[MAX_CHANNEL+2]; // 3D sounds
EMITTER3D dsounds[MAX_CHANNEL+2]; // doublesounds for delay effect
EMITTER2D asounds[3]; //Ambients 2 + 1 m ambient;//Must be used+1 !
/////////////////////////////////////////
HANDLE hAudioThread;
DWORD  AudioTId;

WAVEFORMATEX dsbufwf;  ///////// Do the Same For Ambients
HWND hwndApp;
bool DSSOFT =  false;

//Forwards
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
 wsprintf(logtt, "DirectSound error: %s %s: %Xh\n",em,TranslateDSError(err),err); 
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


LPDIRECTSOUND InitDS3D(void)  //Initialize DirectSound 3D
{
	LPDIRECTSOUND lpD;
	HRESULT err;
		 //PrintLog("Using DirectSound 3D native \n");
         err = DirectSoundCreate(NULL,&lpD,NULL);
         if( err != DS_OK ) { ParseError("CreateDS3D",err); return NULL;}
          return lpD;
}

/////////////////	
int InitDirectSound( HWND hwnd)
{
   iSoundActive = 0;

   //PrintLog("--Direct Sound 3D init (Beta build 1.)--\n");
   HRESULT err;
   wsprintf(logtt,"No Device Enumerating implemented.. \n");  PrintLog(logtt);   

   lpDS = InitDS3D();    
   if (lpDS) PrintLog("Using native DirectSound 3D "); 
   else return false;

   lpDSCAPS.dwSize = sizeof(DSCAPS);
   err = lpDS->GetCaps(&lpDSCAPS);
   if( err != DS_OK ) { ParseError("Caps",err); return false;}

   if (lpDSCAPS.dwMaxHw3DAllBuffers!=0) {
	   PrintLog("(Hardware accelerated mode)\n");DSSOFT = false;}
   else {PrintLog("(Software emulation mode)\n");DSSOFT = true;}

   if (!lpDSCAPS.dwFlags && (DSCAPS_PRIMARY16BIT | DSCAPS_PRIMARYSTEREO | DSCAPS_SECONDARY16BIT | DSCAPS_SECONDARYSTEREO ))
   {   ParseError("Primary Caps error (not a stereo, 16 bit)",err);
	   return false; }

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

   err = lpDS->SetCooperativeLevel(hwnd,DSSCL_EXCLUSIVE);
   if( err != DS_OK ) { ParseError("Cooperative Level",err); return false;}

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


#define MinDistance 64*34
#define MaxDistance 64*400

bool CreateListener3D()
{
   HRESULT err = lpDSBPrimary->QueryInterface(IID_IDirectSound3DListener,(void**)&lpDSListener3D);
   if( err != DS_OK ) { ParseError("QueryInterface Listener 3D (IID_IDirectSound3DListener)",err); return false;} 
   PrintLog("Create Listener3D()\n");

   if (lpDSListener3D == NULL) PrintLog("lpDSListener3D = NULL\n");
      
   //if (!DSSOFT) 
	   lpDSListener3D->SetRolloffFactor((D3DVALUE)2.0,DS3D_DEFERRED);
    //else lpDSListener3D->SetRolloffFactor((D3DVALUE)0.34,DS3D_DEFERRED);

   //if (!DSSOFT)
	   //lpDSListener3D->SetDistanceFactor((D3DVALUE)0.1,DS3D_DEFERRED);

   lpDSListener3D->SetPosition(0,0,0,DS3D_DEFERRED);
   lpDSListener3D->SetOrientation( 0,0,1, 0,1,0, DS3D_DEFERRED);
   err = lpDSListener3D->CommitDeferredSettings();
//   if( err != DS_OK ) { ParseError("CreateListener3D: CommitDeferredSettings",err); return false;} 

   return true;
}


void InitAudioSystem (HWND hw,HANDLE hlogg)
{   
   hwndApp = hw;	
   hlog = hlogg;
   wsprintf(logtt,"\nDirectSound3D(native) audio driver (final)");
   PrintLog(logtt);
   PrintLog(logtt);   
   if( !InitDirectSound( hwndApp ) ) PrintLog("Init Sound System failed\n");
   if( !CreateListener3D() ) PrintLog("Create Listener3D failed\n");
   PrintLog("InitAudioSystem ok\n\n");
}

void Audio_Shutdown(void)
{
 PrintLog(" Releasing Listener3D\n"); 
 if (lpDSListener3D) { lpDSListener3D->Release(); lpDSListener3D = NULL;}
 PrintLog(" Releasing PrimaryBuffer\n");
 if (lpDSBPrimary) { lpDSBPrimary->Release(); lpDSBPrimary = NULL; }
 PrintLog(" Releasing DirectSoundObject\n");
 if (lpDS) { lpDS->Release(); lpDS = NULL; }
 
 PrintLog(" Terminate thread 1\n");
 TerminateThread(hAudioThread,0);
 PrintLog(" AudioShutdown()\n");
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
  err = sounds[num].lpDSBuf->Lock(0,dwSize,&Buffer1,&BufSize1,&Buffer2,&BufSize2,0);
  if( err != DS_OK ) { ParseError("Lock ",err); return false;}  
  CopyMemory(Buffer1,lpData,BufSize1);
  if ( BufSize2 > 0) {
   CopyMemory(Buffer2,lpData+BufSize1,BufSize2);
  }
  err = sounds[num].lpDSBuf->Unlock(Buffer1,BufSize1,Buffer2,BufSize2);
  if( err != DS_OK ) { ParseError("UnLock ",err); return false;}  
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


void AudioSetCameraPos(float cx, float cy, float cz, float ca_, float cb_)
{
  //D3DVECTOR v1,v2;

  xCamera = (int) cx;
  yCamera = (int) cy;
  zCamera = (int) cz;
  alphaCamera = ca_;
  betaCamera = cb_;

  float dx,dy,dz,ux,uy,uz;
  float ca,sa,cb,sb;

  ca = (float) cos(alphaCamera);
  sa = (float) sin(alphaCamera);      
  cb = (float) cos(betaCamera);
  sb = (float) sin(betaCamera);   
  
  dx = sa*cb;
  dy = -sb;  
  dz = -ca*cb;
 
  ux = sa*sb;
  uy = cb;
  uz = -ca*sb;


  Audio_MixChannels();  //SetAmbient3D();
  lpDSListener3D->SetOrientation(dx,dy,dz, ux,uy,uz, DS3D_IMMEDIATE);

}

int int2db(unsigned int lv) //for 3D sounds
{
// lv*=2;
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

void SetEmmiter(int num,float xx,float yy,float zz,int vol,bool play)
{ 
int volu;

	if (sounds[num].lpDS3DBuf) {
    if (!play) sounds[num].lpDS3DBuf->SetMinDistance(MinDistance,DS3D_IMMEDIATE);
    if (!play) sounds[num].lpDS3DBuf->SetMaxDistance(MaxDistance,DS3D_IMMEDIATE);
    sounds[num].lpDS3DBuf->SetPosition(xx,yy,zz,DS3D_IMMEDIATE);
  
	}

 volu = (int)vol/3;

 sounds[num].lpDSBuf->SetVolume(volu);
 sounds[num].lpDSBuf->Play(0,0,0);


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

   //cz=-cz;
   // sprintf(logtt,"AddVoice3dv: len:%d xyz:%3.2f %3.2f %3.2f vol:%d\n",length,cx,cy,cz,vol);  PrintLog(logtt);
   
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
			 if ((cx)&&(cy)&&(cz)) 
			 {
        
			  _x = cx-xCamera;
			  _y = cy-yCamera;
			  _z = cz-zCamera;
			 }else {
				 _x = cx;
				 _y = cy;
				 _z = cz;
			 }
          SetEmmiter(i,_x,_y,_z,int2db(vol*540),true); 
		  } 
         channel[i].status = 1;
		 //channel[i].volume = vol;
		 //if (i==MAX_CHANNEL) PrintLog("MAX_CHANNELS");
	     return 1;
      }
AddVoicev:  //return AddVoice3dv(length, lpdata, 0, 0, 0, v);   
   for( i = 0; i < MAX_CHANNEL; i++ ) 
   if( !channel[i].status ) {      
   if (!LoadEmitter3DFromMem(lpdata,length,i,FALSE)) {continue;}
    if (sounds[i].lpDSBuf) {
          sounds[i].lpDS3DBuf =  NULL; 
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
		 if (!LoadEmitter3DFromMem(lpdata,length,i,FALSE)) {continue;}
		 if (sounds[i].lpDSBuf) {
          sounds[i].lpDS3DBuf =  NULL; 
		  
		  if ((cx==0)&&(cy==0)&&(vol==256)) { //Sounds shots,calls etc
              SetEmmiter(i,0,0,0,-350,true); 

		  } else { //Menu alike processing
		   if ((cy==0)) sounds[i].lpDSBuf->SetPan(((int)(cx*13)-16000));
		   //else sounds[i].lpDSBuf->SetPan(0);
           sounds[i].lpDSBuf->SetVolume(-350); //int2db((int)v/2)
		   sounds[i].lpDSBuf->Play(0,0,0);
		  } 
		 } 
         channel[i].status = 1;
		 //if (i==MAX_CHANNEL) PrintLog("MAX_CHANNELS");
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
//PrintLog("   AudioStop!\n");
#ifdef _AMBIENTS_
if (asounds[0].lpDSBuf) { 
    asounds[0].lpDSBuf->Stop();
    asounds[0].lpDSBuf->Release();
	asounds[0].lpDSBuf = NULL;
   }
if (asounds[1].lpDSBuf) { 
    asounds[1].lpDSBuf->Stop();
    asounds[1].lpDSBuf->Release();
 	asounds[1].lpDSBuf = NULL;
   }
if (asounds[2].lpDSBuf) { 
    asounds[2].lpDSBuf->Stop();
    asounds[2].lpDSBuf->Release();
    asounds[2].lpDSBuf = NULL;
   }
#endif

   for( int i = 0; i < MAX_CHANNEL; i++ ) 
     if(channel[i].status ) {      
		 
		 if (sounds[i].lpDSBuf) {
			if (sounds[i].lpDSBuf) sounds[i].lpDSBuf->Stop();
            if (sounds[i].lpDS3DBuf) sounds[i].lpDS3DBuf->Release();
			sounds[i].lpDS3DBuf = NULL;
			if (sounds[i].lpDSBuf) sounds[i].lpDSBuf->Release();
			sounds[i].lpDSBuf = NULL;
			channel[ i ].status = 0;
			 }
		 } 
PrintLog("  AudioStop ()\n");
}
/////////////////////////


void SetAmbient(int length, short int* lpdata, int av){
#ifdef _AMBIENTS_
   if (!iSoundActive) return;
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
			  _x = cx-xCamera;
			  _y = cy-yCamera;
			  _z = cz-zCamera;

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


int oldE = 0xFF;
void Audio_SetEnvironment(int e, float fadel) 
{
	return;

	if (e==oldE) return;
    oldE = e;
//    sprintf(logtt,"SetEnvironment %d ",e);  PrintLog(logtt);	
/*
    switch (e) {
	 case 0: PrintLog("Generic\n"); break; //Generic
	 case 1: PrintLog("Plate\n"); break; //Plate
	 case 2: PrintLog("Forest\n"); break; //Forest
	 case 3: PrintLog("Mountain\n"); break; //Mountain 
	 case 4: PrintLog("Canyone\n"); break; //Canyone
	 case 5: PrintLog("Special 1 - Cave\n"); break; //Special #1 - Cave
	 case 6: PrintLog("Special 2\n"); break; //Special #2
	 case 7: PrintLog("Special 3\n"); break; //Special #3
     case 8: PrintLog("Underwater \n"); break; //Underwater
	 //default: PrintLog("default - blah!kakogoto huja \n"); break;
	}	 */
}
void Audio_UploadGeometry(int nF, short int* data){ }
