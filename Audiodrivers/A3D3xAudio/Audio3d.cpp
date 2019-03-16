// A3D 3.0 audio driver (Wavetrace/Reverb)

#define INITGUID
#define _AUDIO3D_
 
#define _AMBIENTS_

#define version 0x00010002

#define PI 3.1415926535f

#include <objbase.h>
#include <cguid.h>
#include <ia3dutil.h>
#include <stdio.h>
#include "audio3d.h"
#include <math.h>
#include <d3dtypes.h>
#include <ia3dapi.h>

LPA3D5        lpA3D5;
LPA3DLISTENER lpA3DListener;
LPA3DGEOM2     lpA3DGeom2;
LPA3DMATERIAL lpA3DMaterial0;
LPA3DREVERB   lpA3DReverb;

typedef struct tagEMMITER3D
{
 LPA3DSOURCE2 lpA3DSource2;
 D3DVECTOR pos;
} EMMITER3D;

typedef struct tagEMMITER2D
{
 LPA3DSOURCE2 lpA3DSource2;
} EMMITER2D;

EMMITER3D sounds[MAX_CHANNEL+2];
EMMITER2D asounds[3];

/////////////////////////////////////////
HANDLE hAudioThread;
DWORD  AudioTId;

WAVEFORMATEX wf; 
HWND hwndApp;

bool WavetraceMode =  false;
bool ReverbMode =  false;
bool NonA3D3Mode = false;

#undef RELEASE
#ifdef __cplusplus
#define RELEASE(x) if (x != NULL) {x->Release(); x = NULL;}
#else
#define RELEASE(x) if (x != NULL) {x->lpVtbl->Release(x); x = NULL;}
#endif


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

//Forwards
void Audio_MixAmbient();
void Audio_MixChannels();
void  SetA3D2Environment(A3DEnvironment E);

DWORD WINAPI ProcessAudioThread (LPVOID ptr)
{
   for (;;) {
    if (iSoundActive)
	  Audio_MixAmbient();
    Sleep(70);
   }
   return 0;
}

bool LoadA3D2EmitterFromMem(short int* lpData,int dwSize,int num,bool make3d);
void ParseError(LPSTR em,HRESULT err);


void Audio_Restore(){ return; }
void Audio_MixSound(int DestAddr, int SrcAddr, int MixLen, int LVolume, int RVolume) {}
int ProcessAudio(){ return 1; }

BOOL CALLBACK EnumerateSoundDevice( GUID* lpGuid, LPSTR lpstrDescription, LPSTR lpstrModule, LPVOID lpContext){  return TRUE;}


bool InitA3D2engine(HWND hWin)
{  
	HRESULT err;
	DWORD dwFeatures;

   iSoundActive = 0;

   PrintLog("Initializing A3D 3.0 engine\n");
   A3dRegister();
   wsprintf(logtt,"No Device Enumerating implemented.. \n");  PrintLog(logtt);   

	switch (A3dDetectHardware())
	{
   	 case NO_AUDIO_HARDWARE: PrintLog("Detect: NO_AUDIO_HARDWARE\n"); break;
     case GENERIC_HARDWARE_DETECTED: PrintLog("Detect: GENERIC_HARDWARE_DETECTED\n"); break;
     case A3D_HARDWARE_DETECTED: PrintLog("Detect: A3D_HARDWARE_DETECTED\n"); break;
	}

	dwFeatures = A3D_1ST_REFLECTIONS | A3D_REVERB | A3D_DISABLE_FOCUS_MUTE;// | A3D_GEOMETRIC_REVERB; 
	A3dInitialize();
	err = A3dCreate(NULL,(void**)&lpA3D5,NULL,dwFeatures);
    if (err!=S_OK){ PrintLog("A3DCreate() fail \n"); return false;}

	err = lpA3D5->SetCooperativeLevel(hWin, A3D_CL_EXCLUSIVE); // NORMAL
    if (err!=S_OK){ PrintLog("SetCooperative() fail \n"); return false;}

	PrintLog("dwFeatures:\n");
    PrintLog(" A3D_REVERB");
    if (lpA3D5->IsFeatureAvailable(A3D_REVERB)) 
	{
		PrintLog(" YES\n"); 
		//ReverbMode = true;
	}  
	else 
	{ 
	    PrintLog(" NO\n"); 
		//ReverbMode = false; 
	}

    PrintLog(" A3D_1ST_REFLECTIONS");
    if (lpA3D5->IsFeatureAvailable(A3D_1ST_REFLECTIONS)) 
	{
		PrintLog(" YES\n"); 
		//if (!ReverbMode) WavetraceMode = true;
	}  
	else 
	{ 
	    PrintLog(" NO\n"); 
		//if (!ReverbMode) WavetraceMode = false; 
	}

	WavetraceMode = false;
	ReverbMode =  true;

    PrintLog(" A3D_OCCLUSIONS");
	if (lpA3D5->IsFeatureAvailable(A3D_OCCLUSIONS)) PrintLog(" YES\n"); else PrintLog(" NO\n");

    PrintLog(" A3D_GEOMETRIC_REVERB");
	if (lpA3D5->IsFeatureAvailable(A3D_GEOMETRIC_REVERB)) PrintLog(" YES\n"); else PrintLog(" NO\n");

    PrintLog(" A3D_DIRECT_PATH_A3D");
	if (lpA3D5->IsFeatureAvailable(A3D_DIRECT_PATH_A3D)) PrintLog(" YES\n"); else PrintLog(" NO\n");

    PrintLog(" A3D_DIRECT_PATH_GENERIC");
	if (lpA3D5->IsFeatureAvailable(A3D_DIRECT_PATH_GENERIC)) PrintLog(" YES\n"); else PrintLog(" NO\n");
	
	err = lpA3D5->SetCoordinateSystem(A3D_RIGHT_HANDED_CS);
    if (err!=S_OK) { ParseError("SetCoordinateSystem\n",err); return false;}

	err = lpA3D5->QueryInterface(IID_IA3dListener, (void **)&lpA3DListener);
    if (err!=S_OK){ PrintLog("QUERYINTERFACE: IID_IA3DListener \n"); return false;}
	
	err = lpA3D5->QueryInterface(IID_IA3dGeom2, (void **)&lpA3DGeom2);
    if (err!=S_OK){ PrintLog("QUERYINTERFACE: IID_IA3dGeom  \n"); return false;}

	err = lpA3DGeom2->NewMaterial(&lpA3DMaterial0);
    if (err!=S_OK) { ParseError("NewMaterial\n",err); return false;}

	err = lpA3D5->Clear();
    if (err!=S_OK) { ParseError("Clear: initial\n",err); return false;}

    if (ReverbMode) {
	err = lpA3D5->NewReverb(&lpA3DReverb);
    if (err!=S_OK) { ParseError("SetReverb\n",err); NonA3D3Mode = true;/*return false;*/}
	}

    if (!NonA3D3Mode) {
     err = lpA3D5->BindReverb(lpA3DReverb);
     if (err!=S_OK) { ParseError("BindReverb\n",err); /*return false;*/}
    }
 
/*    if (WavetraceMode) {
	 err = lpA3DGeom2->Enable(A3D_1ST_REFLECTIONS);
	 if (err!=S_OK) { ParseError("Enable Reflections",err); }

	 //err = lpA3DGeom2->Enable(A3D_GEOMETRIC_REVERB);
	 //if (err!=S_OK) { ParseError("Enable Geometry based reverb",err); } 

	 err = lpA3D5->SetMaxReflectionDelayTime(2.3f);
	 if (err!=S_OK) { ParseError("SetMaxReflectionDelayTime",err); }
	}

*/

    if (ReverbMode) PrintLog("InitA3D3 engine - Reverb mode init ok\n");
//	if (WavetraceMode) PrintLog("InitA3D3engine -  Wavetrace mode init ok\n");
   
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

   ZeroMemory(&wf,sizeof(wf)); //setting up buffer's WF
   wf.wFormatTag = 1; 
   wf.nChannels = 1;
   wf.nAvgBytesPerSec = 22050*2*1;
   wf.nBlockAlign = 2*1;
   wf.nSamplesPerSec = 22050;
   wf.wBitsPerSample = 16;
   
   sounds[MAX_CHANNEL+1].lpA3DSource2 =  NULL;
   return true; 
}


