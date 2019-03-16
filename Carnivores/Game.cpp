#include "Hunt.h"

void SetupRes()
{
    if (OptRes==0) { WinW = 320; WinH=240; }
	if (OptRes==1) { WinW = 400; WinH=300; }
	if (OptRes==2) { WinW = 512; WinH=384; }
	if (OptRes==3) { WinW = 640; WinH=480; }
	if (OptRes==4) { WinW = 800; WinH=600; }
	if (OptRes==5) { WinW =1024; WinH=768; }		

}


float GetLandOH(int x, int y)
{
  return (float)(-48 + HMapO[y][x]) * ctHScale;
}


float GetLandOUH(int x, int y)
{
  if (FMap[y][x] & fmReverse)
   return (float)((int)(HMap[y][x+1]+HMap[y+1][x])/2.f)*ctHScale;             
                else
   return (float)((int)(HMap[y][x]+HMap[y+1][x+1])/2.f)*ctHScale;
}



float GetLandUpH(float x, float y)
{ 
   int CX = (int)x / 256;
   int CY = (int)y / 256;
   
   int dx = (int)x % 256;
   int dy = (int)y % 256; 

   int h1 = HMap[CY][CX];
   int h2 = HMap[CY][CX+1];
   int h3 = HMap[CY+1][CX+1];
   int h4 = HMap[CY+1][CX];


   if (FMap[CY][CX] & fmReverse) {
     if (256-dx>dy) h3 = h2+h4-h1;
               else h1 = h2+h4-h3;
   } else {
     if (dx>dy) h4 = h1+h3-h2;
           else h2 = h1+h3-h4;
   }

   float h = (float)
	   (h1   * (256-dx) + h2 * dx) * (256-dy) +
	   (h4   * (256-dx) + h3 * dx) * dy;

   return  h / 256.f / 256.f * ctHScale;	      
}


float GetLandH(float x, float y)
{ 
   int CX = (int)x / 256;
   int CY = (int)y / 256;
   
   int dx = (int)x % 256;
   int dy = (int)y % 256; 

   int h1 = HMap2[CY][CX];
   int h2 = HMap2[CY][CX+1];
   int h3 = HMap2[CY+1][CX+1];
   int h4 = HMap2[CY+1][CX];


   if (FMap[CY][CX] & fmReverse) {
     if (256-dx>dy) h3 = h2+h4-h1;
               else h1 = h2+h4-h3;
   } else {
     if (dx>dy) h4 = h1+h3-h2;
           else h2 = h1+h3-h4;
   }

   float h = (float)
	   (h1   * (256-dx) + h2 * dx) * (256-dy) +
	   (h4   * (256-dx) + h3 * dx) * dy;

   return  (h / 256.f / 256.f - 48) * ctHScale;	      
}




float GetLandQH(float CameraX, float CameraZ)
{
  float h,hh;
  
   h = GetLandH(CameraX, CameraZ);
   hh = GetLandH(CameraX-90.f, CameraZ-90.f); if (hh>h) h=hh;
   hh = GetLandH(CameraX+90.f, CameraZ-90.f); if (hh>h) h=hh;
   hh = GetLandH(CameraX-90.f, CameraZ+90.f); if (hh>h) h=hh; 
   hh = GetLandH(CameraX+90.f, CameraZ+90.f); if (hh>h) h=hh;

   hh = GetLandH(CameraX+128.f, CameraZ); if (hh>h) h=hh;
   hh = GetLandH(CameraX-128.f, CameraZ); if (hh>h) h=hh;
   hh = GetLandH(CameraX, CameraZ+128.f); if (hh>h) h=hh;
   hh = GetLandH(CameraX, CameraZ-128.f); if (hh>h) h=hh;

   int ccx = (int)CameraX / 256;
   int ccz = (int)CameraZ / 256;

   for (int z=-2; z<=2; z++)
    for (int x=-2; x<=2; x++) 
      if (OMap[ccz+z][ccx+x]!=255) {
        int ob = OMap[ccz+z][ccx+x];
        float CR = (float)MObjects[ob].info.Radius - 1.f;
                
        float oz = (ccz+z) * 256.f + 128.f;
        float ox = (ccx+x) * 256.f + 128.f;

        if (MObjects[ob].info.YHi + GetLandOH(ccx+x, ccz+z) < h) continue;
        if (MObjects[ob].info.YHi + GetLandOH(ccx+x, ccz+z) > PlayerY+128) continue;
        if (MObjects[ob].info.YLo + GetLandOH(ccx+x, ccz+z) > PlayerY+256) continue;
        float r = (float) sqrt( (ox-CameraX)*(ox-CameraX) + (oz-CameraZ)*(oz-CameraZ) );
        if (r<CR) 
            h = MObjects[ob].info.YHi + GetLandOH(ccx+x, ccz+z);
   }
  return h;
}


