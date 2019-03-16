#define _AUDIO_
#include "audio.h"

#include <windows.h>
#include <math.h>

#define version 0x0008

char logtt[1024];
DWORD LastMixedTime;
int LastBlockTime;
LONG _master_vol;
int prev_vol = -1;

void PrintLog(LPSTR l)
{
    /*
	DWORD w;
	
	if (l[strlen(l)-1]==0x0A) {
		BYTE b = 0x0D;
		WriteFile(hlog, l, strlen(l)-1, &w, NULL);
		WriteFile(hlog, &b, 1, &w, NULL);
		b = 0x0A;
		WriteFile(hlog, &b, 1, &w, NULL);
	} else
		WriteFile(hlog, l, strlen(l), &w, NULL);
    */
}



void Audio_MixSound(int DestAddr, int SrcAddr, int MixLen, int LVolume, int RVolume) 
{
_asm {
       mov      edi, DestAddr                    
       mov      ecx, MixLen         
       mov      esi, SrcAddr
       sal      LVolume,15 
       sal      RVolume,15
     }
      
SOUND_CYCLE :

_asm {                
       movsx    eax, word ptr [esi]
       sal      eax, 1
       imul     LVolume
       //sar      eax, 16
       mov      bx, word ptr [edi]
               
       add      dx, bx
       jo       LEFT_CHECK_OVERFLOW
       mov      word ptr [edi], dx
       jmp      CYCLE_RIGHT
 }
LEFT_CHECK_OVERFLOW :
__asm {
       cmp      bx, 0
       js       LEFT_MAX_NEGATIVE
       mov      ax, 32767
       mov      word ptr [edi], ax
       jmp      CYCLE_RIGHT
}
LEFT_MAX_NEGATIVE :
__asm  mov      word ptr [edi], -32767
                           
      


CYCLE_RIGHT :
__asm {
       movsx    eax, word ptr [esi]
       sal      eax, 1
       imul     dword ptr RVolume
       //sar      eax, 16
       mov      bx, word ptr [edi+2]
               
       add      dx, bx
       jo       RIGHT_CHECK_OVERFLOW
       mov      word ptr [edi+2], dx
       jmp      CYCLE_CONTINUE
} 
RIGHT_CHECK_OVERFLOW :
__asm {
       cmp      bx, 0
       js       RIGHT_MAX_NEGATIVE
       mov      word ptr [edi+2], 32767
       jmp      CYCLE_CONTINUE
}
RIGHT_MAX_NEGATIVE :
__asm  mov      word ptr [edi+2], -32767
      
CYCLE_CONTINUE :  
__asm {
       add      edi, 4
       add      esi, 2
       dec      ecx
       jnz      SOUND_CYCLE
}
}




void Audio_MixSoundFade(int DestAddr, int SrcAddr, int MixLen, int LVolume, int RVolume, int LVolume0, int RVolume0)
{
_asm {
       mov      edi, DestAddr                    
       mov      ecx, MixLen         
       mov      esi, SrcAddr

       mov      eax, LVolume
       sub      eax, LVolume0
       sal      eax, 15
       cdq
       idiv     ecx
       mov      LVolume, eax

       mov      eax, RVolume
       sub      eax, RVolume0
       sal      eax, 15
       cdq
       idiv     ecx
       mov      RVolume, eax        

       sal      LVolume0,15 
       sal      RVolume0,15
     }
      
SOUND_CYCLE :

_asm {                
       mov      ebx, LVolume
       add      LVolume0, ebx
       movsx    eax, word ptr [esi]       
       sal      eax, 1
       imul     LVolume0
       //sar      eax, 16
       mov      bx, word ptr [edi]
               
       add      dx, bx
       jo       LEFT_CHECK_OVERFLOW
       mov      word ptr [edi], dx
       jmp      CYCLE_RIGHT
 }
LEFT_CHECK_OVERFLOW :
__asm {
       cmp      bx, 0
       js       LEFT_MAX_NEGATIVE
       mov      ax, 32767
       mov      word ptr [edi], ax
       jmp      CYCLE_RIGHT
}
LEFT_MAX_NEGATIVE :
__asm  mov      word ptr [edi], -32767
                           
      


CYCLE_RIGHT :
__asm {
       mov      ebx, RVolume
       add      RVolume0, ebx
       movsx    eax, word ptr [esi]
       
       sal      eax, 1
       imul     dword ptr RVolume0
       //sar      eax, 16
       mov      bx, word ptr [edi+2]
               
       add      dx, bx
       jo       RIGHT_CHECK_OVERFLOW
       mov      word ptr [edi+2], dx
       jmp      CYCLE_CONTINUE
} 
RIGHT_CHECK_OVERFLOW :
__asm {
       cmp      bx, 0
       js       RIGHT_MAX_NEGATIVE
       mov      word ptr [edi+2], 32767
       jmp      CYCLE_CONTINUE
}
RIGHT_MAX_NEGATIVE :
__asm  mov      word ptr [edi+2], -32767
      
CYCLE_CONTINUE :  
__asm {
       add      edi, 4
       add      esi, 2
       dec      ecx
       jnz      SOUND_CYCLE
}
}