int Audio_GetVersion() { return version; }

void InitAudioSystem (HWND hw,HANDLE hlogg)
{   
   hwndApp = hw;
   hlog = hlogg;
   PrintLog("\n");   
   wsprintf(logtt,"A3D 3.0 audio (Wavetrace/Reverb) driver (Beta 1) \n","%d.%d \n", version>>16, version & 0xFFFF);
   PrintLog(logtt);   

   if( !InitA3D2engine( hwndApp ) ) { PrintLog("Sound System init failed\n"); return;}
   PrintLog("InitAudioSystem ok\n\n");
}
 

void Audio_Shutdown()
{
 PrintLog("Releasing A3D 3.0 engine\n"); 
 RELEASE(lpA3DListener);
 RELEASE(lpA3DGeom2);
 RELEASE(lpA3D5);
 A3dUninitialize();
 PrintLog(" Terminate thread 1\n");
 TerminateThread(hAudioThread,0);
 PrintLog(" AudioShutdown()\n");
}


bool LoadA3D2EmitterFromMem(short int* lpData,int dwSize,int num,bool make3d) {
  HRESULT err;
  LPVOID Buffer1 = NULL, Buffer2 = NULL;
  DWORD BufSize1 = 0, BufSize2 = 0;
  DWORD dwFlags;

//  if (make3d){ wsprintf(logtt,"Creating A3D2 Emitter 3D #%d from memory datasize: %i\n",num,dwSize);  PrintLog(logtt); }
//  else wsprintf(logtt,"Creating A3D2 Emitter 2D #%d from memory datasize: %i\n",num,dwSize);  PrintLog(logtt); 

  if (make3d) dwFlags = A3DSOURCE_INITIAL_RENDERMODE_A3D;
  else dwFlags = A3DSOURCE_INITIAL_RENDERMODE_NATIVE;

  err = lpA3D5->NewSource(dwFlags, &sounds[num].lpA3DSource2);
  //if (err!=S_OK){ ParseError("NewSource() error \n",err); return false;}

  err = sounds[num].lpA3DSource2->SetAudioFormat(&wf); 
  //if( err != S_OK ) { ParseError("SetWaveFormat()",err); return false;}
  
  err = sounds[num].lpA3DSource2->AllocateAudioData(dwSize); 
  //if( err != S_OK ) { ParseError("AllocateWaveData()",err); return false;}

/*  if (EAX10mode) 
  {
	  err = sounds[num].lpA3DSource2->QueryInterface(IID_IA3dPropertySet,(void**)&sounds[num].lpEmmiterPropertySet);
	  if( err != S_OK ) { ParseError("QueryInteface:IID_IA3dPropertySet",err);} 
  }*/

  err = sounds[num].lpA3DSource2->Lock(0,dwSize,&Buffer1,&BufSize1,&Buffer2,&BufSize2,0);
  //if( err != S_OK ) { ParseError("Lock error",err); return false;}
  CopyMemory(Buffer1,lpData,BufSize1);
  if ( BufSize2 > 0) CopyMemory(Buffer2,lpData+BufSize1,BufSize2);
  err = sounds[num].lpA3DSource2->Unlock(Buffer1,BufSize1,Buffer2,BufSize2);

  return true;
}

//////////////////////////////////////////////-\\\\\\ Load Ambients//////////////////////////////////////////////-\\\\\\ Load Ambients