float GetLandHObj(float CameraX, float CameraZ)
{
   float h;   

   h = 0;

   int ccx = (int)CameraX / 256;
   int ccz = (int)CameraZ / 256;

   for (int z=-2; z<=2; z++)
    for (int x=-2; x<=2; x++) 
      if (OMap[ccz+z][ccx+x]!=255) {
        int ob = OMap[ccz+z][ccx+x];
        float CR = (float)MObjects[ob].info.Radius - 1.f;
        
        float oz = (ccz+z) * 256.f + 128.f;
        float ox = (ccx+x) * 256.f + 128.f;

        if (MObjects[ob].info.YHi + GetLandOH(ccx+x, ccz+z) < h) continue;
        if (MObjects[ob].info.YLo + GetLandOH(ccx+x, ccz+z) > PlayerY+256) continue;
        float r = (float) sqrt( (ox-CameraX)*(ox-CameraX) + (oz-CameraZ)*(oz-CameraZ) );
        if (r<CR) 
            h = MObjects[ob].info.YHi + GetLandOH(ccx+x, ccz+z);
   }

  return h;
}


float GetLandQHNoObj(float CameraX, float CameraZ)
{
  float h,hh;
  
   h = GetLandH(CameraX, CameraZ);
   hh = GetLandH(CameraX-90.f, CameraZ-90.f); if (hh>h) h=hh;
   hh = GetLandH(CameraX+90.f, CameraZ-90.f); if (hh>h) h=hh;
   hh = GetLandH(CameraX-90.f, CameraZ+90.f); if (hh>h) h=hh; 
   hh = GetLandH(CameraX+90.f, CameraZ+90.f); if (hh>h) h=hh;

   hh = GetLandH(CameraX+128.f, CameraZ); if (hh>h) h=hh;
   hh = GetLandH(CameraX-128.f, CameraZ); if (hh>h) h=hh;
   hh = GetLandH(CameraX, CameraZ+128.f); if (hh>h) h=hh;
   hh = GetLandH(CameraX, CameraZ-128.f); if (hh>h) h=hh;
   
   return h;
}


void ProcessCommandLine()
{
  for (int a=0; a<__argc; a++) {
     LPSTR s = __argv[a];
     if (strstr(s,"x=")) { PlayerX = (float)atof(&s[2])*256.f; LockLanding = TRUE; }
     if (strstr(s,"y=")) { PlayerZ = (float)atof(&s[2])*256.f; LockLanding = TRUE; }
     if (strstr(s,"/nofullscreen")) FULLSCREEN = FALSE;
     if (strstr(s,"/vmode1")) SetVideoMode(320, 240);
     if (strstr(s,"/vmode2")) SetVideoMode(400, 300);
     if (strstr(s,"/vmode3")) SetVideoMode(512, 384);
     if (strstr(s,"/vmode4")) SetVideoMode(640, 480);
     if (strstr(s,"/vmode5")) SetVideoMode(800, 600);     
     if (strstr(s,"prj=")) { strcpy(ProjectName, (s+4)); GameState = 1; }
  } 
}




void AddExplosion(float x, float y, float z)
{
   Explosions[ExpCount].pos.x = x;
   Explosions[ExpCount].pos.y = y;
   Explosions[ExpCount].pos.z = z;
   Explosions[ExpCount].FTime = 0;
   ExpCount++;
}


void AddShipTask(int cindex)
{
  TCharacter *cptr = &Characters[cindex];

  BOOL TROPHYON  = (GetLandUpH(cptr->pos.x, cptr->pos.z) - GetLandH(cptr->pos.x, cptr->pos.z) < 100) && 
	               (!Tranq);

  if (TROPHYON) {
     ShipTask.clist[ShipTask.tcount] = cindex;
     ShipTask.tcount++;
	 AddVoice(ShipModel.SoundFX[3].length,
		      ShipModel.SoundFX[3].lpData);
  }

  //===== trophy =======//
  SYSTEMTIME st;
  GetLocalTime(&st);
  int t=0;
  for (t=0; t<23; t++)
	  if (!TrophyRoom.Body[t].ctype) break;

  float score = (float)DinoInfo[Characters[cindex].CType].BaseScore;

  if (TrophyRoom.Last.ssucces>1)
	  score*=(1.f + TrophyRoom.Last.ssucces / 10.f);

  if (Characters[cindex].CType!=TargetDino+4) score/=2.f;

  if (Tranq    ) score *= 1.25f;
  if (RadarMode) score *= 0.70f;
  if (ScentMode) score *= 0.80f;
  if (CamoMode ) score *= 0.85f;
  TrophyRoom.Score+=(int)score;


  if (TROPHYON) {
   TrophyTime = 20 * 1000;  
   TrophyBody = t;  
   TrophyRoom.Body[t].ctype  = Characters[cindex].CType;
   TrophyRoom.Body[t].scale  = Characters[cindex].scale;
   TrophyRoom.Body[t].weapon = TargetWeapon;
   TrophyRoom.Body[t].score  = (int)score;
   TrophyRoom.Body[t].phase  = (RealTime & 3);
   TrophyRoom.Body[t].time = (st.wHour<<10) + st.wMinute;
   TrophyRoom.Body[t].date = (st.wYear<<20) + (st.wMonth<<10) + st.wDay;
   TrophyRoom.Body[t].range = VectorLength( SubVectors(Characters[cindex].pos, PlayerPos) ) / 128.f;
   PrintLog("Trophy added: ");
   PrintLog(DinoInfo[Characters[cindex].CType].Name);
   PrintLog("\n");
  }  
}