void Audio_SetMasterVolume(int mastervolume)  //0..255
{    
    if (prev_vol == mastervolume) return;
    prev_vol = mastervolume;
    if (lpdsPrimary)
       lpdsPrimary->SetVolume((LONG)-(exp(log(200)*(255-mastervolume)/255)-1)*10);
}










void CalcLRVolumes(int v0, int x, int y, int z, float radius, int &lv, int &rv)
{
   if (x==0) {
    lv = v0*180;
    rv = v0*180;
    return;  }

   v0*=200;
   x-=xCamera;
   y-=yCamera;
   z-=zCamera;
   float xx = (float)x * cosa + (float)z * sina;
   float yy = (float)y;
   float zz = (float)fabs((float)z * cosa - (float)x * sina);

   float xa = (float)fabs(xx);
   float l = 0.8f;
   float r = 0.8f;
   float d = (float)sqrt( xx*xx + yy*yy + zz*zz) - MIN_RADIUS;
   if (d<0) d=0;   
   float k;
   float _fadel = fadel * radius;   
   if (d>_fadel) k = 0;
          else  k = 1/(float)exp(d/_fadel*6);   // exponent fade

   /*
   if (d<=0) k=1.f;    
        else k = (fadel/16.f) / ( (fadel/16.f) + d);
   
   if (d>fadel*0.75f) {
	   d-=fadel*0.75f;
	   k = k * (fadel*0.25f - d) / (fadel*0.25f);
	   if (k<0) k=0.f;
	 }
     */
   

   float fi = (float)atan2(xa, zz);
   r = 0.7f + 0.3f * fi / (3.141593f/2.f);
   l = 0.7f - 0.6f * fi / (3.141593f/2.f);

   if (xx>0) { lv=(int)(v0*l*k); rv=(int)(v0*r*k); }
        else { lv=(int)(v0*r*k); rv=(int)(v0*l*k); }
}





void Audio_DoMixAmbient(AMBIENT &ambient)
{
    if (ambient.lpData==0) return;
    int iMixLen,srcofs;    
	int v = (32000 * ambient.volume * ambient.avolume) / 256 / 256;

    if( ambient.iPosition + 2048*1 >= ambient.iLength )
         iMixLen = (ambient.iLength - ambient.iPosition)>>1;
       else iMixLen = 1024*1;

    srcofs = (int) ambient.lpData + ambient.iPosition;
    Audio_MixSound((int)lpSoundBuffer, srcofs, iMixLen, v, -v);

    if (ambient.iPosition + 2048*1 >= ambient.iLength ) 
       ambient.iPosition=0; else ambient.iPosition += 2048*1;
    
    if (iMixLen<1024*1) {
      Audio_MixSound((int)lpSoundBuffer + iMixLen*4,
                     (int)ambient.lpData, 1024*1-iMixLen, v, -v);
      ambient.iPosition+=(1024*1-iMixLen)*2;
    }
}


void Audio_MixAmbient()
{
	Audio_DoMixAmbient(ambient);
	if (ambient.volume<256) ambient.volume = min(ambient.volume+16, 256);

	if (ambient2.volume) Audio_DoMixAmbient(ambient2);
	if (ambient2.volume>0) ambient2.volume = max(ambient2.volume-16, 0);    
}