bool LoadAmbientFromMem(short int* lpData,int dwSize,int num) {
  HRESULT err;
  LPVOID Buffer1 = NULL, Buffer2 = NULL;
  DWORD BufSize1 = 0, BufSize2 = 0;

//  wsprintf(logtt,"Creating Ambient #%d from memory datasize: %i\n",num,dwSize);  PrintLog(logtt); 
  
  err = lpA3D5->NewSource(A3DSOURCE_INITIAL_RENDERMODE_NATIVE, &asounds[num].lpA3DSource2);
  //if (err!=S_OK){ ParseError("NewSource() error \n",err); return false;}

  err = asounds[num].lpA3DSource2->SetAudioFormat(&wf); 
  //if( err != S_OK ) { ParseError("SetWaveFormat()",err); return false;}
  
  err = asounds[num].lpA3DSource2->AllocateAudioData(dwSize); 
  //if( err != S_OK ) { ParseError("AllocateWaveData()",err); return false;}
  
  err = asounds[num].lpA3DSource2->Lock(0,dwSize,&Buffer1,&BufSize1,&Buffer2,&BufSize2,0);
  //wsprintf(logtt," Lock buffer size %d %d\n",BufSize1,BufSize2);  PrintLog(logtt);
  //if( err != S_OK ) { ParseError("Lock error",err); return false;}
  CopyMemory(Buffer1,lpData,BufSize1);
  //wsprintf(logtt," BufSize1: transferred %d bytes from wave file\n",BufSize1); PrintLog(logtt);
  if ( BufSize2 > 0) { CopyMemory(Buffer2,lpData+BufSize1,BufSize2);
   //wsprintf(logtt," BufSize2: transferred %d bytes from wave file\n",BufSize2); PrintLog(logtt);
  }
  err = asounds[num].lpA3DSource2->Unlock(Buffer1,BufSize1,Buffer2,BufSize2);
  //if( err != S_OK ) { ParseError("Unlock error",err); return false;}
  //wsprintf(logtt," Unlock buffer size %d %d\n",BufSize1,BufSize2); PrintLog(logtt);
 
  //wsprintf(logtt,"Emitter #%d created succesful\n\n",num); PrintLog(logtt);

  return true;
}


float ca,sa;
void AudioSetCameraPos(float cx, float cy, float cz, float ca_, float cb_)
{
  xCamera = cx;
  yCamera = cy;
  zCamera = cz;
  alphaCamera = ca_;
  betaCamera = cb_;

  ca = (float)cos(alphaCamera);
  sa = (float)sin(alphaCamera);
 
  /*if (WavetraceMode) {
   lpA3D5->Clear();    
   lpA3DGeom2->LoadIdentity();
  }*/

  Audio_MixChannels(); 
  
  if (lpA3DListener) {
     lpA3DListener->SetOrientationAngles3f(-alphaCamera*180/PI,-betaCamera*180/PI,0);  
     //lpA3DListener->SetPosition3f(0,0,0);
	}

  lpA3D5->Flush();
}

void  Audio_UploadGeometry (int nF, AudioQuad* qdata)
{
  
//  if (ReverbMode) { lpA3D5->Flush(); return; }

  /*lpA3DGeom2->PushMatrix();
  lpA3DGeom2->BindMaterial(lpA3DMaterial0);
  for (int f=0;f<nF;f++)
  {    
      lpA3DGeom2->Begin(A3D_QUADS);  
	  lpA3DGeom2->Tag(0xAA+f);
      lpA3DGeom2->Vertex3f( qdata[f].x1, qdata[f].y1, qdata[f].z1 );
      lpA3DGeom2->Vertex3f( qdata[f].x2, qdata[f].y2, qdata[f].z2 );
      lpA3DGeom2->Vertex3f( qdata[f].x3, qdata[f].y3, qdata[f].z3 );
      lpA3DGeom2->Vertex3f( qdata[f].x4, qdata[f].y4, qdata[f].z4 );
      lpA3DGeom2->End();       
  }
  lpA3DGeom2->PopMatrix();

  lpA3D5->Flush();*/
  
}

//int volu;
float dmax = 0;
void SetEmmiter(int num,float xx,float yy,float zz, float vol)
{ 
	/*if (ReverbMode) {
    
	  float d = (float)sqrt(xx*xx + yy*yy + zz*zz);
	  float rmix;

	  if (d!=0) {
 		 rmix = (float) (d/11000.f);
         if (rmix>1.f) rmix = 1.f;
//		 rmix = 1 - rmix;
      
	  } else rmix = 0.3f;

	  //if (d>dmax) 
	  //{
	  //dmax = d;
	  //sprintf(logtt,"max - %i - %f\n",num,dmax);  PrintLog(logtt); 
	  }


      //sprintf(logtt,"%i - %2.3f - %f\n",num, rmix, d);  PrintLog(logtt); 
	  
	  A3DVAL r1,h1;
	 HRESULT err = sounds[num].lpA3DSource2->SetReverbMix(-1, rmix);
      if( err != S_OK ) { ParseError("SetReverbMix()",err);}
      sprintf(logtt,"%f - %f\n",r1, h1);  PrintLog(logtt); 
	}
/**/
 sounds[num].lpA3DSource2->SetMinMaxDistance(640,640*80*2,A3D_MUTE);
 sounds[num].lpA3DSource2->SetPosition3f(xx,yy,zz);
 sounds[num].lpA3DSource2->SetGain(vol);
 sounds[num].lpA3DSource2->Play(A3D_SINGLE);
}
//////////////////////////////////////
////////////////////////////////////