void InitShip(int cindex)
{
	TCharacter *cptr = &Characters[cindex];

	Ship.DeltaY = 2048.f + DinoInfo[cptr->CType].ShDelta * cptr->scale;

	Ship.pos.x = PlayerX - 50*256; 
	if (Ship.pos.x < 256) Ship.pos.x = PlayerX + 50*256; 
	Ship.pos.z = PlayerZ - 50*256;
	if (Ship.pos.z < 256) Ship.pos.z = PlayerZ + 50*256; 
	Ship.pos.y = GetLandUpH(Ship.pos.x, Ship.pos.z)  + Ship.DeltaY;

	Ship.tgpos.x = cptr->pos.x;
	Ship.tgpos.z = cptr->pos.z;
    Ship.tgpos.y = GetLandUpH(Ship.tgpos.x, Ship.tgpos.z)  + Ship.DeltaY;
	Ship.State = 0;

	Ship.retpos = Ship.pos;
	Ship.cindex = cindex;
	Ship.FTime = 0;
}





void InitGameInfo()
{
    WeapInfo[0].Name = "Shotgun";
	WeapInfo[0].Power = 1.5f;
	WeapInfo[0].Prec  = 1.1f;
	WeapInfo[0].Loud  = 0.3f;
	WeapInfo[0].Rate  = 1.6f;
	WeapInfo[0].Shots = 6;

	WeapInfo[1].Name = "X-Bow";
	WeapInfo[1].Power = 1.1f;
	WeapInfo[1].Prec  = 0.7f;
	WeapInfo[1].Loud  = 1.9f;
	WeapInfo[1].Rate  = 1.2f;
	WeapInfo[1].Shots = 8;

    WeapInfo[2].Name = "Sniper Rifle";
	WeapInfo[2].Power = 1.0f;
	WeapInfo[2].Prec  = 1.8f;
	WeapInfo[2].Loud  = 0.6f;
	WeapInfo[2].Rate  = 1.0f;
	WeapInfo[2].Shots = 6;

	for (int c=0; c<12; c++) {
		DinoInfo[c].Scale0 = 800;
		DinoInfo[c].ScaleA = 600;
		DinoInfo[c].ShDelta = 0;
	}


	DinoInfo[ 0].Name = "Moschops";
	DinoInfo[ 0].Health0 = 2;
	DinoInfo[ 0].Mass = 0.15f;

    DinoInfo[ 1].Name = "Galimimus";
	DinoInfo[ 1].Health0 = 2;
	DinoInfo[ 1].Mass = 0.1f;

	DinoInfo[ 2].Name = "Dimorphodon";
    DinoInfo[ 2].Health0 = 1;
	DinoInfo[ 2].Mass = 0.05f;

	DinoInfo[ 3].Name = "";


	DinoInfo[ 4].Name = "Parasaurolophus";
	DinoInfo[ 4].Mass = 1.5f;
	DinoInfo[ 4].Length = 5.8f;
	DinoInfo[ 4].Radius = 320.f;
	DinoInfo[ 4].Health0 = 5;
	DinoInfo[ 4].BaseScore = 6;
	DinoInfo[ 4].SmellK = 0.8f; DinoInfo[ 4].HearK = 1.f; DinoInfo[ 4].LookK = 0.4f;
	DinoInfo[ 4].ShDelta = 48;

	DinoInfo[ 5].Name = "Pachycephalosaurus";
	DinoInfo[ 5].Mass = 0.8f;
	DinoInfo[ 5].Length = 4.5f;
	DinoInfo[ 5].Radius = 280.f;
	DinoInfo[ 5].Health0 = 4;
	DinoInfo[ 5].BaseScore = 8;
	DinoInfo[ 5].SmellK = 0.4f; DinoInfo[ 5].HearK = 0.8f; DinoInfo[ 5].LookK = 0.6f;
	DinoInfo[ 5].ShDelta = 36;

	DinoInfo[ 6].Name = "Stegosaurus";
    DinoInfo[ 6].Mass = 7.f;
	DinoInfo[ 6].Length = 7.f;
	DinoInfo[ 6].Radius = 480.f;
	DinoInfo[ 6].Health0 = 5;
	DinoInfo[ 6].BaseScore = 7;
	DinoInfo[ 6].SmellK = 0.4f; DinoInfo[ 6].HearK = 0.8f; DinoInfo[ 6].LookK = 0.6f;
	DinoInfo[ 6].ShDelta = 128;

	DinoInfo[ 7].Name = "Allosaurus";
	DinoInfo[ 7].Mass = 0.5;
	DinoInfo[ 7].Length = 4.2f;
	DinoInfo[ 7].Radius = 256.f;
	DinoInfo[ 7].Health0 = 3;
	DinoInfo[ 7].BaseScore = 12;
	DinoInfo[ 7].Scale0 = 1000;
	DinoInfo[ 7].ScaleA = 600;
	DinoInfo[ 7].SmellK = 1.0f; DinoInfo[ 7].HearK = 0.3f; DinoInfo[ 7].LookK = 0.5f;
	DinoInfo[ 7].ShDelta = 32;
	DinoInfo[ 7].DangerCall = TRUE;

	DinoInfo[ 8].Name = "Triceratops";
	DinoInfo[ 8].Mass = 3.f;
	DinoInfo[ 8].Length = 5.0f;
	DinoInfo[ 8].Radius = 512.f;
	DinoInfo[ 8].Health0 = 8;
	DinoInfo[ 8].BaseScore = 9;
	DinoInfo[ 8].SmellK = 0.6f; DinoInfo[ 8].HearK = 0.5f; DinoInfo[ 8].LookK = 0.4f;
	DinoInfo[ 8].ShDelta = 148;

	DinoInfo[ 9].Name = "Velociraptor";
	DinoInfo[ 9].Mass = 0.3f;
	DinoInfo[ 9].Length = 4.0f;
	DinoInfo[ 9].Radius = 256.f;
	DinoInfo[ 9].Health0 = 3;
	DinoInfo[ 9].BaseScore = 16;
	DinoInfo[ 9].ScaleA = 400;
	DinoInfo[ 9].SmellK = 1.0f; DinoInfo[ 9].HearK = 0.5f; DinoInfo[ 9].LookK = 0.4f;
	DinoInfo[ 9].ShDelta =-24;
	DinoInfo[ 9].DangerCall = TRUE;

	DinoInfo[10].Name = "T-Rex";
    DinoInfo[10].Mass = 6.f;
	DinoInfo[10].Length = 12.f;
	DinoInfo[10].Radius = 400.f;
	DinoInfo[10].Health0 = 1024;
	DinoInfo[10].BaseScore = 20;
	DinoInfo[10].SmellK = 0.85f; DinoInfo[10].HearK = 0.8f; DinoInfo[10].LookK = 0.8f;
	DinoInfo[10].ShDelta = 168;
	DinoInfo[10].DangerCall = TRUE;
}