void Audio_MixChannels()
{
    int iMixLen;
    int LV, RV;

    for( int i = 0; i < MAX_CHANNEL; i++ )        
      if( channel[ i ].status ) {
/*
         if (channel[i].iMixPos > 2048*1) {
              channel[i].iMixPos-=2048*1;
              continue;
         }*/

         CalcLRVolumes(channel[i].volume, channel[i].x, channel[i].y, channel[i].z, channel[i].radius, LV, RV);

         if (channel[i].Lv0==-1)
         { channel[i].Lv0 = LV; channel[i].Rv0 = RV; }

         channel[i].iMixPos = 0;
         int BufLen = 2048*1 - channel[i].iMixPos;
         int BufOfs = channel[i].iMixPos;
         
          
         if( channel[i].iPosition + BufLen > channel[i].iLoopB ) 
             if (!channel[i].loop) {
                 channel[i].iLoopB = channel[i].iLength;
                 channel[i].id = 0;
             }

         if( channel[i].iPosition + BufLen > channel[i].iLoopB )
             iMixLen = (channel[i].iLoopB - channel[i].iPosition)>>1; else
             iMixLen = BufLen>>1;
         
         if (LV || RV) {
           if (channel[i].Lv0!=LV ||
               channel[i].Rv0!=RV)
           Audio_MixSoundFade((int)lpSoundBuffer + BufOfs*2, (int)channel[i].lpData+channel[i].iPosition, iMixLen, LV, RV, channel[i].Lv0, channel[i].Rv0);
           else
           Audio_MixSound((int)lpSoundBuffer + BufOfs*2, (int)channel[i].lpData+channel[i].iPosition, iMixLen, LV, RV);
         }

         channel[i].Lv0 = LV;
         channel[i].Rv0 = RV;

         if (channel[i].iPosition + BufLen < channel[i].iLoopB ) 
           channel[i].iPosition += BufLen; else
           if (channel[i].loop) {
             channel[i].loop = 0;
             channel[i].iPosition = channel[i].iLoopA; 
             /*
             char t[128];
             wsprintf(t, "Loop: %d-%d\n", channel[i].iLoopA, channel[i].iLoopB);
             PrintLog(t);
             */
           } else
           {
             channel[i].status = 0; 
             continue;
           }

         if (iMixLen<(BufLen>>1)) {
           if (LV || RV)
            Audio_MixSound((int)lpSoundBuffer + BufOfs*2 + iMixLen*4,
                           (int)channel[i].lpData+channel[i].iPosition, 
                           (BufLen>>1)-iMixLen, LV, RV);
            channel[i].iPosition+=((BufLen>>1)-iMixLen)*2;
         }         
         
      }
}


void Audio_Restore(void)
{
   if (!iSoundActive) return;
   
   LastMixedTime = timeGetTime();
   lpdsWork->Stop();	   
   lpdsWork->Restore();
   HRESULT hres = lpdsWork->Play( 0, 0, DSBPLAY_LOOPING );   
   if (hres != DS_OK) AudioNeedRestore = TRUE;
                 else AudioNeedRestore = FALSE;
   
   if (!AudioNeedRestore)
	   PrintLog("Audio restored.\n");   
}

int ProcessAudio()
{
   LPVOID lpStart1;
   LPVOID lpStart2;
   DWORD  len1, len2;
   HRESULT hres;
   static int PrevBlock = 1;            

   if (AudioNeedRestore) Audio_Restore();
   if (AudioNeedRestore) return 1;

   hres = lpdsWork->GetCurrentPosition( &len1, &dwWritePos );
   hres = lpdsWork->GetCurrentPosition( &len2, &dwWritePos );
   if( hres != DS_OK ) { AudioNeedRestore = TRUE; return 0; }

   if ( (len1>len2) || (len1<len2+16) ) 
       hres = lpdsWork->GetCurrentPosition( &len2, &dwWritePos );   
   
   if (len1==len2) {
	   Sleep(2);
	   lpdsWork->GetCurrentPosition( &len2, &dwWritePos );
       if (!len2) { AudioNeedRestore = TRUE; return 0; }
   }

   dwPlayPos = len2;

   if (timeGetTime() > LastMixedTime + 2048)  { 
	   LastMixedTime = timeGetTime();
	   AudioNeedRestore = TRUE; return 0; 
   }

   int CurBlock = dwPlayPos / (4096*1);
   if (CurBlock==PrevBlock) return 1;

   //if( (int)dwPlayPos < CurBlock*4096*1 + 2) return 1; // it's no time to put info

   LastMixedTime = timeGetTime();
   int MixOffs = dwPlayPos - CurBlock*4096*1;
   LastBlockTime = LastMixedTime - (MixOffs/4)/10;

   FillMemory( lpSoundBuffer, MAX_BUFFER_LENGTH*2, 0 );
   Audio_MixChannels();
   Audio_MixAmbient();

   PrevBlock = CurBlock; 
   int NextBlock = (CurBlock + 1) % iBufferLength;
   hres = lpdsWork->Lock(NextBlock*4096*1, 4096*1, &lpStart1, &len1, &lpStart2, &len2, 0 );
   if( hres != DS_OK ) {	     	     
         return 0;      
		 }
   
   CopyMemory( lpStart1, lpSoundBuffer, 4096*1);
         
   hres = lpdsWork->Unlock( lpStart1, len1, lpStart2, len2 );
   if( hres != DS_OK ) 
         return 0;         
   
   return 1;
}



