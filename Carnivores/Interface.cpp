#define INITGUID
#include "Hunt.h"
#include "stdio.h"

typedef struct _TMenuSet {
	int x0, y0;
	int Count;
	char Item[32][32];
} TMenuSet;


TMenuSet Options[3];
char HMLtxt[3][12];
char CKtxt[2][16];
char Restxt[8][24];
char Textxt[3][12];
char Ontxt[2][12];
char Systxt[2][12];
int CurPlayer = -1;
int MaxDino, AreaMax;

#define REGLISTX 320
#define REGLISTY 370

BOOL NEWPLAYER = FALSE;

int  MapVKKey(int k)
{
	if (k==VK_LBUTTON) return 124;
	if (k==VK_RBUTTON) return 125;
	return MapVirtualKey(k , 0);
}

void AddMenuItem(TMenuSet &ms, LPSTR txt)
{
	wsprintf(ms.Item[ms.Count++], "%s", txt);
}


POINT p;
int OptMode = 0;
int OptLine = 0;


void wait_mouse_release()
{
    while (GetAsyncKeyState(VK_RBUTTON) & 0x80000000);
	while (GetAsyncKeyState(VK_LBUTTON) & 0x80000000);
		
}


int GetTextW(HDC hdc, LPSTR s)
{
  SIZE sz;
  GetTextExtentPoint(hdc, s, strlen(s), &sz);
  return sz.cx;
}

void PrintText(LPSTR s, int x, int y, int rgb)
{
  HBITMAP hbmpOld = SelectObject(hdcCMain,hbmpVideoBuf);   
  SetBkMode(hdcCMain, TRANSPARENT);     
   
  SetTextColor(hdcCMain, 0x00000000);  
  TextOut(hdcCMain, x+1, y+1, s, strlen(s));
  SetTextColor(hdcCMain, rgb);
  TextOut(hdcCMain, x, y, s, strlen(s));

  SelectObject(hdcCMain,hbmpOld);		  
}

void DoHalt(LPSTR Mess)
{
  if (!HARD3D)
   if (DirectActive)
    if (FULLSCREEN) lpDD->RestoreDisplayMode();
  EnableWindow(hwndMain, FALSE);
  if (strlen(Mess)) {
	PrintLog("ABNORMAL_HALT: ");
	PrintLog(Mess);
	PrintLog("\n");
    MessageBox(NULL, Mess, "HUNT Termination", IDOK | MB_SYSTEMMODAL);
  }

  CloseLog();
  TerminateProcess(GetCurrentProcess(), 0);
}

void InitDirectDraw()
{  
   PrintLog("\n");
   PrintLog("==Init Direct Draw==\n");
   HRESULT hres;


   hres = DirectDrawCreate( NULL, &lpDD, NULL );
   if( hres != DD_OK ) {      
	  wsprintf(logt, "DirectDrawCreate Error: %Xh\n", hres);
      PrintLog(logt);
	  DoHalt("");	  
   }
   PrintLog("DirectDrawCreate: Ok\n");



/*
   hres = lpDD->QueryInterface( IID_IDirectDraw2, (LPVOID *)&lpDD2);
   if( hres != DD_OK ) {
	  wsprintf(logt, "QueryInterface Error: %Xh\n", hres);
      PrintLog(logt);
	  DoHalt("");
   }
   PrintLog("QueryInterface: Ok\n");
*/


   DWORD cl;
   if (HARD3D) cl = DDSCL_NORMAL; else cl = DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN;   
   cl = DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN;

#ifdef _DEBUG
   cl = DDSCL_NORMAL;
#endif
   
   hres = lpDD->SetCooperativeLevel( hwndMain, cl);
   if( hres != DD_OK )  {
	  wsprintf(logt, "SetCooperativeLevel Error: %Xh\n", hres);
      PrintLog(logt);
	  DoHalt("");
   }
   PrintLog("SetCooperativeLevel: Ok\n");


   PrintLog("Direct Draw activated.\n");
   PrintLog("\n");
   DirectActive = TRUE;
}


void WaitRetrace()
{
    BOOL bv = FALSE;
    if (DirectActive)
      while (!bv)  lpDD->GetVerticalBlankStatus(&bv);
}

void SetFullScreen()
{   
   HRESULT res = DD_OK;
   if (!DirectActive) return;
   if (HARD3D) return;
   if (!GameState) return;

   FULLSCREEN=!FULLSCREEN;
     
   if (!HARD3D)
    if (FULLSCREEN) res = lpDD->SetDisplayMode( WinW, WinH, 16);    
               else res = lpDD->RestoreDisplayMode();
 
   if (res != DD_OK) {
	 wsprintf(logt, "DDRAW: Error set video mode %dx%d\n", WinW, WinH);
     PrintLog(logt);
   }

   lpVideoRAM = 0;
   
   if (!HARD3D) 
     SetVideoMode(WinW, WinH);
   SetCursorPos(VideoCX, VideoCY);
/*
   DDSURFACEDESC ddsd;
   FillMemory( &ddsd, 0, sizeof( DDSURFACEDESC ) );
   ddsd.dwSize         = sizeof( ddsd );
   ddsd.dwFlags        = DDSD_CAPS;
   ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

   if( lpddsPrimary )
      lpddsPrimary->Release( );
   
   if( lpDD2->CreateSurface( &ddsd, &lpddsPrimary, NULL ) != DD_OK )
      return;

   if (lpddsPrimary->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL ) == DD_OK ) {      
        lpVideoRAM = (WORD*)ddsd.lpSurface;
        lpddsPrimary->Unlock( ddsd.lpSurface );
        };

   lpddsPrimary->GetSurfaceDesc( &ddsd );
   iBytesPerLine = ddsd.lPitch;

   ShowCursor(FALSE); */
}


void Wait(int time)
{
  unsigned int t = timeGetTime() + time;
  while (t>timeGetTime()) ;
}



void SetVideoMode(int W, int H)
{
   WinW = W; 
   WinH = H;

   WinEX = WinW - 1;
   WinEY = WinH - 1;
   VideoCX = WinW / 2;
   VideoCY = WinH / 2;
    
   CameraW = (float)VideoCX*1.25f;
   CameraH = CameraW;

   if (HARD3D) FULLSCREEN=TRUE;

   if (!HARD3D) {
	HRESULT res = DD_OK;
    if (FULLSCREEN) 
       res = lpDD->SetDisplayMode( WinW, WinH, 16);        
	
	if (res != DD_OK) {
	 wsprintf(logt, "DDRAW: Error set video mode %dx%d\n", WinW, WinH);
     PrintLog(logt);
    }
   }

   if (FULLSCREEN) {
    SetWindowPos(hwndMain, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);
    SetCursorPos(VideoCX, VideoCY);
   } else {
	SetWindowPos(hwndMain, HWND_TOP, 10, 10, WinW, WinH, SWP_SHOWWINDOW);
   }

   //char buf[40];
   //wsprintf(buf,"Video mode: %dx%d", WinW, WinH);
   //AddMessage(buf);
   LoDetailSky =(W>400);
   SetCursor(hcArrow);
   if (FULLSCREEN) while (ShowCursor(FALSE)>=0) ;
              else while (ShowCursor(TRUE)<0) ;
}