void InitEngine()
{
    WATERANI     = TRUE;
	NODARKBACK   = TRUE;
    LoDetailSky  = TRUE;
    CORRECTION   = TRUE;
	FOGON        = TRUE;
	FOGENABLE    = TRUE;
    Clouds       = TRUE;   
    SKY          = TRUE;
    GOURAUD      = TRUE;   
    MODELS       = TRUE;   
    //TIMER        = TRUE;
    BITMAPP      = FALSE;
    MIPMAP       = TRUE;
    NOCLIP       = FALSE;
    CLIP3D       = TRUE;

    DEBUG        = FALSE;
    SLOW         = FALSE;
	LOWRESTX     = FALSE;
    MORPHP       = TRUE;
    MORPHA       = TRUE;

	GameState = 0;

	RadarMode    = FALSE;

	fnt_BIG = CreateFont(
        23, 10, 0, 0,
        600, 0,0,0,
#ifdef __rus
		RUSSIAN_CHARSET,		
#else
        ANSI_CHARSET,
#endif				
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, NULL);	




    fnt_Small = CreateFont(
        14, 5, 0, 0,
        100, 0,0,0,
#ifdef __rus
		RUSSIAN_CHARSET,		
#else
        ANSI_CHARSET,
#endif		        
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, NULL);	


    fnt_Midd  = CreateFont(
        16, 7, 0, 0,
        550, 0,0,0,
#ifdef __rus
		RUSSIAN_CHARSET,		
#else
        ANSI_CHARSET,
#endif		        
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, NULL);	




    Heap = HeapCreate( 0, 16000000, 40000000 );
    if( Heap == NULL ) {
      MessageBox(hwndMain,"Error creating heap.","Error",IDOK);     
      return; }

    Textures[255] = (TEXTURE*) _HeapAlloc(Heap, 0, sizeof(TEXTURE));

    WaterR = 10;
    WaterG = 38;
    WaterB = 46;
    WaterA = 10;
	TargetDino = 0;
    MessageList.timeleft = 0;

	InitGameInfo();

	InitMenu();
    
    InitAudioSystem(hwndMain);
	        
    InitDirectDraw();

    CreateVideoDIB();
    CreateFadeTab();	
    CreateDivTable();
    InitClips();
	/*
#ifdef _3dfx
    SetVideoMode(640, 480);    
#else
    SetVideoMode(512, 384);    
#endif
*/
	FULLSCREEN = TRUE;
    MenuState = -1;
    
//    MenuState = 0;
    TrophyRoom.RegNumber=0;
	LoadTrophy();

    PlayerX = (ctMapSize / 2) * 256;
	PlayerZ = (ctMapSize / 2) * 256;
    strcpy(ProjectName,"hunt");

    ProcessCommandLine();    
    
    ctViewR = 36;
    Soft_Persp_K = 1.5f;
    HeadY = 220;

	FogsList[0].fogRGB = 0x000000;
	FogsList[0].YBegin = 0;
//  FogsList[0].YMax = 0;
	FogsList[0].Transp = 000;
	FogsList[0].FLimit = 000; 	

	FogsList[127].fogRGB = 0x604500;	
    FogsList[127].Mortal = FALSE;
	FogsList[127].Transp = 450; 
	FogsList[127].FLimit = 160; 

	FillMemory( FogsMap, sizeof(FogsMap), 0);
	PrintLog("Init Engine: Ok.\n");
}