BOOL CALLBACK EnumerateSoundDevice( GUID* lpGuid, LPSTR lpstrDescription, LPSTR lpstrModule, LPVOID lpContext)
{
   if( lpGuid == NULL )
       if( !iTotalSoundDevices )
           return TRUE;
       else
           return FALSE;   

   wsprintf(logtt,"Device%d: ",iTotalSoundDevices);
   PrintLog(logtt);
   PrintLog(lpstrDescription);
   PrintLog("/");
   PrintLog(lpstrModule);
   PrintLog("\n");
   

   CopyMemory( &sdd[iTotalSoundDevices].Guid, lpGuid, sizeof( GUID ) );

   iTotalSoundDevices++;

   return TRUE;
}


DWORD WINAPI ProcessAudioThread (LPVOID ptr)
{
   for (;;) {
    if (iSoundActive)
       ProcessAudio();
    Sleep(25);
   }
   return 0;
}


int Audio_GetVersion()
{
	return version;
}


int InitDirectSound( HWND hwnd)
{
   PrintLog("\n");
   PrintLog("==Init Direct Sound==\n");
 
   wsprintf(logtt,"Software mixer version %d.%d\n", version>>16, version & 0xFFFF);
   PrintLog(logtt);

   HRESULT hres;
   iTotalSoundDevices = 0;   

   lpSoundBuffer = (char*) _SoundBufferData;
   if( !lpSoundBuffer )
      return 0;
   PrintLog("Back Sound Buffer created.\n");

   /*
   for( int i = 0; i < MAX_SOUND_DEVICE; i++ )       
      sdd[i].DSC.dwSize = sizeof( DSCAPS );      

   hres = DirectSoundEnumerate( (LPDSENUMCALLBACK)EnumerateSoundDevice, NULL);
   if( hres != DS_OK ) {
      wsprintf(logtt, "DirectSoundEnumerate Error: %Xh\n", hres);
	  PrintLog(logtt);
      return 0;
   }
   PrintLog("DirectSoundEnumerate: Ok\n");

   iTotal16SD = 0;
   for( i = 0; i < iTotalSoundDevices; i++ ) {
      LPDIRECTSOUND lpds;
      if( DirectSoundCreate( &sdd[i].Guid, &lpds, NULL ) != DS_OK ) continue;

      if( lpds->GetCaps( &sdd[i].DSC ) != DS_OK ) continue;

      if( sdd[i].DSC.dwFlags & (DSCAPS_PRIMARY16BIT | DSCAPS_PRIMARYSTEREO | DSCAPS_SECONDARY16BIT | DSCAPS_SECONDARYSTEREO ) ) {
         sdd[i].status = 1;
         iTotal16SD++;
		 wsprintf(logtt,"Acceptable device: %d\n",i);
		 PrintLog(logtt);
      }

      lpds->Release();
   }

   if (!iTotal16SD) return 0;
   iCurrentDriver = 0;
   while( !sdd[iCurrentDriver].status )
	   iCurrentDriver++;
   
   wsprintf(logtt,"Device selected  : %d\n", iCurrentDriver);
   PrintLog(logtt);

*/
   hres = DirectSoundCreate(NULL, &lpDS, NULL );   //&sdd[iCurrentDriver].Guid
   if( (hres != DS_OK) || (!lpDS) ) {
	  wsprintf(logtt, "DirectSoundCreate Error: %Xh\n", hres);
	  PrintLog(logtt);
      return 0;
   }
   PrintLog("DirectSoundCreate: Ok\n");


   PrimaryMode = TRUE;
   PrintLog("Attempting to set WRITEPRIMARY CooperativeLevel:\n");
   hres = lpDS->SetCooperativeLevel( hwnd, DSSCL_WRITEPRIMARY );
   if (hres != DS_OK) {	  
	  wsprintf(logtt, "SetCooperativeLevel Error: %Xh\n", hres);
	  PrintLog(logtt);
      PrimaryMode = FALSE;
   } else
   PrintLog("Set Cooperative  : Ok\n");

   //PrimaryMode = FALSE;

   if (!PrimaryMode) {
	   PrintLog("Attempting to set EXCLUSIVE CooperativeLevel:\n");
	   hres = lpDS->SetCooperativeLevel( hwnd, DSSCL_EXCLUSIVE);
       if (hres != DS_OK) {	     
	     wsprintf(logtt, "==>>SetCooperativeLevel Error: %Xh\n", hres);
	     PrintLog(logtt);
		 return 0;
	   }   
       PrintLog("Set Cooperative  : Ok\n");
   }


/*======= creating primary buffer ==============*/
   CopyMemory( &WaveFormat, &wf, sizeof( WAVEFORMATEX ) );
   
   DSBUFFERDESC dsbd;
   dsbd.dwSize = sizeof( DSBUFFERDESC );
   dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
   dsbd.dwBufferBytes = 0;
   dsbd.lpwfxFormat = NULL; 
   dsbd.dwReserved = 0;
   
   hres = lpDS->CreateSoundBuffer( &dsbd, &lpdsPrimary, NULL );
   if( hres != DS_OK ) {
	  wsprintf(logtt, "==>>CreatePrimarySoundBuffer Error: %Xh\n", hres);
	  PrintLog(logtt);
      return 0;
   }   
   PrintLog("CreateSoundBuffer: Ok (Primary)\n");   
   lpdsPrimary->GetVolume(&_master_vol);
   lpdsWork = lpdsPrimary;

   hres = lpdsPrimary->SetFormat( &wf );
   if( hres != DS_OK ) {
	  wsprintf(logtt, "SetFormat Error: %Xh\n", hres);
	  PrintLog(logtt);
      return 0;
   }   
   PrintLog("SetFormat        : Ok\n");   


   if (PrimaryMode) goto SKIPSECONDARY;

// ========= creating secondary ================//
   dsbd.dwSize = sizeof( DSBUFFERDESC );
   dsbd.dwFlags = 0;
   dsbd.dwBufferBytes = 2*8192;
   dsbd.lpwfxFormat = &wf; 
   dsbd.dwReserved = 0;
   
   hres = lpDS->CreateSoundBuffer( &dsbd, &lpdsSecondary, NULL );
   if( hres != DS_OK ) {
	  wsprintf(logtt, "CreateSecondarySoundBuffer Error: %Xh\n", hres);
	  PrintLog(logtt);
      return 0;
   }   
   PrintLog("CreateSoundBuffer: Ok (Secondary)\n");
   lpdsWork = lpdsSecondary;


SKIPSECONDARY:
   
   
   DSBCAPS dsbc;
   dsbc.dwSize = sizeof( DSBCAPS );
   lpdsWork->GetCaps( &dsbc );
   iBufferLength = dsbc.dwBufferBytes;
   iBufferLength /= (8192/2);
         
   hres = lpdsWork->Play( 0, 0, DSBPLAY_LOOPING );
   if( hres != DS_OK ) {
      wsprintf(logtt, "Play Error: %Xh\n", hres);
	  PrintLog(logtt);
      return 0;   
   }
   PrintLog("Play             : Ok\n");
   LastMixedTime = timeGetTime();
   
   iSoundActive = 1;
   FillMemory( channel, sizeof( CHANNEL )*MAX_CHANNEL, 0 );
   ambient.iLength = 0;


   hAudioThread = CreateThread(NULL, 0, ProcessAudioThread, NULL, 0, &AudioTId);
   SetThreadPriority(hAudioThread, THREAD_PRIORITY_HIGHEST);
   PrintLog("Direct Sound activated.\n");
   
   return 1;
}



