void SetMenuVideoMode()
{   
   HRESULT hres = lpDD->SetDisplayMode( 800, 600, 16);
   if (hres != DD_OK) hres = lpDD->SetDisplayMode( 800, 600, 24);
   if (hres != DD_OK) hres = lpDD->SetDisplayMode( 800, 600, 32);
   if (hres != DD_OK) hres = lpDD->SetDisplayMode( 800, 600, 8);
   
   if (hres != DD_OK) 
     PrintLog("DDRAW: Error setting menu mode\n");

   SetWindowPos(hwndMain, HWND_TOP, 0, 0, 800, 600, SWP_SHOWWINDOW);
   SetCursorPos(400, 300);
   while (ShowCursor(TRUE)<0) ;
}




void ReloadDinoInfo()
{
	switch (TargetDino) {
	 case 0: LoadPictureTGA(DinoPic,  "HUNTDAT\\MENU\\DINOPIC\\mpara.tga"); 
		     LoadPictureTGA(DinoPicM, "HUNTDAT\\MENU\\DINOPIC\\mpara_on.tga"); 
			 if (OptSys) LoadTextFile(DinoText, "HUNTDAT\\MENU\\DINOPIC\\para.txu");
			        else LoadTextFile(DinoText, "HUNTDAT\\MENU\\DINOPIC\\para.txm");
		     break;
	 case 1: LoadPictureTGA(DinoPic,  "HUNTDAT\\MENU\\DINOPIC\\mpach.tga"); 
		     LoadPictureTGA(DinoPicM, "HUNTDAT\\MENU\\DINOPIC\\mpach_on.tga"); 			 
			 if (OptSys) LoadTextFile(DinoText, "HUNTDAT\\MENU\\DINOPIC\\pach.txu");
			        else LoadTextFile(DinoText, "HUNTDAT\\MENU\\DINOPIC\\pach.txm");
		     break;
	 case 2: LoadPictureTGA(DinoPic,  "HUNTDAT\\MENU\\DINOPIC\\msteg.tga"); 
		     LoadPictureTGA(DinoPicM, "HUNTDAT\\MENU\\DINOPIC\\msteg_on.tga"); 			 
			 if (OptSys) LoadTextFile(DinoText, "HUNTDAT\\MENU\\DINOPIC\\steg.txu");
			        else LoadTextFile(DinoText, "HUNTDAT\\MENU\\DINOPIC\\steg.txm");
		     break;
	 case 3: LoadPictureTGA(DinoPic,  "HUNTDAT\\MENU\\DINOPIC\\mallo.tga"); 
		     LoadPictureTGA(DinoPicM, "HUNTDAT\\MENU\\DINOPIC\\mallo_on.tga"); 
			 if (OptSys) LoadTextFile(DinoText, "HUNTDAT\\MENU\\DINOPIC\\allo.txu");
			        else LoadTextFile(DinoText, "HUNTDAT\\MENU\\DINOPIC\\allo.txm");
		     break;

     case 4: if (MaxDino<4) 
		      LoadPictureTGA(DinoPic,  "HUNTDAT\\MENU\\DINOPIC\\mtric_no.tga"); 		      
			 else {
			  LoadPictureTGA(DinoPic,  "HUNTDAT\\MENU\\DINOPIC\\mtric.tga"); 
		      LoadPictureTGA(DinoPicM, "HUNTDAT\\MENU\\DINOPIC\\mtric_on.tga"); 
			 }			 
			 if (OptSys) LoadTextFile(DinoText, "HUNTDAT\\MENU\\DINOPIC\\tric.txu");
			        else LoadTextFile(DinoText, "HUNTDAT\\MENU\\DINOPIC\\tric.txm");
		     break;						 
     case 5: if (MaxDino<4) 
              LoadPictureTGA(DinoPic,  "HUNTDAT\\MENU\\DINOPIC\\mvelo_no.tga"); 
			 else {
			  LoadPictureTGA(DinoPic,  "HUNTDAT\\MENU\\DINOPIC\\mvelo.tga"); 
		      LoadPictureTGA(DinoPicM, "HUNTDAT\\MENU\\DINOPIC\\mvelo_on.tga"); 
			 }			 
			 if (OptSys) LoadTextFile(DinoText, "HUNTDAT\\MENU\\DINOPIC\\velo.txu");
			        else LoadTextFile(DinoText, "HUNTDAT\\MENU\\DINOPIC\\velo.txm");
		     break;			 

     case 6: if (MaxDino<6) 
			  LoadPictureTGA(DinoPic,  "HUNTDAT\\MENU\\DINOPIC\\mtrex_no.tga"); 
			 else {
			  LoadPictureTGA(DinoPic,  "HUNTDAT\\MENU\\DINOPIC\\mtrex.tga"); 
		      LoadPictureTGA(DinoPicM, "HUNTDAT\\MENU\\DINOPIC\\mtrex_on.tga"); 
			 }
			 if (OptSys) LoadTextFile(DinoText, "HUNTDAT\\MENU\\DINOPIC\\trex.txu");
			        else LoadTextFile(DinoText, "HUNTDAT\\MENU\\DINOPIC\\trex.txm");
		     break;			      
	}			
}


void ReloadWeaponInfo()
{
	switch (TargetWeapon) {
	 case 0:
      LoadPictureTGA(WepPic,   "HUNTDAT\\MENU\\WEPPIC\\weapon1.tga");
	  LoadTextFile  (WepText,  "HUNTDAT\\MENU\\WEPPIC\\weapon1.txt");
	  break;
	 case 1:
      LoadPictureTGA(WepPic,   "HUNTDAT\\MENU\\WEPPIC\\weapon2.tga");
	  LoadTextFile  (WepText,  "HUNTDAT\\MENU\\WEPPIC\\weapon2.txt");
	  break;
	 case 2:
	  if (TrophyRoom.Rank<1)
        LoadPictureTGA(WepPic,   "HUNTDAT\\MENU\\WEPPIC\\weapon3a.tga");
	  else
		LoadPictureTGA(WepPic,   "HUNTDAT\\MENU\\WEPPIC\\weapon3.tga");
	  LoadTextFile  (WepText,  "HUNTDAT\\MENU\\WEPPIC\\weapon3.txt");
	  break;
	}
}