void ShutDownEngine()
{
   ReleaseDC(hwndMain,hdcMain);
   if (DirectActive)
       lpDD->RestoreDisplayMode();

}






void ProcessSyncro()
{
   RealTime = timeGetTime();
   srand( (unsigned) RealTime );
   if (SLOW) RealTime/=4;
   TimeDt = RealTime - PrevTime;
   if (TimeDt<0) TimeDt = 10;
   if (TimeDt>10000) TimeDt = 10;
   if (TimeDt>1000) TimeDt = 1000;
   PrevTime = RealTime;
   Takt++;
   if (!PAUSE)
    if (MyHealth) MyHealth+=TimeDt*4;
   if (MyHealth>MAX_HEALTH) MyHealth = MAX_HEALTH;   
}












void MakeCall()
{
   if (ObservMode || TrophyMode) return;
   if (CallLockTime) return;
   
   CallLockTime=1024*3;
   
   NextCall+=(RealTime % 2)+1;
   NextCall%=3;

   AddVoice(fxCall[NextCall].length,  fxCall[NextCall].lpData);

   float dmin = 200*256;
   int ai = -1;

   for (int c=0; c<ChCount; c++) {
	 TCharacter *cptr = &Characters[c];

	 if (DinoInfo[TargetDino+4].DangerCall)
		 if (cptr->CType<4) {
			 cptr->State=2;
			 cptr->AfraidTime = (10 + rRand(5)) * 1024; 
		 }

	 if (cptr->CType!=4 + TargetDino) continue;
	 if (cptr->AfraidTime) continue;
	 if (cptr->State) continue;

	 float d = VectorLength(SubVectors(PlayerPos, cptr->pos));
	 if (d < 98 * 256) {
	  if (rRand(128) > 32)
	    if (d<dmin) { dmin = d; ai = c; }
	  cptr->tgx = PlayerX + siRand(1800);
	  cptr->tgz = PlayerZ + siRand(1800);
	 }
   }

   if (ai!=-1) {
	   answpos = SubVectors(Characters[ai].pos, PlayerPos);
       answpos.x/=-3.f; answpos.y/=-3.f; answpos.z/=-3.f;
       answpos = SubVectors(PlayerPos, answpos);
	   answtime = 2000 + rRand(2000);	   
   }
	              
}


void MakeShot(float ax, float ay, float az,
              float bx, float by, float bz)
{
  int sres;
  TrophyRoom.Last.smade++;
  if (TargetWeapon!=1)
   sres = TraceShot(ax, ay, az, bx, by, bz);
  else {
	 Vector3d dl;
	 float dy = 40;
	 dl.x = (bx-ax) / 3;
	 dl.y = (by-ay) / 3;
	 dl.z = (bz-az) / 3;
	 bx = ax + dl.x;
	 by = ay + dl.y - dy / 2;
	 bz = az + dl.z;
	 sres = TraceShot(ax, ay, az, bx, by, bz);
	 if (sres!=-1) goto ENDTRACE;
	 ax = bx; ay = by; az = bz;

	 bx = ax + dl.x;
	 by = ay + dl.y - dy * 3;
	 bz = az + dl.z;
	 sres = TraceShot(ax, ay, az, bx, by, bz);
	 if (sres!=-1) goto ENDTRACE;
	 ax = bx; ay = by; az = bz;

	 bx = ax + dl.x;
	 by = ay + dl.y - dy * 5;
	 bz = az + dl.z;
	 sres = TraceShot(ax, ay, az, bx, by, bz);
	 if (sres!=-1) goto ENDTRACE;
	 ax = bx; ay = by; az = bz;
  }

ENDTRACE:
  if (sres==-1) return;

  AddExplosion(bx, by, bz);

  int mort = (sres & 0xFF00) && (Characters[ShotDino].Health);
  sres &= 0xFF;
  
  if (sres!=3) return;
  if (!Characters[ShotDino].Health) return;

  if (mort) Characters[ShotDino].Health = 0; 
       else Characters[ShotDino].Health--;

  
  if (sres == 3) 
	if (!Characters[ShotDino].Health) {	 
	 DemoPoint.pos = Characters[ShotDino].pos;
	 DemoPoint.pos.y;
	 if (Characters[ShotDino].CType>3) {
	   TrophyRoom.Last.ssucces++;
	   AddShipTask(ShotDino);	   
	 }

	 if (Characters[ShotDino].CType<=3) 
	     Characters_AddSecondaryOne(Characters[ShotDino].CType);
     
	 DemoPoint.CIndex = ShotDino;
	}
	else {
	 Characters[ShotDino].AfraidTime = 60*1000;
	 if (Characters[ShotDino].CType!=10 || Characters[ShotDino].State==0)
        Characters[ShotDino].State = 2;	 
	}   

   if (Characters[ShotDino].CType==10) 
	if (Characters[ShotDino].State) 
	   Characters[ShotDino].State = 5;
	else
		Characters[ShotDino].State = 1;
}