BOOL InitAudioSystem(HWND hw, HANDLE hl, BOOL shit, void* shit2)
{   
   hwndApp = hw;
   //hlog = hl;
   fadel = 256*50;   
   wf.wFormatTag      = WAVE_FORMAT_PCM;
   wf.nChannels       = 2;
   wf.nSamplesPerSec  = 11025*2;
   wf.nAvgBytesPerSec = 44100*2;
   wf.nBlockAlign     = 4;
   wf.wBitsPerSample  = 16;
   wf.cbSize          = 0;
   if( !InitDirectSound( hwndApp ) ) {
      PrintLog("Sound System failed\n");
      return FALSE;
   }
   return TRUE;
}


void  AudioStop()
{		
  FillMemory(&ambient, sizeof(ambient), 0);
  FillMemory(&ambient2, sizeof(ambient2), 0);
  FillMemory(channel, sizeof( CHANNEL )*MAX_CHANNEL, 0 );
  AudioNeedRestore = TRUE;
}


void Audio_Shutdown()
{
   if (!iSoundActive) return;
   
   TerminateThread(hAudioThread ,0);
   if (!PrimaryMode)
      lpdsSecondary->Release();   
   lpdsPrimary->SetVolume(_master_vol);
   lpdsPrimary->Release();
   
   lpDS->Release();   
}