int AddVoice3dv(int length, short int* lpdata, float cx, float cy, float cz, int vol)
{
   float _x,_y,_z;
   int i;

   if (!iSoundActive) return 0;
   if (lpdata == 0) return 0;

   // sprintf(logtt,"AddVoice3dv: len:%d xyz:%3.2f %3.2f %3.2f vol:%d\n",length,cx,cy,cz,vol);  PrintLog(logtt);
   
   if ((cy==0) && (vol==256)) goto AddVoice;   
   if ((cx==0) && (cy==0)) goto AddVoicev;
   //if ((cx==0)&&(cy==0)&&(vol==256)) goto AddVoice;

   for(i = 0; i < MAX_CHANNEL; i++ ) 
     if( !channel[i].status ) {      
		 
		 if (!LoadA3D2EmitterFromMem(lpdata,length,i,TRUE)) {
			// PrintLog("Load Failed \n");
			 continue; 
		 }

		 if (sounds[i].lpA3DSource2) {
			 if ((cx)&&(cy)&&(cz)) {
		      _x = cx-xCamera;
              _y = cy-yCamera;
              _z = cz-zCamera;
			 }else {
				 _x = cx;
				 _y = cy;
				 _z = cz;
			 }
          SetEmmiter(i,_x,_y,_z,(float)vol/256); 
		  } 
         channel[i].status = 1;

	     return 1;
      }
AddVoicev:  // AddVoice3dv(length, lpdata, 0, 0, 0, v);    
   for( i = 0; i < MAX_CHANNEL; i++ ) //footsteps, deadscreams, shipvoice etc
   if( !channel[i].status ) {      
   if (!LoadA3D2EmitterFromMem(lpdata,length,i,FALSE)) {continue;}
    if (sounds[i].lpA3DSource2) {
		  sounds[i].lpA3DSource2->SetGain((float)vol/256);
		  sounds[i].lpA3DSource2->Play(A3D_SINGLE);
		 } 
         channel[i].status = 1;
	     return 1;
      }
AddVoice:  //AddVoice3dv(length, lpdata, 0,0,0, 256);
   for( i = 0; i < MAX_CHANNEL; i++ ) 
   if( !channel[i].status )  {      
		 if (!LoadA3D2EmitterFromMem(lpdata,length,i,TRUE)) {continue;}

		 if (sounds[i].lpA3DSource2) {
 
		  if ((cx==0)&&(cy==0)&&(vol==256)) { // shots,calls
              SetEmmiter(i,0,0,0, 1.0f );   //-300
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

   	    sounds[i].lpA3DSource2->GetStatus(&dwStatus);

        if ( !(dwStatus & A3DSTATUS_PLAYING) && !(dwStatus & A3DSTATUS_WAITING_FOR_FLUSH)) 
		{
		    if (sounds[i].lpA3DSource2) sounds[i].lpA3DSource2->Stop();
			if (sounds[ i ].lpA3DSource2) RELEASE(sounds[ i ].lpA3DSource2);
			channel[ i ].status = 0;
		}
    }
}
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

void AudioStop()
{
#ifdef _AMBIENTS_
if (asounds[0].lpA3DSource2) { 
    asounds[0].lpA3DSource2->Stop();
    asounds[0].lpA3DSource2->Release();
	asounds[0].lpA3DSource2 = NULL;   }
if (asounds[1].lpA3DSource2) { 
    asounds[1].lpA3DSource2->Stop();
    asounds[1].lpA3DSource2->Release();
 	asounds[1].lpA3DSource2 = NULL;   }
if (asounds[2].lpA3DSource2) { 
    asounds[2].lpA3DSource2->Stop();
    asounds[2].lpA3DSource2->Release();
    asounds[2].lpA3DSource2 = NULL;   }
#endif

   for( int i = 0; i < MAX_CHANNEL; i++ ) 
     if(channel[i].status ) {      
		 
		 if (sounds[i].lpA3DSource2) {
			if (sounds[ i ].lpA3DSource2) sounds[ i ].lpA3DSource2->Stop();
            if (sounds[ i ].lpA3DSource2) RELEASE(sounds[ i ].lpA3DSource2);
			channel[ i ].status = 0;
			 }		 
	 } 
PrintLog("AudioStop()\n");
}
/////////////////////////


void SetAmbient(int length, short int* lpdata, int av){
#ifdef _AMBIENTS_

   av/=2;   //////////////////////////////// half volume

   if (!iSoundActive) return;
   if (ambient.lpData == lpdata) return;
   ambient2 = ambient;
   
   if (ambient.lpData) {
   if (asounds[1].lpA3DSource2) { 
    asounds[1].lpA3DSource2->Stop();
    asounds[1].lpA3DSource2->Release();
   }
     LoadAmbientFromMem(ambient.lpData,ambient.iLength,1);
	   if (asounds[1].lpA3DSource2) {
        asounds[1].lpA3DSource2->SetGain((float)(ambient2.volume*ambient2.avolume)/65535); //SetVolume(intA2db(ambient2.volume*ambient2.avolume));
		asounds[1].lpA3DSource2->Play(A3D_LOOPED);
   }}

   ambient.iLength = length;
   ambient.lpData = lpdata;
   ambient.iPosition = 0;
   ambient.volume = 0;
   ambient.avolume = av;
   
   if (asounds[0].lpA3DSource2) { 
   asounds[0].lpA3DSource2->Stop();
   asounds[0].lpA3DSource2->Release();
   }
   LoadAmbientFromMem(ambient.lpData,ambient.iLength,0);
	   if (asounds[0].lpA3DSource2) {
        asounds[0].lpA3DSource2->SetGain((float)(ambient.volume*ambient.avolume)/65535);//SetVolume(intA2db(ambient.volume*ambient.avolume));
		asounds[0].lpA3DSource2->Play(A3D_LOOPED);
   }
#endif
}

void SetAmbient3d(int length, short int* lpdata, float cx, float cy, float cz)
{
#ifdef _AMBIENTS_
if (!iSoundActive) return;

if ((length==0)&&(lpdata==NULL)) 
{   //ShipStop
	if (sounds[MAX_CHANNEL+1].lpA3DSource2){
      sounds[MAX_CHANNEL+1].lpA3DSource2->SetGain(0); 
      sounds[MAX_CHANNEL+1].lpA3DSource2->Stop();
	  sounds[MAX_CHANNEL+1].lpA3DSource2->Release();
      sounds[MAX_CHANNEL+1].lpA3DSource2= NULL;
	} 
	return;
}
	  //Process Ship
 
	 if (!sounds[MAX_CHANNEL+1].lpA3DSource2) 
  	   LoadA3D2EmitterFromMem(lpdata,length,MAX_CHANNEL+1,TRUE);
  
	if (sounds[MAX_CHANNEL+1].lpA3DSource2) {
     sounds[MAX_CHANNEL+1].lpA3DSource2->SetMinMaxDistance(64*10,64*80*10,A3D_MUTE);
     
     sounds[MAX_CHANNEL+1].lpA3DSource2->SetPosition3f(cx-xCamera,cy-yCamera,cz-zCamera);
     sounds[MAX_CHANNEL+1].lpA3DSource2->SetGain((float)0.9f);
     sounds[MAX_CHANNEL+1].lpA3DSource2->Play(A3D_LOOPED);
	}
  return;  
#endif
}

void Audio_MixAmbient() {
#ifdef _AMBIENTS_
 	
 if (ambient.volume<256) 
  {
	  ambient.volume = min(ambient.volume+16, 256);
	  if (asounds[0].lpA3DSource2) asounds[0].lpA3DSource2->SetGain((float)(ambient.volume*ambient.avolume)/65535); //SetVolume(intA2db(ambient.volume*ambient.avolume));
  }
  if (ambient2.volume) {
	  ambient2.volume = max(ambient2.volume-16, 0);
	  if (asounds[1].lpA3DSource2) asounds[1].lpA3DSource2->SetGain((float)(ambient2.volume*ambient2.avolume)/65535); //SetVolume(intA2db(ambient2.volume*ambient2.avolume));
  }
#endif
}

 
void  SetA3D2Environment(A3DEnvironment E)
{ 
  //HRESULT err;
  lpA3D5->SetEq(1.0f);
  //if( err != S_OK ) { ParseError("SetEQ()",err); return ;}
  lpA3DGeom2->SetReflectionGainScale(E.RefGain);
  //if( err != S_OK ) { ParseError("SetReflectionGainScale()",err); return ;}
  lpA3DGeom2->SetReflectionDelayScale(E.RefDelay);
  //if( err != S_OK ) { ParseError("SetReflectionDelayScale()",err); return ;}
  lpA3DMaterial0->SetReflectance(E.MatRefGain, E.MatRefHF); 
  //if( err != S_OK ) { ParseError("SetReflectance()",err); return ;}
  lpA3DMaterial0->SetTransmittance(E.MatTransGain, E.MatTransHF);
  //if( err != S_OK ) { ParseError("SetTransmittance()",err); return ;}

//  lpA3D5->SetMaxReflectionDelayTime(E.MaxRefDelay);
/*   
  sprintf(logtt,"Environment %1.1f  %1.1f %1.1f  %1.1f %1.1f  %1.1f %1.1f\n",
	             E.MaxRefDelay,
				 E.RefGain,E.RefDelay,	
				 E.MatRefGain,E.MatRefHF,
				 E.MatTransGain,E.MatTransHF);*/
  PrintLog(logtt);
}
//                               RG    RD    MRG   MRHF  MTG   MTHF
A3DEnvironment forestA       = {0.7f, 0.7f,  0.4f, 0.3f, 0.9f, 0.9f};
A3DEnvironment genericA      = {0.8f, 1.0f,  0.5f, 0.5f, 0.8f, 0.9f};
A3DEnvironment plateA        = {0.7f, 0.9f,  0.4f, 0.4f, 0.9f, 0.9f};
A3DEnvironment mountainsA    = {0.8f, 0.3f,  0.7f, 0.8f, 0.9f, 0.9f};
A3DEnvironment special2A     = {0.7f, 0.6f,  0.8f, 0.1f, 0.9f, 0.9f};
A3DEnvironment canyoneA      = {0.7f, 0.7f,  0.5f, 0.3f, 0.9f, 0.9f};
A3DEnvironment special1A     = {0.8f, 0.2f,  0.7f, 0.9f, 0.9f, 0.9f};
//A3DEnvironment underwater   = {0.5f, 1.0f,  0.3f, 0.2f, 0.9f, 0.9f};


A3DREVERB_PRESET forest1       = {0, A3DREVERB_PRESET_FOREST,0.12f,1.92145f,0.44f};
A3DREVERB_PRESET generic1      = {0, A3DREVERB_PRESET_FOREST,0.1f,1.2f,0.35f}; 
A3DREVERB_PRESET mountains1    = {0, A3DREVERB_PRESET_MOUNTAINS,0.994f,2.841f,0.4992f};
A3DREVERB_PRESET canyone1      = {0, A3DREVERB_PRESET_PLAIN,0.5f,0.34f,0.1f};
A3DREVERB_PRESET plate1        = {0, A3DREVERB_PRESET_PLAIN,0.25f,0.67f,0.16f};
A3DREVERB_PRESET special21     = {0, A3DREVERB_PRESET_MOUNTAINS,0.6f,1.3f,0.58f};
A3DREVERB_PRESET special1cave1 = {0, A3DREVERB_PRESET_STONECORRIDOR,0.1f,0.3f,0.2f};
A3DREVERB_PRESET underwater1   = {0, A3DREVERB_PRESET_UNDERWATER,0.3f,1.499f,0.0f};


int SetA3DReverb(A3DREVERB_PRESET ee)
{
	//HRESULT err;
	ee.dwSize = sizeof(A3DREVERB_PRESET);

	lpA3DReverb->SetReverbPreset(ee.dwEnvPreset);
    //if (err!=S_OK) { ParseError("SetReverbPreset\n",err); return false;}
    lpA3DReverb->SetPresetVolume(ee.fVolume);
    //if (err!=S_OK) { ParseError("SetPresetVolume\n",err); return false;}
    lpA3DReverb->SetPresetDecayTime(ee.fDecayTime);
    //if (err!=S_OK) { ParseError("SetPresetDecayTime\n",err); return false;}
    lpA3DReverb->SetPresetDamping(ee.fDamping);
    //if (err!=S_OK) { ParseError("SetPresetDamping\n",err); return false;}
	return true;
}

int oldE = 0xFF;
void Audio_SetEnvironment(int e, float fadel) 
{
	if (e==oldE) return;
    oldE = e;
	wsprintf(logtt,"SetEnv %d ",e);  PrintLog(logtt);	
    
	if (NonA3D3Mode) return;

	if (ReverbMode) {
    switch (e) {
	 case 0: PrintLog("R_Generic - \n"); SetA3DReverb(generic1); break; 
	 case 1: PrintLog("R_Plate   - \n"); SetA3DReverb(plate1); break; 
	 case 2: PrintLog("R_Forest  - \n"); SetA3DReverb(forest1); break; 
	 case 3: PrintLog("R_Mountain- \n"); SetA3DReverb(mountains1); break;
	 case 4: PrintLog("R_Canyone -\n"); SetA3DReverb(canyone1); break; 
	 case 5: PrintLog("R_Special1- Cave \n"); SetA3DReverb(special1cave1); break;
	 case 6: PrintLog("R_Special2-\n"); SetA3DReverb(special21); break; 
	 case 7: PrintLog("R_Unused - Special 3\n"); break; 
     case 8: PrintLog("R_Underwater -\n"); break; lpA3D5->SetEq(0.23f);
		SetA3DReverb(underwater1); 
	 default: break;	 
		}
	}

/*	if (WavetraceMode) {
    switch (e) {
	 case 0: PrintLog("W_Generic - \n"); SetA3D2Environment(genericA); break; 
	 case 1: PrintLog("W_Plate   - \n"); SetA3D2Environment(plateA); break; 
	 case 2: PrintLog("W_Forest  - \n"); SetA3D2Environment(forestA); break; 
	 case 3: PrintLog("W_Mountain- \n"); SetA3D2Environment(mountainsA); break;
	 case 4: PrintLog("W_Canyone -\n"); SetA3D2Environment(canyoneA); break; 
	 case 5: PrintLog("W_Special1- Cave \n"); SetA3D2Environment(special1A); break;
	 case 6: PrintLog("W_Special2-\n"); SetA3D2Environment(special2A); break; 
	 case 7: PrintLog("W_Unused - Special 3\n"); break; 
     case 8: 
		lpA3D5->SetEq(0.23f);
  	    PrintLog("WUnderwater -\n"); break; 
	 default: break;	 
		}
	}*/
}

LPSTR TranslateA3DError( HRESULT hr ) 
{
  switch (hr) {
 case A3DERROR_MEMORY_ALLOCATION		: return  "A3DERROR_MEMORY_ALLOCATION";
 case A3DERROR_FAILED_CREATE_PRIMARY_BUFFER					: return  "A3DERROR_FAILED_CREATE_PRIMARY_BUFFER";
 case A3DERROR_FAILED_CREATE_SECONDARY_BUFFER					: return  "A3DERROR_FAILED_CREATE_SECONDARY_BUFFER";
 case A3DERROR_FAILED_INIT_A3D_DRIVER							: return  "A3DERROR_FAILED_INIT_A3D_DRIVER";
 case A3DERROR_FAILED_QUERY_DIRECTSOUND						: return  "A3DERROR_FAILED_QUERY_DIRECTSOUND";
 case A3DERROR_FAILED_QUERY_A3D3								: return  "A3DERROR_FAILED_QUERY_A3D3";
 case A3DERROR_FAILED_INIT_A3D3								: return  "A3DERROR_FAILED_INIT_A3D3";
 case A3DERROR_FAILED_QUERY_A3D2								: return  "A3DERROR_FAILED_QUERY_A3D2";
 case A3DERROR_FAILED_FILE_OPEN								: return  "A3DERROR_FAILED_FILE_OPEN";
 case A3DERROR_FAILED_CREATE_SOUNDBUFFER						: return  "A3DERROR_FAILED_CREATE_SOUNDBUFFER";
 case A3DERROR_FAILED_QUERY_3DINTERFACE						: return  "A3DERROR_FAILED_QUERY_3DINTERFACE";
 case A3DERROR_FAILED_LOCK_BUFFER								: return  "A3DERROR_FAILED_LOCK_BUFFER";
 case A3DERROR_FAILED_UNLOCK_BUFFER							: return  "A3DERROR_FAILED_UNLOCK_BUFFER";
 case A3DERROR_UNRECOGNIZED_FORMAT							: return  "A3DERROR_UNRECOGNIZED_FORMAT";
 case A3DERROR_NO_WAVE_DATA									: return  "A3DERROR_NO_WAVE_DATA";
 case A3DERROR_UNKNOWN_PLAYMODE								: return  "A3DERROR_UNKNOWN_PLAYMODE";
 case A3DERROR_FAILED_PLAY									: return  "A3DERROR_FAILED_PLAY";
 case A3DERROR_FAILED_STOP									: return  "A3DERROR_FAILED_STOP";
 case A3DERROR_NEEDS_FORMAT_INFORMATION						: return  "A3DERROR_NEEDS_FORMAT_INFORMATION";
 case A3DERROR_FAILED_ALLOCATE_WAVEDATA						: return  "A3DERROR_FAILED_ALLOCATE_WAVEDATA";
 case A3DERROR_NOT_VALID_SOURCE								: return  "A3DERROR_NOT_VALID_SOURCE";
 case A3DERROR_FAILED_DUPLICATION								: return  "A3DERROR_FAILED_DUPLICATION";
 case A3DERROR_FAILED_INIT									: return  "A3DERROR_FAILED_INIT";
 case A3DERROR_FAILED_SETCOOPERATIVE_LEVEL					: return  "A3DERROR_FAILED_SETCOOPERATIVE_LEVEL";
 case A3DERROR_FAILED_INIT_QUERIED_INTERFACE					: return  "A3DERROR_FAILED_INIT_QUERIED_INTERFACE";
 case A3DERROR_GEOMETRY_INPUT_OUTSIDE_BEGIN_END_BLOCK			: return  "A3DERROR_GEOMETRY_INPUT_OUTSIDE_BEGIN_END_BLOCK";
 case A3DERROR_INVALID_NORMAL									: return  "A3DERROR_INVALID_NORMAL";
 case A3DERROR_END_BEFORE_VALID_BEGIN_BLOCK					: return  "A3DERROR_END_BEFORE_VALID_BEGIN_BLOCK";
 case A3DERROR_INVALID_BEGIN_MODE								: return  "A3DERROR_INVALID_BEGIN_MODE";
 case A3DERROR_INVALID_ARGUMENT								: return  "A3DERROR_INVALID_ARGUMENT";
 case A3DERROR_INVALID_INDEX									: return  "A3DERROR_INVALID_INDEX";
 case A3DERROR_INVALID_VERTEX_INDEX							: return  "A3DERROR_INVALID_VERTEX_INDEX";
 case A3DERROR_INVALID_PRIMITIVE_INDEX						: return  "A3DERROR_INVALID_PRIMITIVE_INDEX";
 case A3DERROR_MIXING_2D_AND_3D_MODES							: return  "A3DERROR_MIXING_2D_AND_3D_MODES";
 case A3DERROR_2DWALL_REQUIRES_EXACTLY_ONE_LINE				: return  "A3DERROR_2DWALL_REQUIRES_EXACTLY_ONE_LINE";
 case A3DERROR_NO_PRIMITIVES_DEFINED							: return  "A3DERROR_NO_PRIMITIVES_DEFINED";
 case A3DERROR_PRIMITIVES_NON_PLANAR							: return  "A3DERROR_PRIMITIVES_NON_PLANAR";
 case A3DERROR_PRIMITIVES_OVERLAPPING							: return  "A3DERROR_PRIMITIVES_OVERLAPPING";
 case A3DERROR_PRIMITIVES_NOT_ADJACENT						: return  "A3DERROR_PRIMITIVES_NOT_ADJACENT";
 case A3DERROR_OBJECT_NOT_FOUND								: return  "A3DERROR_OBJECT_NOT_FOUND";
 case A3DERROR_ROOM_HAS_NO_SHELL_WALLS						: return  "A3DERROR_ROOM_HAS_NO_SHELL_WALLS";
 case A3DERROR_WALLS_DO_NOT_ENCLOSE_ROOM						: return  "A3DERROR_WALLS_DO_NOT_ENCLOSE_ROOM";
 case A3DERROR_INVALID_WALL									: return  "A3DERROR_INVALID_WALL";
 case A3DERROR_ROOM_HAS_LESS_THAN_4SHELL_WALLS				: return  "A3DERROR_ROOM_HAS_LESS_THAN_4SHELL_WALLS";
 case A3DERROR_ROOM_HAS_LESS_THAN_3UNIQUE_NORMALS				: return  "A3DERROR_ROOM_HAS_LESS_THAN_3UNIQUE_NORMALS";
 case A3DERROR_INTERSECTING_WALL_EDGES						: return  "A3DERROR_INTERSECTING_WALL_EDGES";
 case A3DERROR_INVALID_ROOM									: return  "A3DERROR_INVALID_ROOM";
 case A3DERROR_SCENE_HAS_ROOMS_INSIDE_ANOTHER_ROOMS			: return  "A3DERROR_SCENE_HAS_ROOMS_INSIDE_ANOTHER_ROOMS";
 case A3DERROR_SCENE_HAS_OVERLAPPING_STATIC_ROOMS				: return  "A3DERROR_SCENE_HAS_OVERLAPPING_STATIC_ROOMS";
 case A3DERROR_DYNAMIC_OBJ_UNSUPPORTED						: return  "A3DERROR_DYNAMIC_OBJ_UNSUPPORTED";
 case A3DERROR_DIR_AND_UP_VECTORS_NOT_PERPENDICULAR			: return  "A3DERROR_DIR_AND_UP_VECTORS_NOT_PERPENDICULAR";
 case A3DERROR_INVALID_ROOM_INDEX								: return  "A3DERROR_INVALID_ROOM_INDEX";
 case A3DERROR_INVALID_WALL_INDEX								: return  "A3DERROR_INVALID_WALL_INDEX";
 case A3DERROR_SCENE_INVALID									: return  "A3DERROR_SCENE_INVALID";
 case A3DERROR_UNIMPLEMENTED_FUNCTION							: return  "A3DERROR_UNIMPLEMENTED_FUNCTION";
 case A3DERROR_NO_ROOMS_IN_SCENE								: return  "A3DERROR_NO_ROOMS_IN_SCENE";
 case A3DERROR_2D_GEOMETRY_UNIMPLEMENTED						: return  "A3DERROR_2D_GEOMETRY_UNIMPLEMENTED";
 case A3DERROR_OPENING_NOT_WALL_COPLANAR						: return  "A3DERROR_OPENING_NOT_WALL_COPLANAR";
 case A3DERROR_OPENING_NOT_VALID								: return  "A3DERROR_OPENING_NOT_VALID";
 case A3DERROR_INVALID_OPENING_INDEX							: return  "A3DERROR_INVALID_OPENING_INDEX";
 case A3DERROR_FEATURE_NOT_REQUESTED							: return  "A3DERROR_FEATURE_NOT_REQUESTED";
 case A3DERROR_FEATURE_NOT_SUPPORTED							: return  "A3DERROR_FEATURE_NOT_SUPPORTED";
 case A3DERROR_FUNCTION_NOT_VALID_BEFORE_INIT					: return  "A3DERROR_FUNCTION_NOT_VALID_BEFORE_INIT";
 case A3DERROR_INVALID_NUMBER_OF_CHANNELS  					: return  "A3DERROR_INVALID_NUMBER_OF_CHANNELS";
 case A3DERROR_SOURCE_IN_NATIVE_MODE      					: return  "A3DERROR_SOURCE_IN_NATIVE_MODE";
 case A3DERROR_SOURCE_IN_A3D_MODE 	      					: return  "A3DERROR_SOURCE_IN_A3D_MODE";
 case A3DERROR_BBOX_CANNOT_ENABLE_AFTER_BEGIN_LIST_CALL		: return  "A3DERROR_BBOX_CANNOT_ENABLE_AFTER_BEGIN_LIST_CALL";
 case A3DERROR_CANNOT_CHANGE_FORMAT_FOR_ALLOCATED_BUFFER      : return  "A3DERROR_CANNOT_CHANGE_FORMAT_FOR_ALLOCATED_BUFFER";
 case A3DERROR_FAILED_QUERY_DIRECTSOUNDNOTIFY					: return "A3DERROR_FAILED_QUERY_DIRECTSOUNDNOTIFY";
 case A3DERROR_DIRECTSOUNDNOTIFY_FAILED						: return  "A3DERROR_DIRECTSOUNDNOTIFY_FAILED";
 case A3DERROR_RESOURCE_MANAGER_ALWAYS_ON						: return  "A3DERROR_RESOURCE_MANAGER_ALWAYS_ON";
 case A3DERROR_CLOSED_LIST_CANNOT_BE_CHANGED					: return  "A3DERROR_CLOSED_LIST_CANNOT_BE_CHANGED";
 case A3DERROR_END_CALLED_BEFORE_BEGIN						: return  "A3DERROR_END_CALLED_BEFORE_BEGIN";
 case A3DERROR_UNMANAGED_BUFFER								: return  "A3DERROR_UNMANAGED_BUFFER";
 case A3DERROR_COORD_SYSTEM_CAN_ONLY_BE_SET_ONCE				: return  "A3DERROR_COORD_SYSTEM_CAN_ONLY_BE_SET_ONCE";
 case A3DERROR_BUFFER_IN_SOFTWARE								: return  "A3DERROR_BUFFER_IN_SOFTWARE";
 case A3DERROR_INITIAL_PARAMETERS_NOT_SET						: return  "A3DERROR_INITIAL_PARAMETERS_NOT_SET";
 case A3DERROR_INCORRECT_FORMAT_SPECIFIED						: return  "A3DERROR_INCORRECT_FORMAT_SPECIFIED";
 case A3DERROR_NO_SOUND_BUFFERS_CREATED						: return  "A3DERROR_NO_SOUND_BUFFERS_CREATED";
 case A3DERROR_SOURCE_IN_USE									: return  "A3DERROR_SOURCE_IN_USE";
 case A3DERROR_STREAMING_BUFFER_LENGTH						: return  "A3DERROR_STREAMING_BUFFER_LENGTH";
 case A3DERROR_STREAMING_PRIORITY								: return  "A3DERROR_STREAMING_PRIORITY";
 case A3DERROR_CANT_DUPLICATE_STREAM_SRC						: return  "A3DERROR_CANT_DUPLICATE_STREAM_SRC";
 case A3DERROR_CANT_INSTANTIATE_MORE_ONE_INSTANCE				: return  "A3DERROR_CANT_INSTANTIATE_MORE_ONE_INSTANCE";
 case A3DERROR_CANT_SET_TIME_POSITION_FOR_AC3_STREAM			: return  "A3DERROR_CANT_SET_TIME_POSITION_FOR_AC3_STREAM";
 case A3DERROR_HARDWARE_AC3_OBJECT_DOES_NOT_IMPLEMENT_THIS_MEMBER		: return  "A3DERROR_HARDWARE_AC3_OBJECT_DOES_NOT_IMPLEMENT_THIS_MEMBER";
 case A3DERROR_NO_MORE_THAN_ONE_AC3_SRC_AT_ONCE				: return  "A3DERROR_NO_MORE_THAN_ONE_AC3_SRC_AT_ONCE";
 case A3DERROR_ENCODED_SOURCE_TYPE_CANNOT_BE_TIME_SEEKED		: return  "A3DERROR_ENCODED_SOURCE_TYPE_CANNOT_BE_TIME_SEEKED";
 case A3DERROR_INVALID_AC3_KEY								: return  "A3DERROR_INVALID_AC3_KEY";
 case A3DERROR_MUST_UNLOCK_SOFTAC3_BEFORE_USE					: return  "A3DERROR_MUST_UNLOCK_SOFTAC3_BEFORE_USE";
 case A3DERROR_REFLECTIONS_NOT_ENABLED						: return  "A3DERROR_REFLECTIONS_NOT_ENABLED";
 case A3DERROR_FEATURE_UNSUPPORTED_BY_DECODER					: return  "A3DERROR_FEATURE_UNSUPPORTED_BY_DECODER";
 case A3DERROR_AC3_FALLBACK_REQUIRES_DVD_DRIVE				: return  "A3DERROR_AC3_FALLBACK_REQUIRES_DVD_DRIVE";
 case E_NOINTERFACE:       return "DSERR_NOINTERFACE";
 case E_INVALIDARG:  return "DSERR_INVALIDARGUMENT";

 default:	 return "Unknown error";}
}

void ParseError(LPSTR em,HRESULT err)
{
 wsprintf(logtt, "A3D3 error: %s %s: %Xh\n",em,TranslateA3DError(err),err); 
 PrintLog(logtt);
}