void RemoveCharacter(int index)
{
  if (index==-1) return;
  memcpy( &Characters[index], &Characters[index+1], (255 - index) * sizeof(TCharacter) );
  ChCount--;
   
  if (DemoPoint.CIndex > index) DemoPoint.CIndex--;

  for (int c=0; c<ShipTask.tcount; c++) 
	 if (ShipTask.clist[c]>index) ShipTask.clist[c]--;
}


void AnimateShip()
{
  if (Ship.State==-1) {
	  SetAmbient3d(0,0, 0,0,0);
	  if (!ShipTask.tcount) return; 
	  InitShip(ShipTask.clist[0]);
	  memcpy(&ShipTask.clist[0], &ShipTask.clist[1], 250*4);
	  ShipTask.tcount--;
	  return;
  }


  SetAmbient3d(ShipModel.SoundFX[0].length, 
	           ShipModel.SoundFX[0].lpData, 
			   Ship.pos.x, Ship.pos.y, Ship.pos.z);

  int _TimeDt = TimeDt;

//====== get up/down time acceleration ===========//
  if (Ship.FTime) {
	int am = ShipModel.Animation[0].AniTime;
	if (Ship.FTime < 500) _TimeDt = TimeDt * (Ship.FTime + 48) / 548;
    if (am-Ship.FTime < 500) _TimeDt = TimeDt * (am-Ship.FTime + 48) / 548;
	if (_TimeDt<2) _TimeDt=2;
  }


  Ship.tgalpha    = FindVectorAlpha(Ship.tgpos.x - Ship.pos.x, Ship.tgpos.z - Ship.pos.z);
  float currspeed;
  float dalpha = AngleDifference(Ship.tgalpha, Ship.alpha);        
  

  float L = VectorLength( SubVectors(Ship.tgpos, Ship.pos) );
  Ship.pos.y+=0.3f*(float)cos(RealTime / 256.f);


  if (Ship.State)
   if (Ship.speed>1)
   if (L<4000)
	if (VectorLength(SubVectors(PlayerPos, Ship.pos))<(ctViewR+2)*256) {
		Ship.tgpos.x += (float)cos(Ship.alpha) * 256*6.f;
		Ship.tgpos.z += (float)sin(Ship.alpha) * 256*6.f;
		Ship.tgpos.y = GetLandUpH(Ship.tgpos.x, Ship.tgpos.z) + Ship.DeltaY;
	}


//========= animate down ==========//
  if (Ship.State==3) {
	Ship.FTime+=_TimeDt;
	if (Ship.FTime>=ShipModel.Animation[0].AniTime) {
	  Ship.FTime=ShipModel.Animation[0].AniTime-1;
      Ship.State=2;
	  AddVoice(ShipModel.SoundFX[4].length,
		       ShipModel.SoundFX[4].lpData);
	  AddVoice3d(ShipModel.SoundFX[1].length, ShipModel.SoundFX[1].lpData,
		        Ship.pos.x, Ship.pos.y, Ship.pos.z);   
	}
	return;
  }

  
//========= get body on board ==========//
  if (Ship.State) {
	  if (Ship.cindex!=-1) {
	    DeltaFunc(Characters[Ship.cindex].pos.y, Ship.pos.y-650 - (Ship.DeltaY-2048), _TimeDt / 3.f);
	    DeltaFunc(Characters[Ship.cindex].beta,  0, TimeDt / 4048.f);
	    DeltaFunc(Characters[Ship.cindex].gamma, 0, TimeDt / 4048.f);
	  }
	
	if (Ship.State==2) {
	  Ship.FTime-=_TimeDt;
	  if (Ship.FTime<0) Ship.FTime=0;

	  if (Ship.FTime==0)
		  if (fabs(Characters[Ship.cindex].pos.y - (Ship.pos.y-650 - (Ship.DeltaY-2048))) < 1.f) {
		      Ship.State = 1;		  	  
			  AddVoice(ShipModel.SoundFX[5].length,
		               ShipModel.SoundFX[5].lpData);
		  	  AddVoice3d(ShipModel.SoundFX[2].length, ShipModel.SoundFX[2].lpData,
		        Ship.pos.x, Ship.pos.y, Ship.pos.z);   
		  }
      return;
	}
  }
  
//====== speed ===============//  
  float vspeed = 1.f + L / 128.f;
  if (vspeed > 24) vspeed = 24;
  if (Ship.State) vspeed = 24;
  if (fabs(dalpha) > 0.4) vspeed = 0.f;
  float _s = Ship.speed;
  if (vspeed>Ship.speed) DeltaFunc(Ship.speed, vspeed, TimeDt / 200.f);
                    else Ship.speed = vspeed;

  if (Ship.speed>0 && _s==0) 
	  AddVoice3d(ShipModel.SoundFX[2].length, ShipModel.SoundFX[2].lpData,
		        Ship.pos.x, Ship.pos.y, Ship.pos.z);   

//====== fly ===========//
  float l = TimeDt * Ship.speed / 16.f;   
  
  if (fabs(dalpha) < 0.4)
  if (l<L) {
   Ship.pos.x += (float)cos(Ship.alpha)*l;
   Ship.pos.z += (float)sin(Ship.alpha)*l;
  } else {   
   if (Ship.State) {
	 Ship.State = -1;
	 RemoveCharacter(Ship.cindex);
	 return;
   } else {
	 Ship.pos = Ship.tgpos;
	 Ship.State = 3;
	 Ship.FTime = 1;
	 Ship.tgpos = Ship.retpos;
	 Characters[Ship.cindex].StateF = 0xFF;	 
	 AddVoice3d(ShipModel.SoundFX[1].length, ShipModel.SoundFX[1].lpData,
		        Ship.pos.x, Ship.pos.y, Ship.pos.z);   
   }
  }

//======= y movement ============//
  float h = GetLandUpH(Ship.pos.x, Ship.pos.z);
  DeltaFunc(Ship.pos.y, Ship.tgpos.y, TimeDt / 8.f);
  if (Ship.pos.y < h + 1024) Ship.pos.y = h + 1024;
  
  

//======= rotation ============//
  if (Ship.tgalpha > Ship.alpha) currspeed = 0.1f + (float)fabs(dalpha)/2.f;
                            else currspeed =-0.1f - (float)fabs(dalpha)/2.f;     
							 
  if (fabs(dalpha) > pi) currspeed*=-1;   
         
  DeltaFunc(Ship.rspeed, currspeed, (float)TimeDt / 420.f);
  
  float rspd=Ship.rspeed * TimeDt / 1024.f;
  if (fabs(dalpha) < fabs(rspd)) { Ship.alpha = Ship.tgalpha; Ship.rspeed/=2; }
  else {
	Ship.alpha+=rspd;
	if (Ship.State) 
	 if (Ship.cindex!=-1)
		Characters[Ship.cindex].alpha+=rspd;
  }

  if (Ship.alpha<0) Ship.alpha+=pi*2;
  if (Ship.alpha>pi*2) Ship.alpha-=pi*2;
//======== move body ===========//
  if (Ship.State) {
	  if (Ship.cindex!=-1) {
	   Characters[Ship.cindex].pos.x = Ship.pos.x;
	   Characters[Ship.cindex].pos.z = Ship.pos.z;
	  }
	 if (L>1000) Ship.tgpos.y+=TimeDt / 12.f;
  } else {
    Ship.tgpos.x = Characters[Ship.cindex].pos.x;
	Ship.tgpos.z = Characters[Ship.cindex].pos.z;
    Ship.tgpos.y = GetLandUpH(Ship.tgpos.x, Ship.tgpos.z)  + Ship.DeltaY;
  }
}