void  StopVoice(TSFX *voice, int id)
{
   if (!voice) return;
   if (!iSoundActive) return;
   if (!id) return;
   if (voice->lpData == 0) return;   
   
   
   for (int i = 0; i < MAX_CHANNEL; i++)
     if (channel[i].status) 
       if (channel[i].id == id)
         if (channel[i].lpData == voice->lpData) 
         {                                  
                 channel[i].status = 0;                 
                 return;
         }
}



int ReleaseChannel()
{
  int min_volume = 128000; 
  int min_c = 0;
  int lv, rv;
  for (int i = 0; i < MAX_CHANNEL; i++)     
  {
    CalcLRVolumes(channel[i].volume, channel[i].x, channel[i].y, channel[i].z, channel[i].radius, lv, rv);
    if (min_volume>lv+rv) { min_volume = lv+rv; min_c = i; }
  }

  return min_c;
}


void  AddVoice3dv(TSFX *voice, float cx, float cy, float cz, int vol, int id, int env, float radius)
{  
   if (!voice) return;
   if (!iSoundActive) return;
   if (voice->lpData == 0) return;
   int i;

   if (radius==0) radius = 1;

   if ( (cx>0) || (cz>0) )
   {
   float dist = (float)sqrt(
                     (xCamera-cx)*(xCamera-cx)+
                     (yCamera-cy)*(yCamera-cy)+
                     (zCamera-cz)*(zCamera-cz));
   if (dist>fadel*radius+256) return;
   }

   if (voice->loopbegin)
   if (id)
   for (i = 0; i < MAX_CHANNEL; i++)
     if (channel[i].status) 
         if (channel[i].id == id)
             if (channel[i].lpData == voice->lpData) {
                 channel[i].x = (int)cx;
                 channel[i].y = (int)cy;
                 channel[i].z = (int)cz;
                 if (channel[i].iPosition+2048*6 > voice->loopbegin)
                 if (channel[i].iPosition < voice->loopend)                 
                     channel[i].loop = 1;
                 return;
             }

   i = -1;

   for (int ii = 0; ii < MAX_CHANNEL; ii++)
     if (!channel[ii].status ) 
     {
         i = ii; break;
     }

   if (i == -1)
       if (id>=100 && id<=110)
           i = ReleaseChannel();

   if (i==-1) return;

   channel[i].iLength = voice->length;         
   channel[i].iLoopA  = voice->loopbegin;
   channel[i].iLoopB  = voice->loopend;         
   channel[i].lpData  = voice->lpData;
   channel[i].iPosition = 0;

   channel[i].x = (int)cx;
   channel[i].y = (int)cy;
   channel[i].z = (int)cz;

   channel[i].id     = id;
   channel[i].status = 1;
   channel[i].volume = vol;
   channel[i].radius = radius;
   channel[i].loop   = 0;
   channel[i].Lv0 = channel[i].Rv0 = -1;
   int ofs = (timeGetTime() - LastBlockTime);// - delay + 100;
   if (ofs <  0) ofs =  0;
   if (ofs > 90) ofs = 90;
       //wsprintf(logtt, "%d\n", ofs);
       //PrintLog(logtt);
   channel[i].iMixPos = 0;//ofs*20;   
}




void SetAmbient(int length, short int* lpdata, int av)
{
   if (!iSoundActive) return;   
   if (ambient.lpData == lpdata) return;      
   ambient2 = ambient;
   ambient.iLength = length;
   ambient.lpData = lpdata;
   ambient.iPosition = 0;
   ambient.volume = 0;
   ambient.avolume = av;
}


void AudioSetCameraPos(float cx, float cy, float cz, float ca, float cb, float dist_d, int volume)
{
  xCamera = (int) cx;
  yCamera = (int) cy;
  zCamera = (int) cz;
  alphaCamera = ca;
  cosa = (float)cos(ca);
  sina = (float)sin(ca);
  fadel = dist_d;
  Audio_SetMasterVolume(volume);  
}


void Audio_SetEnvironment(int e, float f)
{	
}


void Audio_SetExtData(int ID, float f1, float f2, float f3)
{	
}