void ReloadAreaInfo()
{
	char aname[64], tname[64];
	if (TargetArea > AreaMax)  wsprintf(aname, "HUNTDAT\\MENU\\LANDPIC\\area%d_no.bmp", TargetArea+1);
	else                       wsprintf(aname, "HUNTDAT\\MENU\\LANDPIC\\area%d.bmp", TargetArea+1);
    wsprintf(tname, "HUNTDAT\\MENU\\LANDPIC\\area%d.txt", TargetArea+1);
	LoadPicture(LandPic,  aname);
	LoadTextFile(LandText, tname);
}




void LoadMenuTGA()
{    
    LPSTR m1,m2,mm;
	MenuSelect = 0;

    switch (MenuState) {
    case -3:
		  m1="HUNTDAT\\MENU\\menud.tga";
		  m2="HUNTDAT\\MENU\\menud_on.tga";
		  mm="HUNTDAT\\MENU\\md_map.raw";
		  break;
    case -2:
		  m1="HUNTDAT\\MENU\\menul.tga";
		  m2="HUNTDAT\\MENU\\menul_on.tga";
		  mm="HUNTDAT\\MENU\\ml_map.raw";
		  break;
	case -1:
		  m1="HUNTDAT\\MENU\\menur.tga";
		  m2="HUNTDAT\\MENU\\menur_on.tga";
		  mm="HUNTDAT\\MENU\\mr_map.raw";
		  break;
      case 0:
          m1="HUNTDAT\\MENU\\menum.tga";
          m2="HUNTDAT\\MENU\\menum_on.tga";
          mm="HUNTDAT\\MENU\\main_map.raw";
          break;
      case 1:
          m1="HUNTDAT\\MENU\\loc_off.tga";
          m2="HUNTDAT\\MENU\\loc_on.tga";
          mm="HUNTDAT\\MENU\\loc_map.raw";
          break;
      case 2:
          m1="HUNTDAT\\MENU\\wep_off.tga";
          m2="HUNTDAT\\MENU\\wep_on.tga";
          mm="HUNTDAT\\MENU\\wep_map.raw";
          break;
      case 3:
          m1="HUNTDAT\\MENU\\opt_off.tga";
          m2="HUNTDAT\\MENU\\opt_on.tga";
          mm="HUNTDAT\\MENU\\opt_map.raw";
          break;
      case 4:
          m1="HUNTDAT\\MENU\\credits.tga";
          m2="HUNTDAT\\MENU\\credits.tga";
          mm="";
          break;
      case 8:
          m1="HUNTDAT\\MENU\\menuq.tga";
          m2="HUNTDAT\\MENU\\menuq_on.tga";
          mm="HUNTDAT\\MENU\\mq_map.raw";
          break;	  
      case 111:
		  m1="HUNTDAT\\MENU\\menus.tga";
          m2="HUNTDAT\\MENU\\menus.tga";
          mm="";
          break;
	  case 112:
		  m1="HUNTDAT\\MENU\\menua.tga";
          m2="HUNTDAT\\MENU\\menua.tga";
          mm="";
          break;
	  case 113:
		  m1="HUNTDAT\\MENU\\menue.tga";
          m2="HUNTDAT\\MENU\\menue.tga";
          mm="";
          break;
    }
    
    DWORD l;
    HANDLE hfile;

    hfile = CreateFile(m1, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if( hfile == INVALID_HANDLE_VALUE ) return;
    SetFilePointer(hfile, 18, 0, FILE_BEGIN);
    for (int y=599; y>=0; y--) ReadFile( hfile, (WORD*)lpMenuBuf+y*800,  800*2, &l, NULL );       
    CloseHandle( hfile ); 
    Sleep(2);

    hfile = CreateFile(m2, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if( hfile == INVALID_HANDLE_VALUE ) return;
    SetFilePointer(hfile, 18, 0, FILE_BEGIN);
    //ReadFile( hfile, lpMenuBuf2, 800*600*2, &l, NULL );       
	for (y=599; y>=0; y--) ReadFile( hfile, (WORD*)lpMenuBuf2+y*800,  800*2, &l, NULL );       
    CloseHandle( hfile ); 
    Sleep(2);

	FillMemory(MenuMap, sizeof(MenuMap), 0);

    hfile = CreateFile(mm, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if( hfile == INVALID_HANDLE_VALUE ) return;    
    ReadFile( hfile, MenuMap, 120000, &l, NULL );    
    CloseHandle( hfile ); 
    Sleep(2);
   
}






void ShowMenuVideo()
{
  HDC _hdc =  hdcCMain;
  HBITMAP hbmpOld = SelectObject(_hdc,hbmpVideoBuf);
  if (RestartMode) 
	  FillMemory(lpVideoBuf, 1024*600*2, 0);
  BitBlt(hdcMain,0,0,800,600, _hdc,0,0, SRCCOPY);
  SelectObject(_hdc,hbmpOld);		  
}




void AddPicture(TPicture &pic, int x0, int y0)
{  
  for (int y=0; y<pic.H; y++)
	memcpy( (WORD*)lpVideoBuf + x0 + ((y0+y)<<10),
	        pic.lpImage + y * pic.W,
			pic.W<<1);
}


void Line(HDC hdc, int x1, int y1, int x2, int y2)
{
        MoveToEx(hdc, x1,y1, NULL);
        LineTo(hdc, x2,y2);
}



void DrawSlider(int x, int y, float l)
{
	int W = 120; y+=13;	

	HPEN wp = CreatePen(PS_SOLID, 0, 0x009F9F9F);
	HBRUSH wb = CreateSolidBrush(0x003FAF3F);

    HPEN oldpen = SelectObject(hdcCMain, GetStockObject(BLACK_PEN));
	HBRUSH  oldbrs = SelectObject(hdcCMain, GetStockObject(BLACK_BRUSH));
	HBITMAP oldbmp = SelectObject(hdcCMain,hbmpVideoBuf);   
	

	x+=1; y+=1; 
	Line(hdcCMain, x,y-9, x+W+1,y-9);
	Line(hdcCMain, x,y, x+W+1,y);
	Line(hdcCMain, x,y-8, x,y);
	Line(hdcCMain, x+W,y-8, x+W,y);
	Line(hdcCMain, x+W/2,y-8, x+W/2,y);
	Line(hdcCMain, x+W/4,y-8, x+W/4,y);
	Line(hdcCMain, x+W*3/4,y-8, x+W*3/4,y);

	x-=1; y-=1; 
	SelectObject(hdcCMain, wp);
	Line(hdcCMain, x,y-9, x+W+1,y-9);
	Line(hdcCMain, x,y, x+W+1,y);
	Line(hdcCMain, x,y-8, x,y);
	Line(hdcCMain, x+W,y-8, x+W,y);
	Line(hdcCMain, x+W/2,y-8, x+W/2,y);
	Line(hdcCMain, x+W/4,y-8, x+W/4,y);
	Line(hdcCMain, x+W*3/4,y-8, x+W*3/4,y);

	W-=2;
	PatBlt(hdcCMain, x+2,y-5, (int)(W*l/2.f), 4, PATCOPY);

	SelectObject(hdcCMain, wb);
	PatBlt(hdcCMain, x+1,y-6, (int)(W*l/2.f), 4, PATCOPY);
	

	SelectObject(hdcCMain, oldpen);
	SelectObject(hdcCMain, oldbrs);
    SelectObject(hdcCMain, oldbmp);
	DeleteObject(wp);	
	DeleteObject(wb);	
}


void DrawOptions()
{
	HFONT oldfont = SelectObject(hdcCMain, fnt_BIG);
	
	for (int m=0; m<3; m++) 
		for (int l=0; l<Options[m].Count; l++) {
			int x0 = Options[m].x0;
			int y0 = Options[m].y0 + l*25;

			int c = 0x005282b2;
			if (m == OptMode && l== OptLine) c = 0x00a0d0f0;
			PrintText(Options[m].Item[l], 
				      x0 - GetTextW(hdcCMain, Options[m].Item[l]),  y0, c);
			

			c = 0xB0B0A0;
			x0+=16;
			if (m==0) 
				switch (l) {
				case 0:
					PrintText(HMLtxt[OptAgres], x0, y0, c);
					break;
				case 1:
					PrintText(HMLtxt[OptDens], x0, y0, c);
					break;
				case 2:
					PrintText(HMLtxt[OptSens], x0, y0, c);
					break;
				case 3:
					PrintText(Systxt[OptSys], x0, y0, c);
					break;

			}

  			if (m==1)
			if (l<Options[m].Count-1)
			 if (WaitKey==l)
				PrintText("<?>", x0, y0, c); 
			 else
				PrintText(KeysName[MapVKKey( *((int*)(&KeyMap)+l) )], x0, y0, c); 
			 else
				PrintText(Ontxt[REVERSEMS], x0, y0, c); 


			if (m==2) 
				switch (l) {
				case 0:
					PrintText(Restxt[OptRes], x0, y0, c);
					break;
			    case 1:
					PrintText(Ontxt[FOGENABLE], x0, y0, c);
					break;
			    case 2:
					PrintText(Textxt[OptText], x0, y0, c);
					break;
				case 3:
					PrintText(Ontxt[SHADOWS3D], x0, y0, c);
					break;
				case 4:
					PrintText(CKtxt[OPT_ALPHA_COLORKEY], x0, y0, c);
					break;
			}


		
	}

	
	SelectObject(hdcCMain, oldfont);
}





void DrawMainStats()
{
   HFONT oldfont = SelectObject(hdcCMain, fnt_BIG);
   char t[32];
   int  c = 0x003070A0;

   PrintText(TrophyRoom.PlayerName, 90, 9, c);

   wsprintf(t,"%d", TrophyRoom.Score);
   PrintText(t, 540, 9, c);
   
   switch (TrophyRoom.Rank) {
	case 0: PrintText("Novice  ", 344, 9, c); break;
	case 1: PrintText("Advanced", 344, 9, c); break;
	case 2: PrintText("Expert  ", 344, 9, c); break;
	}

   SelectObject(hdcCMain, oldfont);
}


void DrawMainStats2()
{
   char t[32];
   int  c = 0x00209090;   
   int  ttm = (int)TrophyRoom.Total.time;
   int  ltm = (int)TrophyRoom.Last.time;

   PrintText("Path travelled  ", 718 - GetTextW(hdcCMain,"Path travelled  "), 78, c);
   
   if (OptSys)  sprintf(t,"%1.0f ft.", TrophyRoom.Last.path / 0.3f);
   else         sprintf(t,"%1.0f m.", TrophyRoom.Last.path);

   PrintText(t, 718, 78, c);

   PrintText("Time hunted  ", 718 - GetTextW(hdcCMain,"Time hunted  "), 98, c);
   sprintf(t,"%d:%02d:%02d", (ltm / 3600), ((ltm % 3600) / 60), (ltm % 60) );
   PrintText(t, 718, 98, c);

   PrintText("Shots made  ", 718 - GetTextW(hdcCMain,"Shots made  "), 118, c);
   sprintf(t,"%d", TrophyRoom.Last.smade);
   PrintText(t, 718, 118, c);

   PrintText("Shots succeed  ", 718 - GetTextW(hdcCMain,"Shots succeed  "), 138, c);
   sprintf(t,"%d", TrophyRoom.Last.ssucces);
   PrintText(t, 718, 138, c);




   PrintText("Path travelled  ", 718 - GetTextW(hdcCMain,"Path travelled  "), 208, c);
   if (TrophyRoom.Total.path < 1000)
    if (OptSys)  sprintf(t,"%1.0f ft.", TrophyRoom.Total.path / 0.3f);
    else         sprintf(t,"%1.0f m.", TrophyRoom.Total.path);
   else
	if (OptSys)  sprintf(t,"%1.1f miles.", TrophyRoom.Total.path / 1667);
    else         sprintf(t,"%1.1f km.", TrophyRoom.Total.path / 1000.f);


   PrintText(t, 718, 208, c);

   PrintText("Time hunted  ", 718 - GetTextW(hdcCMain,"Time hunted  "), 228, c);
   sprintf(t,"%d:%02d:%02d", (ttm / 3600), ((ttm % 3600) / 60), (ttm % 60) );
   PrintText(t, 718, 228, c);

   PrintText("Shots made  ", 718 - GetTextW(hdcCMain,"Shots made  "), 248, c);
   sprintf(t,"%d", TrophyRoom.Total.smade);
   PrintText(t, 718, 248, c);

   PrintText("Shots succeed  ", 718 - GetTextW(hdcCMain,"Shots succeed  "), 268, c);
   sprintf(t,"%d", TrophyRoom.Total.ssucces);
   PrintText(t, 718, 268, c);

   PrintText("Succes ratio  ", 718 - GetTextW(hdcCMain,"Succes ratio  "), 288, c);
   if (TrophyRoom.Total.smade)
     sprintf(t,"%d%%", TrophyRoom.Total.ssucces * 100 / TrophyRoom.Total.smade);
   else
	 wsprintf(t,"100%%");
   PrintText(t, 718, 288, c);   

   DrawMainStats();
}


void DrawRegistry()
{
   int  c = 0x00309070;
   char t[128];
   if ( (timeGetTime() % 800) > 300)
	   wsprintf(t,"%s_", TrophyRoom.PlayerName);
   else
	   wsprintf(t,"%s", TrophyRoom.PlayerName);

   PrintText(t, 330, 326, c);
   for (int i=0; i<6; i++) {
	   if (PlayerR[i].PName[0]==0) {
		   PrintText("...", REGLISTX, REGLISTY+i*20, 0x003070A0);
		   continue;
	   }

	   if (i==CurPlayer) c = 0x005090E0; else c=0x003070A0;
	   PrintText(PlayerR[i].PName, REGLISTX, REGLISTY+i*20, c);

	   if (PlayerR[i].Rank==2) PrintText("Exp", REGLISTX+146, REGLISTY+i*20, c); else
	   if (PlayerR[i].Rank==1) PrintText("Adv", REGLISTX+146, REGLISTY+i*20, c); else
	                           PrintText("Nov", REGLISTX+146, REGLISTY+i*20, c); 
   }
}



void DrawRemove()
{
   //HFONT oldfont = SelectObject(hdcCMain, fnt_Small);
   char t[32];
   int  c = 0x00B08030;   
   
   PrintText("Do you want to delete player", 290, 370, c);
   wsprintf(t,"'%s' ?",PlayerR[CurPlayer].PName);
   PrintText(t, 300, 394, c);
   

   //SelectObject(hdcCMain, oldfont);
}


void CopyMenuToVideo(int m)
{
  int smap[256];
  FillMemory(smap, 256*4, 0);
  LastMenuSelect = MenuSelect;
  int *msrc;
  int mda;
  if (!m) m=255;

  smap[m] = 1;

  
  if (MenuState==1) 
	  if (ObservMode) smap[3] = 1; else smap[3] = 0;
  
  if (MenuState==2) {
	  if (Tranq) smap[3] = 1; else smap[3] = 0;
	  
	  if (ScentMode) smap[4] = 1; else smap[4] = 0;
	  if (CamoMode)  smap[5] = 1; else smap[5] = 0;
	  if (RadarMode) smap[6] = 1; else smap[6] = 0;
  }

  if (MenuState==3) {
   smap[1] = 0; smap[2] = 0; smap[3] = 0;
   smap[OptMode+1]=1; 
  }

  for (int y=0; y<300; y++) {
   int *vbuf = (int*)lpVideoBuf+(y<<10);
   mda=y*800;
   for (int x=0; x<400; x++) {     
		 if (smap[MenuMap[y][x]]) msrc = (int*)lpMenuBuf2 + mda; else msrc = (int*)lpMenuBuf+mda;
         *(vbuf)     = *(msrc);
		 *(vbuf+512) = *(msrc + 400);		
         vbuf++;
		 mda++;
		}
  }


  if (MenuState==0) DrawMainStats();
  if (MenuState==8) DrawMainStats();
  if (MenuState==112) DrawMainStats();
  if (MenuState==113) DrawMainStats();

  if (MenuState==111) DrawMainStats2();
  if (MenuState==-1) DrawRegistry();


  if (MenuState==1) {
   AddPicture(LandPic, 143, 71);
   if (MenuSelect==6 && (TargetDino > MaxDino) ) m = 0;
   if (m==6) AddPicture(DinoPicM, 401, 64);
        else AddPicture(DinoPic, 401, 64);

   for (int t=0; t<DinoText.Lines; t++)
     PrintText(DinoText.Text[t], 420, 330+t*16, 0x209F85);   

     PrintText("Sight", 520 - GetTextW(hdcCMain,"Sight"), 450+0*20, 0x209F85);
	 PrintText("Scent", 520 - GetTextW(hdcCMain,"Scent"), 450+1*20, 0x209F85);
	 PrintText("Hearing", 520 - GetTextW(hdcCMain,"Hearing"), 450+2*20, 0x209F85);
     	 
     DrawSlider(526, 450+0*20,  DinoInfo[TargetDino+4].LookK*2);
	 DrawSlider(526, 450+1*20,  DinoInfo[TargetDino+4].SmellK*2);
	 DrawSlider(526, 450+2*20,  DinoInfo[TargetDino+4].HearK*2);
	 

   if (MenuSelect==3)
    for (t=0; t<ObserText.Lines; t++)
      PrintText(ObserText.Text[t], 50, 330+t*16, 0x809F25);   
   else
    for (t=0; t<LandText.Lines; t++)
      PrintText(LandText.Text[t], 50, 330+t*16, 0x809F25);   
  }

  if (MenuState==2) {
   AddPicture(WepPic, 120, 120);
   for (int t=0; t<WepText.Lines; t++)
     PrintText(WepText.Text[t], 60, 330+t*16, 0xB09F45);

	 PrintText("Fire power:", 160 - GetTextW(hdcCMain,"Fire power:"), 454+0*20, 0xB09F45);
	 PrintText("Shot precision:", 160 - GetTextW(hdcCMain,"Shot precision:"), 454+1*20, 0xB09F45);
	 PrintText("Volume", 160 - GetTextW(hdcCMain,"Volume:"), 454+2*20, 0xB09F45);
     PrintText("Rate of fire:", 160 - GetTextW(hdcCMain,"Rate of fire:"), 454+3*20, 0xB09F45);
	 
     DrawSlider(166, 454+0*20,  WeapInfo[TargetWeapon].Power);
	 DrawSlider(166, 454+1*20,  WeapInfo[TargetWeapon].Prec);
	 DrawSlider(166, 454+2*20,  2.0f-WeapInfo[TargetWeapon].Loud);
	 DrawSlider(166, 454+3*20,  WeapInfo[TargetWeapon].Rate);

   if (MenuSelect==3)
    for (int t=0; t<TranqText.Lines; t++)
     PrintText(TranqText.Text[t], 420, 330+t*16, 0xB09F45);

   if (MenuSelect==4)
    for (int t=0; t<ScentText.Lines; t++)
     PrintText(ScentText.Text[t], 420, 330+t*16, 0xB09F45);

   if (MenuSelect==5)
    for (int t=0; t<ComfText.Lines; t++)
     PrintText(ComfText.Text[t], 420, 330+t*16, 0xB09F45);
   
   if (MenuSelect==6)
    for (int t=0; t<RadarText.Lines; t++)
     PrintText(RadarText.Text[t], 420, 330+t*16, 0xB09F45);
  }

  if (MenuState==3) DrawOptions();

  if (MenuState==-3) DrawRemove();
}






void SelectMenu0(int s)
{  
    if (!s) return;    
	AddVoice3d(fxMenuGo.length, fxMenuGo.lpData,
		       (float)1024+(p.x-200)*3, 0.f, 200.f);
    CopyMenuToVideo(0); ShowMenuVideo();        Wait(50);
    CopyMenuToVideo(s); ShowMenuVideo();        
	LastMenuSelect=255;
          
   switch (s) {      
//============ new =============//
   case 1: 
	  TrophyMode = FALSE;
      MenuState=1;	  
      LoadMenuTGA();
	  ReloadDinoInfo();
	  ReloadAreaInfo();
	  break;   
//============ options =============//
   case 2:
	  if (WinW==320) OptRes=0;
	  if (WinW==400) OptRes=1;
	  if (WinW==512) OptRes=2;
	  if (WinW==640) OptRes=3;
	  if (WinW==800) OptRes=4;
	  if (WinW==1024) OptRes=5;

	  MenuState=3;
	  LoadMenuTGA();
	  break;
//============ trphy =============//
   case 3:
	  wsprintf(ProjectName, "huntdat\\areas\\trophy");
	  TrophyMode = TRUE;
      GameState = 1;
      break;
//============ credits =============//
   case 4: 
      MenuState=4;       
      LoadMenuTGA();
      break;
//============ quit =============//
   case 5: 	  
	  //DestroyWindow(hwndMain);
      MenuState=8;       
      LoadMenuTGA();
      break;
   case 6:	  
      MenuState=111;
      LoadMenuTGA();
      break;
  }
  
  wait_mouse_release();
}



void SelectMenu1(int s)
{
    if (!s) return;    
    AddVoice3d(fxMenuGo.length, fxMenuGo.lpData,
		       (float)1024+(p.x-200)*3, 0, 200);                 
	CopyMenuToVideo(0); 	
	ShowMenuVideo();        Wait(50);

    CopyMenuToVideo(s); 	
	ShowMenuVideo();        

	LastMenuSelect=255;
	
	switch (s) {           
     case 1:
      if (TargetArea<5) TargetArea++; else TargetArea=0;
	  ReloadAreaInfo();
	  break;
	 case 2:
      if (TargetArea) TargetArea--; else TargetArea=5;
	  ReloadAreaInfo();
	  break;       
	 case 3:
	  ObservMode = !ObservMode;
	  break;
	 

	 case 4:
      if (TargetDino<6) TargetDino++; else TargetDino=0;
	  ReloadDinoInfo();
	  break;
	 case 5:
	  if (TargetDino) TargetDino--; else TargetDino=6;
	  ReloadDinoInfo();
	  break;
	 
     case 7: 
      MenuState=0;
      LoadMenuTGA();
      break;
     case 8:
	  if (TargetDino > MaxDino) {
		  MessageBeep(0xFFFFFFFF);
		  break; }
	  if (TargetArea > AreaMax) {
		  MessageBeep(0xFFFFFFFF);
		  break;
	  }

	  if (!ChInfo[TargetDino+4].mptr) break;
	  if (ObservMode) {
		  wsprintf(ProjectName, "huntdat\\areas\\area%d", TargetArea+1);		  
	      GameState = 1;		 
	  } else {
	      MenuState=2;
          LoadMenuTGA();
	      ReloadWeaponInfo();
	  }      
	  break;
	}
	
	
    wait_mouse_release();
}


void SelectMenu2(int s)
{
    if (!s) return;    
    AddVoice3d(fxMenuGo.length, fxMenuGo.lpData,
		       (float)1024+(p.x-200)*3, 0, 200);                 
	CopyMenuToVideo(0); ShowMenuVideo();  Wait(50);
    CopyMenuToVideo(s); ShowMenuVideo();        

	LastMenuSelect=255;

	
	switch (s) {           
     case 1:
      if (TargetWeapon<2) TargetWeapon++; else TargetWeapon=0;
	  ReloadWeaponInfo();
	  break;
	 case 2:
      if (TargetWeapon) TargetWeapon--; else TargetWeapon=2;
	  ReloadWeaponInfo();
	  break;
	 case 3:
	  Tranq = !Tranq;
	  break;

	 case 4:
	  ScentMode = !ScentMode;
	  break;
	 case 5:
	  CamoMode = !CamoMode;
	  break;
	 case 6:
	  RadarMode = !RadarMode;
	  break;		 
	 	 
     case 7: 
      MenuState=1;
      LoadMenuTGA();
      break;
     case 8: 	  
		 if (TrophyRoom.Rank<1 && TargetWeapon>1) {
			 MessageBeep(0xFFFFFFFF);
			 break;
		 }
      wsprintf(ProjectName, "huntdat\\areas\\area%d", TargetArea+1);
	  GameState = 1;
	  break;
	}
	
	
    wait_mouse_release();
}



void SelectOptions()
{
	switch (OptMode) {
	case 0:
		switch (OptLine) {
		case 0: OptAgres++; if (OptAgres>2) OptAgres=0; break;
		case 1: OptDens ++; if (OptDens >2) OptDens=0; break;
		case 2: OptSens ++; if (OptSens >2) OptSens=0; break;
	    case 3: OptSys=1-OptSys; break;
		}
		break;

	case 2:
		switch (OptLine) {
		case 0: OptRes++; if (OptRes>5) OptRes=0; break;
		case 1: FOGENABLE=!FOGENABLE; break;
        case 2: OptText++; if (OptText>2) OptText=0; break;
		case 3: SHADOWS3D=!SHADOWS3D; break;
	    case 4: OPT_ALPHA_COLORKEY=!OPT_ALPHA_COLORKEY; break;
		}
		break;

	case 1:
		if (OptLine==Options[1].Count-1) REVERSEMS=!REVERSEMS;
		break;
	}
}

HCURSOR menucurs;



void ProcessMainMenu()
{
 if (KeyboardState [VK_LBUTTON] & 128) 
    SelectMenu0(MenuSelect);
            
 if (MenuSelect!=LastMenuSelect) {
	CopyMenuToVideo(MenuSelect);  	
    ShowMenuVideo();  
 }
}


void ProcessCreditMenu()
{ 
  if (KeyboardState [VK_ESCAPE] & 128) KeyboardState [VK_LBUTTON] |= 128;
  if (KeyboardState [VK_RETURN] & 128) KeyboardState [VK_LBUTTON] |= 128;

  if (KeyboardState [VK_LBUTTON] & 128) {
    MenuState=0;       
    LoadMenuTGA();
	wait_mouse_release();
  }
           
  CopyMenuToVideo(MenuSelect);
  ShowMenuVideo();  
}


void ProcessLicense()
{ 
 if (MenuSelect)
  if (KeyboardState [VK_LBUTTON] & 128) {

	AddVoice3d(fxMenuGo.length, fxMenuGo.lpData, (float)1024+(p.x-200)*3, 0.f, 200.f);
    CopyMenuToVideo(0); ShowMenuVideo();        Wait(50);
    CopyMenuToVideo(MenuSelect); ShowMenuVideo();        
	LastMenuSelect=255;
	wait_mouse_release();

	if (MenuSelect==1) {
      MenuState=0;       
      LoadMenuTGA();
	} else {
	  char fname[128];	  
	  wsprintf(fname, "trophy0%d.sav", TrophyRoom.RegNumber);
	  DeleteFile(fname);
	  DoHalt("");
	}
	
  }
           
  CopyMenuToVideo(MenuSelect);
  ShowMenuVideo();  
}


void RemovePlayer()
{
	if (CurPlayer==-1) return;
	TrophyRoom.PlayerName[0]=0;
	char fname[128];
	wsprintf(fname, "trophy0%d.sav", CurPlayer);
	DeleteFile(fname);	
	LoadPlayersInfo();
}



void ProcessRemove()
{ 
 if (MenuSelect)
  if (KeyboardState [VK_LBUTTON] & 128) {

	AddVoice3d(fxMenuGo.length, fxMenuGo.lpData, (float)1024+(p.x-200)*3, 0.f, 200.f);
    CopyMenuToVideo(0); ShowMenuVideo();        Wait(50);
    CopyMenuToVideo(MenuSelect); ShowMenuVideo();        
	LastMenuSelect=255;
	wait_mouse_release();

	if (MenuSelect==1) {
	  RemovePlayer();
      MenuState=-1;      
      LoadMenuTGA();
	} else {
	  MenuState=-1;
	  LoadMenuTGA();
	  return;
	}
	
  }
           
  CopyMenuToVideo(MenuSelect);
  ShowMenuVideo();  
}


void IdentifyPlayer()
{
   CurPlayer=-1;
   if (!TrophyRoom.PlayerName[0]) return;

   NEWPLAYER = FALSE;
//=== search for registered player =======//
   for (int i=0; i<6; i++)
	   if (!strcmp(TrophyRoom.PlayerName,  PlayerR[i].PName)) {
		   CurPlayer=i;
		   return;
	   }

//=== search for free slot =======//
   for (i=0; i<6; i++)
	   if (!PlayerR[i].PName[0]) {
		   NEWPLAYER = TRUE;
		   CurPlayer=i;
		   break;
	   }        
   
   
   if (CurPlayer!=-1) {
     TrophyRoom.RegNumber=CurPlayer;
	 if (HARD3D) OptRes = 3; else OptRes=2;
     RadarMode = FALSE;
	 ScentMode = FALSE;
	 CamoMode  = FALSE;
     SaveTrophy();
   }
}





void ProcessRegistry()
{ 
  if (KeyboardState [VK_RETURN] & 128) {
	  MenuSelect=1;
	  KeyboardState [VK_LBUTTON] |= 128;
  }

  if (KeyboardState [VK_LBUTTON] & 128) {
	  for (int i=0; i<6; i++) 
        if (p.x<<1 > REGLISTX && p.x<<1 < REGLISTX + 200 &&
			p.y<<1 > REGLISTY+i*20 && p.y<<1 < REGLISTY+i*20+18) 
		if (PlayerR[i].PName[0])
		{
			if (CurPlayer==i) MenuSelect=1;
			CurPlayer=i;
			wsprintf(TrophyRoom.PlayerName, "%s", PlayerR[i].PName);
		}
	  
    AddVoice3d(fxMenuGo.length, fxMenuGo.lpData, (float)1024+(p.x-200)*3, 0.f, 200.f);
    CopyMenuToVideo(0); ShowMenuVideo();        Wait(50);
    CopyMenuToVideo(MenuSelect); ShowMenuVideo();        
	LastMenuSelect=255;
	wait_mouse_release();

	if (MenuSelect==2) 
		if (CurPlayer>=0)
			if (PlayerR[CurPlayer].PName[0]) {
		MenuState=-3;
        LoadMenuTGA();	  
		//RemovePlayer();	
	}
          
    if (MenuSelect==1) {
	  IdentifyPlayer();
	  if (CurPlayer>=0) {
        if (NEWPLAYER) MenuState=-2; 
		          else MenuState=0; 
	    TrophyRoom.RegNumber=CurPlayer;
	    LoadTrophy();
        LoadMenuTGA();	  
	  }
	}
  }	  
           
  
	CopyMenuToVideo(MenuSelect);  	
    ShowMenuVideo();  
  
}





void ProcessOptionsMenu()
{ 
  if (KeyboardState [VK_LBUTTON] & 128) {

	if (MenuSelect)    AddVoice(fxMenuGo.length, fxMenuGo.lpData);

	if (!MenuSelect)
     if (WaitKey==-1)
		for (int m=0; m<3; m++)			
			for (int l=0; l<Options[m].Count; l++) {
			 int x0 = Options[m].x0;
			 int y0 = Options[m].y0 + l*25;			 
			 if (p.x*2>x0-120 && p.x*2<x0+120 && p.y*2>y0 && p.y*2<y0+23) {
				 OptMode = m; OptLine = l;
				 if (m==1) 
				   if (l<Options[m].Count-1)
					 WaitKey = l;				 
				 AddVoice(fxMenuGo.length, fxMenuGo.lpData);
				 SelectOptions();				 
				 CopyMenuToVideo(0); ShowMenuVideo();
				 //SetupKey();
			 }
			}


    if (MenuSelect)
	  if (MenuSelect==4) {
		if (OptRes==0) { WinW = 320; WinH=240; }
		if (OptRes==1) { WinW = 400; WinH=300; }
		if (OptRes==2) { WinW = 512; WinH=384; }
		if (OptRes==3) { WinW = 640; WinH=480; }
		if (OptRes==4) { WinW = 800; WinH=600; }		
		if (OptRes==5) { WinW =1024; WinH=768; }		
		
		SaveTrophy();
		CopyMenuToVideo(0); ShowMenuVideo();  Wait(50);
        CopyMenuToVideo(MenuSelect); ShowMenuVideo();        
        MenuState=0;       
        LoadMenuTGA();
	  } else {
		OptMode = MenuSelect-1;
		OptLine = 0;
	  }
 	wait_mouse_release();
  }
           
  CopyMenuToVideo(MenuSelect);
  
  ShowMenuVideo();  
}


void ProcessLocationMenu()
{
 MaxDino = 3;
 AreaMax = 2;
 if (TrophyRoom.Rank  )   { MaxDino = 5; AreaMax = 4; }
 if (TrophyRoom.Rank==2)  { MaxDino = 6; AreaMax = 5; }
 
 if (KeyboardState [VK_LBUTTON] & 128) 
    SelectMenu1(MenuSelect);
            
 if (MenuSelect!=LastMenuSelect) {
	CopyMenuToVideo(MenuSelect);  		
    ShowMenuVideo();  
 }
}


void ProcessWeaponMenu()
{
 if (TargetDino==6) Tranq = FALSE;
 if (KeyboardState [VK_LBUTTON] & 128) 
    SelectMenu2(MenuSelect); 
 if (TargetDino==6) Tranq = FALSE;            

 if (MenuSelect!=LastMenuSelect) {
	CopyMenuToVideo(MenuSelect);  		
    ShowMenuVideo();  
 }
}


void ProcessQuitMenu()
{
  if (KeyboardState [VK_RETURN] & 128) {
	  MenuSelect=1; KeyboardState [VK_LBUTTON] |= 128;
  }

  if (KeyboardState [VK_ESCAPE] & 128) {
	  MenuSelect=2; KeyboardState [VK_LBUTTON] |= 128;
  }

  if (KeyboardState [VK_LBUTTON] & 128) {
    if (MenuSelect==1) DestroyWindow(hwndMain);
    if (MenuSelect==2) MenuState = 0;
    if (MenuSelect)    AddVoice(fxMenuGo.length, fxMenuGo.lpData);
  }

  if (KeyboardState [VK_ESCAPE] & 128) MenuState = 0;  
  
//  //SetCursor(hcArrow);
  CopyMenuToVideo(MenuSelect);
  ShowMenuVideo();    

  if (MenuState!=8) LoadMenuTGA();
}



void ProcessMenu()
{      
  
  if (_GameState != GameState) {
	 PrintLog("  Entered menu\n");
	 LastMenuSelect=255;
     if (MenuState>0 && MenuState<112) MenuState = 0;
     LoadMenuTGA();     
     ShutDown3DHardware();              
	 SetMenuVideoMode();  
	 SetCursor(hcArrow);
	 AudioStop();
	 AudioSetCameraPos(1024, 0, 0, 0);
	 if (MenuState>=0 && !RestartMode)
	 SetAmbient(fxMenuAmb.length,
	            fxMenuAmb.lpData, 240);
  }
  _GameState = GameState;

  if (RestartMode) {	 
	  GameState = 1;
	  return;
	}

  GetCursorPos(&p); p.x/=2; p.y/=2;
  int m = MenuSelect;
  MenuSelect = MenuMap[p.y][p.x];
  GetKeyboardState(KeyboardState);   

  if (MenuState>=0)
  SetAmbient3d(fxMenuAmb.length,
	           fxMenuAmb.lpData, 
			   (float)1024+(p.x-200)*2, 0, 200);

  if (m!=MenuSelect && MenuSelect)
       AddVoice3d(fxMenuMov.length, fxMenuMov.lpData,
	              (float)1024+(p.x-200)*2, 0, 200);
 
  switch (MenuState) {
  case -3: ProcessRemove(); break;
  case -2: ProcessLicense(); break;
  case -1: ProcessRegistry(); break;
   case 0: ProcessMainMenu(); break;
   case 1: ProcessLocationMenu(); break;
   case 2: ProcessWeaponMenu(); break;
   case 3: ProcessOptionsMenu(); break;
   
   case 111:
   case 112:
   case 113:
   case 4: ProcessCreditMenu(); break;
   case 8: ProcessQuitMenu(); break;
  }
}


void InitMenu()
{
    Options[0].x0 = 200;
	Options[0].y0 = 100;
	AddMenuItem(Options[0], "Agressivity");
	AddMenuItem(Options[0], "Density");
	AddMenuItem(Options[0], "Sensitivity");
	AddMenuItem(Options[0], "Measurement");

	Options[2].x0 = 200;
	Options[2].y0 = 370;
	AddMenuItem(Options[2], "Resolution");
	AddMenuItem(Options[2], "Fog");
	AddMenuItem(Options[2], "Textures");
    AddMenuItem(Options[2], "Shadows");
	AddMenuItem(Options[2], "ColorKey");

	Options[1].x0 = 610;
	Options[1].y0 = 90;

	AddMenuItem(Options[1], "Forward");
	AddMenuItem(Options[1], "Backward");
	AddMenuItem(Options[1], "Turn Up");
	AddMenuItem(Options[1], "Turn Down");
	AddMenuItem(Options[1], "Turn Left");
	AddMenuItem(Options[1], "Turn Right");
    AddMenuItem(Options[1], "Fire");
	AddMenuItem(Options[1], "Get weapon");

	AddMenuItem(Options[1], "Step Left");
	AddMenuItem(Options[1], "Step Right");
	AddMenuItem(Options[1], "Strafe");
	AddMenuItem(Options[1], "Jump");	
    AddMenuItem(Options[1], "Run");
	AddMenuItem(Options[1], "Crouch");
	AddMenuItem(Options[1], "Call");
	AddMenuItem(Options[1], "Binoculars");
	
	AddMenuItem(Options[1], "Reverse mouse");
	//AddMenuItem(Options[1], "Mouse sensitivity");
	
	wsprintf(CKtxt[0],"Color");
	wsprintf(CKtxt[1],"Alpha Channel");

	wsprintf(HMLtxt[0],"Low");
	wsprintf(HMLtxt[1],"Medium");
	wsprintf(HMLtxt[2],"High");

	wsprintf(Restxt[0],"320x240");
	wsprintf(Restxt[1],"400x300");
	wsprintf(Restxt[2],"512x384");
	wsprintf(Restxt[3],"640x480");
	wsprintf(Restxt[4],"800x600");
	wsprintf(Restxt[5],"1024x768");
	wsprintf(Restxt[6],"1280x1024");

    wsprintf(Textxt[0],"Low");
	wsprintf(Textxt[1],"High");
	wsprintf(Textxt[2],"Auto");

	wsprintf(Ontxt[0],"Off");
	wsprintf(Ontxt[1],"On");

    wsprintf(Systxt[0],"Metric");
	wsprintf(Systxt[1],"US");
	

	OptText = 2;
	FOGENABLE = 1;
	SHADOWS3D = 1;
	OptDens = 1;
	OptSens = 1;
	OptAgres= 1;
	
	KeyMap.fkForward  = 'A';
	KeyMap.fkBackward = 'Z';
	KeyMap.fkFire     = VK_LBUTTON;
	KeyMap.fkShow     = VK_RBUTTON;
    KeyMap.fkJump     = VK_SPACE;
	KeyMap.fkCall     = VK_MENU;
	KeyMap.fkBinoc    = 'B';
	KeyMap.fkCrouch   = 'X';
	KeyMap.fkRun      = VK_SHIFT;
	KeyMap.fkUp       = VK_UP;
	KeyMap.fkDown     = VK_DOWN;
	KeyMap.fkLeft     = VK_LEFT;
	KeyMap.fkRight    = VK_RIGHT;
	WaitKey = -1;

}