void ProcessTrophy()
{
	TrophyBody = -1;

	for (int c=0; c<ChCount; c++) {
		Vector3d p = Characters[c].pos;
		p.x+=Characters[c].lookx * 256*2.5f;
		p.z+=Characters[c].lookz * 256*2.5f;

		if (VectorLength( SubVectors(p, PlayerPos) ) < 148)
			TrophyBody = c;
	}

	if (TrophyBody==-1) return;

	TrophyBody = Characters[TrophyBody].State;
}





void AnimateProcesses()
{
//Wind.alpha+=siRand(16) / 512.f;
  Wind.speed+=siRand(1600) / 6400.f;
  if (Wind.speed< 0.f) Wind.speed=0.f;
  if (Wind.speed>20.f) Wind.speed=20.f;

  Wind.nv.x = (float) sin(Wind.alpha);
  Wind.nv.z = (float)-cos(Wind.alpha);
  Wind.nv.y = 0.f;


  if (answtime) {
	  answtime-=TimeDt;
	  if (answtime<=0) {
		  answtime = 0;
		  int r = rRand(128) % 3;
		  AddVoice3d(fxCall[r].length,  fxCall[r].lpData,
		          answpos.x, answpos.y, answpos.z);				  
	  }
  }



  if (CallLockTime) {
	  CallLockTime-=TimeDt;
	  if (CallLockTime<0) CallLockTime=0;
  }

  CheckAfraid();
  AnimateShip();
  if (TrophyMode)
      ProcessTrophy();
	  
  for (int e=0; e<ExpCount; e++) {
     Explosions[e].FTime+=TimeDt;
     if (Explosions[e].FTime >= ExplodInfo.Animation[0].AniTime) {         
       memcpy(&Explosions[e], &Explosions[e+1], sizeof(TExplosion) * (ExpCount+1-e) );
       e--;
       ExpCount--;
     }
  }



  if (ExitTime) {
	ExitTime-=TimeDt;
	if (ExitTime<=0) {		
        TrophyRoom.Total.time   +=TrophyRoom.Last.time;
	    TrophyRoom.Total.smade  +=TrophyRoom.Last.smade;
		TrophyRoom.Total.ssucces+=TrophyRoom.Last.ssucces;
		TrophyRoom.Total.path   +=TrophyRoom.Last.path;
		if (!ShotsLeft)
		   if (!TrophyRoom.Last.ssucces) {
			  TrophyRoom.Score--;
			  if (TrophyRoom.Score<0) TrophyRoom.Score = 0;
		   }
				  
	    if (MyHealth) SaveTrophy();
				 else LoadTrophy();				
	    GameState = 0;
	}
  }
}



void RemoveCurrentTrophy()
{	
    int p = 0;
	if (!TrophyMode) return;
	if (!TrophyRoom.Body[TrophyBody].ctype) return;

	PrintLog("Trophy removed: ");
	PrintLog(DinoInfo[TrophyRoom.Body[TrophyBody].ctype].Name);
	PrintLog("\n");

	for (int c=0; c<TrophyBody; c++)
		if (TrophyRoom.Body[c].ctype) p++;


	TrophyRoom.Body[TrophyBody].ctype = 0;

	if (TrophyMode) {
	  memcpy(&Characters[p],  
	         &Characters[p+1], 
		     (250-p) * sizeof(TCharacter) );
	  ChCount--;
	}

	TrophyTime = 0;
	TrophyBody = -1;
}


void LoadTrophy()
{
    int pr = TrophyRoom.RegNumber;
	FillMemory(&TrophyRoom, sizeof(TrophyRoom), 0);
	TrophyRoom.RegNumber = pr;
	DWORD l;
	char fname[128];
	int rn = TrophyRoom.RegNumber;
	wsprintf(fname, "trophy0%d.sav", TrophyRoom.RegNumber);
	HANDLE hfile = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile==INVALID_HANDLE_VALUE) {
		PrintLog("===> Error loading trophy!\n");
		return;
	}
	ReadFile(hfile, &TrophyRoom, sizeof(TrophyRoom), &l, NULL);

	ReadFile(hfile, &OptAgres, 4, &l, NULL);
	ReadFile(hfile, &OptDens , 4, &l, NULL);
	ReadFile(hfile, &OptSens , 4, &l, NULL);

	ReadFile(hfile, &OptRes, 4, &l, NULL);
	ReadFile(hfile, &FOGENABLE, 4, &l, NULL);
	ReadFile(hfile, &OptText , 4, &l, NULL);
	ReadFile(hfile, &SHADOWS3D, 4, &l, NULL);

	ReadFile(hfile, &KeyMap, sizeof(KeyMap), &l, NULL);
	ReadFile(hfile, &REVERSEMS, 4, &l, NULL);

	ReadFile(hfile, &ScentMode, 4, &l, NULL);
	ReadFile(hfile, &CamoMode, 4, &l, NULL);
	ReadFile(hfile, &RadarMode, 4, &l, NULL);
	ReadFile(hfile, &Tranq    , 4, &l, NULL);

	ReadFile(hfile, &OptSys  , 4, &l, NULL);

	SetupRes();

	CloseHandle(hfile);
	OPT_ALPHA_COLORKEY = TrophyRoom.Body[0].r4;
	TrophyRoom.RegNumber = rn;
	PrintLog("Trophy Loaded.\n");
//	TrophyRoom.Score = 299;
}




void SaveTrophy()
{
	DWORD l;
	char fname[128];
	wsprintf(fname, "trophy0%d.sav", TrophyRoom.RegNumber);

	int r = TrophyRoom.Rank;
	TrophyRoom.Rank = 0;
	if (TrophyRoom.Score >= 100) TrophyRoom.Rank = 1;
	if (TrophyRoom.Score >= 300) TrophyRoom.Rank = 2;

	if (r != TrophyRoom.Rank) {
		if (TrophyRoom.Rank==1) MenuState = 112;
		if (TrophyRoom.Rank==2) MenuState = 113;
	}

	TrophyRoom.Body[0].r4 = OPT_ALPHA_COLORKEY;

    //DeleteFile(fname);
    HANDLE hfile = CreateFile(fname, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		PrintLog("==>> Error saving trophy!\n");
		return;
	}
	WriteFile(hfile, &TrophyRoom, sizeof(TrophyRoom), &l, NULL);
    WriteFile(hfile, &OptAgres, 4, &l, NULL);
	WriteFile(hfile, &OptDens , 4, &l, NULL);
	WriteFile(hfile, &OptSens , 4, &l, NULL);

	WriteFile(hfile, &OptRes, 4, &l, NULL);
	WriteFile(hfile, &FOGENABLE, 4, &l, NULL);
	WriteFile(hfile, &OptText , 4, &l, NULL);
	WriteFile(hfile, &SHADOWS3D, 4, &l, NULL);

	WriteFile(hfile, &KeyMap, sizeof(KeyMap), &l, NULL);
	WriteFile(hfile, &REVERSEMS, 4, &l, NULL);	

	WriteFile(hfile, &ScentMode, 4, &l, NULL);
	WriteFile(hfile, &CamoMode, 4, &l, NULL);
	WriteFile(hfile, &RadarMode, 4, &l, NULL);
	WriteFile(hfile, &Tranq    , 4, &l, NULL);

	WriteFile(hfile, &OptSys  , 4, &l, NULL);

	CloseHandle(hfile);
	PrintLog("Trophy Saved.\n");
}


void LoadPlayersInfo()
{
	FillMemory(PlayerR, sizeof(PlayerR), 0);
	for (int p=0; p<16; p++) {			
	  char fname[128];
	  DWORD l;

	  wsprintf(fname, "trophy0%d.sav", p);
	  HANDLE hfile = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	  if (!hfile) continue;
	  ReadFile(hfile, &PlayerR[p], sizeof(PlayerR[p]), &l, NULL);
      CloseHandle(hfile);
	}
}