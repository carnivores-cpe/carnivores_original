#include "Hunt.h"
#include "stdio.h"
HANDLE hfile;
DWORD  l,HeapAllocated, HeapReleased;

void GenerateModelMipMaps(TModel *mptr);
void GenerateAlphaFlags(TModel *mptr);


LPVOID _HeapAlloc(HANDLE hHeap, 
                  DWORD dwFlags, 
                  DWORD dwBytes)
{
   LPVOID res = HeapAlloc(hHeap, 
                          dwFlags | HEAP_ZERO_MEMORY,
                          dwBytes);
   if (!res)
	   DoHalt("Heap allocation error!");

   HeapAllocated+=dwBytes;
   return res;
}


BOOL _HeapFree(HANDLE hHeap, 
               DWORD  dwFlags, 
               LPVOID lpMem)
{	
	if (!lpMem) return FALSE;
	
	HeapReleased+=
		HeapSize(hHeap, HEAP_NO_SERIALIZE, lpMem);

	BOOL res = HeapFree(hHeap, 
                       dwFlags, 
                       lpMem);
	if (!res)
		DoHalt("Heap free error!");
	
	return res;
}


void AddMessage(LPSTR mt)
{
  MessageList.timeleft = timeGetTime() + 2 * 1000;
  lstrcpy(MessageList.mtext, mt);
}

void PlaceHunter()
{
  if (LockLanding) return;

  if (TrophyMode) {
	  PlayerX = 76*256+128;
      PlayerZ = 70*256+128;
	  PlayerY = GetLandQH(PlayerX, PlayerZ);
	  return;
  }
  
  int p = (timeGetTime() % LandingList.PCount);
  PlayerX = (float)LandingList.list[p].x * 256+128;
  PlayerZ = (float)LandingList.list[p].y * 256+128;
  PlayerY = GetLandQH(PlayerX, PlayerZ);
}

int DitherHi(int C)
{
  int d = C & 255;  
  C = C / 256;
  if (rand() * 255 / RAND_MAX < d) C++;
  if (C>31) C=31;
  return C;
}


int DitherHiC(int C)
{
  int d = C & 7;    
  C = C / 8;
  if (rand() * 8 / RAND_MAX < d) C++;
  if (C>31) C=31;
  return C;
}




int DizeredHiColor(int R, int G, int B)
{
    return (DitherHiC(R)<<10) + 
           (DitherHiC(G)<< 5) +
           (DitherHiC(B));            
}


void CreateWaterTab()
{
  for (int c=0; c<0x8000; c++) {
     int R = (c >> 10);
	 int G = (c >>  5) & 31;
	 int B = c & 31;
     R =  1+(R * 8 ) / 28; if (R>31) R=31;
	 G =  2+(G * 18) / 28; if (G>31) G=31;
	 B =  3+(B * 22) / 28; if (B>31) B=31; /*
     R =  (WaterR*WaterA + R * WaterR) / 64; if (R>31) R=31;
	 G =  (WaterR*WaterA + G * WaterG) / 64; if (G>31) G=31;
	 B =  (WaterR*WaterA + B * WaterB) / 64; if (B>31) B=31; */
	 FadeTab[64][c] = HiColor(R, G, B);
   }
}

void CreateFadeTab()
{
#ifdef _soft
  for (int l=0; l<64; l++) 
   for (int c=0; c<0x8000; c++) {
     int R = (c >> 10);
	 int G = (c >>  5) & 31;
	 int B = c & 31;
     //if (l<16) ll=64
	 R = (int)((float)R * (64.f-l) / 60.f + (float)rand() *0.2f / RAND_MAX); if (R>31) R=31;
	 G = (int)((float)G * (64.f-l) / 60.f + (float)rand() *0.2f / RAND_MAX); if (G>31) G=31;
	 B = (int)((float)B * (64.f-l) / 60.f + (float)rand() *0.2f / RAND_MAX); if (B>31) B=31;
	 FadeTab[l][c] = HiColor(R, G, B);
   }

  CreateWaterTab();
#endif
}


void CreateDivTable()
{
  DivTbl[0] = 0x7fffffff;
  DivTbl[1] = 0x7fffffff;
  DivTbl[2] = 0x7fffffff;
  for( int i = 3; i < 10240; i++ )
     DivTbl[i] = (int) ((float)0x100000000 / i);

  for (int y=0; y<32; y++)
   for (int x=0; x<32; x++)
    RandomMap[y][x] = rand() * 1024 / RAND_MAX;
}


void CreateVideoDIB()
{
    hdcMain=GetDC(hwndMain);
    hdcCMain = CreateCompatibleDC(hdcMain);

    SelectObject(hdcMain,  fnt_Midd);
	SelectObject(hdcCMain, fnt_Midd);

	BITMAPINFOHEADER bmih;
	bmih.biSize = sizeof( BITMAPINFOHEADER ); 
    bmih.biWidth  =1024; 
    bmih.biHeight = -768; 
    bmih.biPlanes = 1; 
    bmih.biBitCount = 16;
    bmih.biCompression = BI_RGB; 
    bmih.biSizeImage = 0;
    bmih.biXPelsPerMeter = 400; 
    bmih.biYPelsPerMeter = 400; 
    bmih.biClrUsed = 0; 
    bmih.biClrImportant = 0;    

	BITMAPINFO binfo;
	binfo.bmiHeader = bmih;
	hbmpVideoBuf = 
     CreateDIBSection(hdcMain, &binfo, DIB_RGB_COLORS, &lpVideoBuf, NULL, 0);   


    bmih.biWidth  = 800; 
    bmih.biHeight = -600; 
    binfo.bmiHeader = bmih;
	hbmpMenuBuf = 
     CreateDIBSection(hdcMain, &binfo, DIB_RGB_COLORS, &lpMenuBuf, NULL, 0);   
    hbmpMenuBuf2 = 
     CreateDIBSection(hdcMain, &binfo, DIB_RGB_COLORS, &lpMenuBuf2, NULL, 0);   
}



int GetDepth(int y, int x)
{
  int h = HMap[y][x] + 48+1;
  float lmin = 17.2f;
  float l;
  BOOL Done = FALSE;
  for (int yy=-12; yy<=12; yy++)    
   for (int xx=-12; xx<=12; xx++) {
     if (!(FMap[(y+yy) & 511][(x+xx)& 511] & fmWater)) {
        l = (float)sqrt ( (xx*xx) + (yy*yy) );
        if (l<lmin) lmin = l;            
     }
   }
  
  //if (lmin>16.f) lmin = 16.f; 
  float ladd = 0;
  if (lmin>6) { ladd = lmin-6; lmin = 6; }
  h -= (int)(lmin*4 + ladd*2.4);
  if (h<0) h = 0;
  return h;
}


int GetObjectH(int x, int y, int R) 
{
  x = (x<<8) + 128;
  y = (y<<8) + 128;
  float hr,h;
  hr =GetLandH((float)x,    (float)y);
  h = GetLandH( (float)x+R, (float)y); if (h < hr) hr = h;
  h = GetLandH( (float)x-R, (float)y); if (h < hr) hr = h;
  h = GetLandH( (float)x,   (float)y+R); if (h < hr) hr = h;
  h = GetLandH( (float)x,   (float)y-R); if (h < hr) hr = h;
  hr += 15;
  return  (int) (hr / ctHScale + 48);
}


int GetObjectHWater(int x, int y) 
{
  if (FMap[y][x] & fmReverse)
   return (int)(HMap[y][x+1]+HMap[y+1][x]) / 2 + 48;
                else
   return (int)(HMap[y][x]+HMap[y+1][x+1]) / 2 + 48;
}



void CreateTMap()
{
 int x,y; 
 LandingList.PCount = 0;
 for (y=0; y<512; y++)
     for (x=0; x<512; x++) {          
          if (TMap1[y][x]==255) TMap1[y][x] = 1;
          if (TMap2[y][x]==255) TMap2[y][x] = 1;          
     }


  for (y=0; y<ctMapSize; y++)
   for (x=0; x<ctMapSize; x++) {

	if (OMap[y][x]==254) {		
	   LandingList.list[LandingList.PCount].x = x;
	   LandingList.list[LandingList.PCount].y = y;
	   LandingList.PCount++;
       OMap[y][x]=255;
	}

    int ob = OMap[y][x];
    if (ob == 255) { HMapO[y][x] = 48; continue; }
    
    //HMapO[y][x] = GetObjectH(x,y); 
    if (MObjects[ob].info.flags & ofPLACEUSER)   HMapO[y][x]+= 48;
    if (MObjects[ob].info.flags & ofPLACEGROUND) HMapO[y][x] = GetObjectH(x,y, MObjects[ob].info.GrRad);
    if (MObjects[ob].info.flags & ofPLACEWATER)  HMapO[y][x] = GetObjectHWater(x,y);                    
    
   } 

   if (!LandingList.PCount) {
	   LandingList.list[LandingList.PCount].x = 256;
	   LandingList.list[LandingList.PCount].y = 256;
	   LandingList.PCount=1;
   }

   if (TrophyMode) {
       LandingList.PCount = 0;
	   for (x=0; x<6; x++) { 
		   LandingList.list[LandingList.PCount].x = 69 + x*3;
		   LandingList.list[LandingList.PCount].y = 66;
		   LandingList.PCount++;
	   }

	   for (y=0; y<6; y++) { 
		   LandingList.list[LandingList.PCount].x = 87;
		   LandingList.list[LandingList.PCount].y = 69 + y*3;
		   LandingList.PCount++;
	   }

	   for (x=0; x<6; x++) { 
		   LandingList.list[LandingList.PCount].x = 84 - x*3;
		   LandingList.list[LandingList.PCount].y = 87;
		   LandingList.PCount++;
	   }

	   for (y=0; y<6; y++) { 
		   LandingList.list[LandingList.PCount].x = 66;
		   LandingList.list[LandingList.PCount].y = 84 - y*3;
		   LandingList.PCount++;
	   }
   }


}



void GenerateMipMap(WORD* A, WORD* D, int L)
{ 
  for (int y=0; y<L; y++)
   for (int x=0; x<L; x++) {
	int C1 = *(A + x*2 +   (y*2+0)*2*L);
	int C2 = *(A + x*2+1 + (y*2+0)*2*L);
	int C3 = *(A + x*2 +   (y*2+1)*2*L);
	int C4 = *(A + x*2+1 + (y*2+1)*2*L);
	//C4 = C1;
	/*
    if (L==64)
     C3=((C3 & 0x7bde) +  (C1 & 0x7bde))>>1;      
     */
	int B = ( ((C1>>0) & 31) + ((C2>>0) & 31) + ((C3>>0) & 31) + ((C4>>0) & 31) +2 ) >> 2;
    int G = ( ((C1>>5) & 31) + ((C2>>5) & 31) + ((C3>>5) & 31) + ((C4>>5) & 31) +2 ) >> 2;
    int R = ( ((C1>>10) & 31) + ((C2>>10) & 31) + ((C3>>10) & 31) + ((C4>>10) & 31) +2 ) >> 2;
	*(D + x + y * L) = HiColor(R,G,B);
   }
}


int CalcColorSum(WORD* A, int L)
{
  int R = 0, G = 0, B = 0;
  for (int x=0; x<L; x++) {
	B+= (*(A+x) >> 0) & 31;
	G+= (*(A+x) >> 5) & 31;
	R+= (*(A+x) >>10) & 31;
  }
  return HiColor(R/L, G/L, B/L);
}


void GenerateShadedMipMap(WORD* src, WORD* dst, int L)
{
  for (int x=0; x<16*16; x++) {
	int B = (*(src+x) >> 0) & 31;
	int G = (*(src+x) >> 5) & 31;
	int R = (*(src+x) >>10) & 31;
	R=DitherHi(SkyR*L/8 + R*(256-L)+6);
	G=DitherHi(SkyG*L/8 + G*(256-L)+6);
	B=DitherHi(SkyB*L/8 + B*(256-L)+6);
	*(dst + x) = HiColor(R,G,B);
  }
}


void GenerateShadedSkyMipMap(WORD* src, WORD* dst, int L)
{
  for (int x=0; x<128*128; x++) {
	int B = (*(src+x) >> 0) & 31;
	int G = (*(src+x) >> 5) & 31;
	int R = (*(src+x) >>10) & 31;
	R=DitherHi(SkyR*L/8 + R*(256-L)+6);
	G=DitherHi(SkyG*L/8 + G*(256-L)+6);
	B=DitherHi(SkyB*L/8 + B*(256-L)+6);
	*(dst + x) = HiColor(R,G,B);
  }
}


void DATASHIFT(WORD* d, int cnt)
{
  if (HARD3D) return;
  cnt>>=1;
  for (int l=0; l<cnt; l++) 
	  *(d+l)*=2;

}



void ApplyAlphaFlags(WORD* tptr, int cnt)
{
#ifdef _d3d
	for (int w=0; w<cnt; w++)
		*(tptr+w)|=0x8000;
#endif
}

void LoadTexture(TEXTURE* &T)
{
    T = (TEXTURE*) _HeapAlloc(Heap, 0, sizeof(TEXTURE));     	
    DWORD L;        	    
    ReadFile(hfile, T->DataA, 128*128*2, &L, NULL);
	for (int y=0; y<128; y++)
	    for (int x=0; x<128; x++)
			if (!T->DataA[y*128+x]) T->DataA[y*128+x]=1;
		
            
	GenerateMipMap(T->DataA, T->DataB, 64);
	GenerateMipMap(T->DataB, T->DataC, 32);
	GenerateMipMap(T->DataC, T->DataD, 16);	
	memcpy(T->SDataC[0], T->DataC, 32*32*2);
	memcpy(T->SDataC[1], T->DataC, 32*32*2);

	DATASHIFT((unsigned short *)T, sizeof(TEXTURE));
    for (int w=0; w<32*32; w++)
	 T->SDataC[1][w] = FadeTab[16][T->SDataC[1][w]>>1];

	ApplyAlphaFlags(T->DataA, 128*128);
	ApplyAlphaFlags(T->DataB, 64*64);
	ApplyAlphaFlags(T->DataC, 32*32);
}




void LoadSky()
{	        
    ReadFile(hfile, SkyPic, 256*256*2, &l, NULL);
    
    for (int y=0; y<128; y++)
        for (int x=0; x<128; x++) 
            SkyFade[0][y*128+x] = SkyPic[y*2*256  + x*2];         

    for (int l=1; l<8; l++)
       GenerateShadedSkyMipMap(SkyFade[0], SkyFade[l], l*32-16);
    GenerateShadedSkyMipMap(SkyFade[0], SkyFade[8], 250);
	ApplyAlphaFlags(SkyPic, 256*256);
}


void LoadSkyMap()
{	        
    ReadFile(hfile, SkyMap, 128*128, &l, NULL);
}


void CalcLights(TModel* mptr);

void fp_conv(LPVOID d)
{
	int i;
	float f;
	memcpy(&i, d, 4);
	f = ((float)i) / 256.f;
	memcpy(d, &f, 4);
}



void CorrectModel(TModel *mptr)
{
	TFace tface[1024];

    for (int f=0; f<mptr->FCount; f++) {	 
     if (!(mptr->gFace[f].Flags & sfDoubleSide))
        mptr->gFace[f].Flags |= sfNeedVC;
#ifdef _d3d
	 fp_conv(&mptr->gFace[f].tax);
     fp_conv(&mptr->gFace[f].tay);
     fp_conv(&mptr->gFace[f].tbx);
     fp_conv(&mptr->gFace[f].tby);
     fp_conv(&mptr->gFace[f].tcx);
     fp_conv(&mptr->gFace[f].tcy);
#else
     mptr->gFace[f].tax = (mptr->gFace[f].tax<<16) + 0x8000;
     mptr->gFace[f].tay = (mptr->gFace[f].tay<<16) + 0x8000;
     mptr->gFace[f].tbx = (mptr->gFace[f].tbx<<16) + 0x8000;
     mptr->gFace[f].tby = (mptr->gFace[f].tby<<16) + 0x8000;
     mptr->gFace[f].tcx = (mptr->gFace[f].tcx<<16) + 0x8000;
     mptr->gFace[f].tcy = (mptr->gFace[f].tcy<<16) + 0x8000;
#endif
    }

	int fp = 0;
    for (f=0; f<mptr->FCount; f++) 
		if ( (mptr->gFace[f].Flags & (sfOpacity | sfTransparent))==0)
		{
			tface[fp] = mptr->gFace[f];
            fp++;
		}

	for (f=0; f<mptr->FCount; f++) 
		if ( (mptr->gFace[f].Flags & (sfOpacity | sfTransparent))!=0)
		{
			tface[fp] = mptr->gFace[f];
            fp++;
		}
    
    memcpy( mptr->gFace, tface, sizeof(tface) );
}

void LoadModel(TModel* &mptr)
{         
    mptr = (TModel*) _HeapAlloc(Heap, 0, sizeof(TModel));

    ReadFile( hfile, &mptr->VCount,      4,         &l, NULL );
	ReadFile( hfile, &mptr->FCount,      4,         &l, NULL );
    ReadFile( hfile, &OCount,            4,         &l, NULL );
	ReadFile( hfile, &mptr->TextureSize, 4,         &l, NULL );
	ReadFile( hfile, mptr->gFace,        mptr->FCount<<6, &l, NULL );
	ReadFile( hfile, mptr->gVertex,      mptr->VCount<<4, &l, NULL );
	ReadFile( hfile, gObj,               OCount*48, &l, NULL );

	if (HARD3D) CalcLights(mptr);

    int ts = mptr->TextureSize;
    
    if (HARD3D) mptr->TextureHeight = 256;
          else  mptr->TextureHeight = mptr->TextureSize>>9;    

    mptr->TextureSize = mptr->TextureHeight*512;

    mptr->lpTexture = (WORD*) _HeapAlloc(Heap, 0, mptr->TextureSize);

    ReadFile(hfile, mptr->lpTexture, ts, &l, NULL);

    for (int v=0; v<mptr->VCount; v++) {
     mptr->gVertex[v].x*=2.f;
     mptr->gVertex[v].y*=2.f;
     mptr->gVertex[v].z*=-2.f;
    }

	CorrectModel(mptr);
        
    DATASHIFT(mptr->lpTexture, mptr->TextureSize);
}



void LoadAnimation(TVTL &vtl)
{
   int vc;
   DWORD l;

   ReadFile( hfile, &vc,          4,    &l, NULL );
   ReadFile( hfile, &vc,          4,    &l, NULL );
   ReadFile( hfile, &vtl.aniKPS,  4,    &l, NULL );
   ReadFile( hfile, &vtl.FramesCount,  4,    &l, NULL );
   vtl.FramesCount++;

   vtl.AniTime = (vtl.FramesCount * 1000) / vtl.aniKPS;
   vtl.aniData = (short int*) 
    _HeapAlloc(Heap, 0, (vc*vtl.FramesCount*6) );
   ReadFile( hfile, vtl.aniData, (vc*vtl.FramesCount*6), &l, NULL);

}



void LoadModelEx(TModel* &mptr, char* FName)
{    
    
    hfile = CreateFile(FName,
        GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hfile==INVALID_HANDLE_VALUE) {		
        char sz[512];
        wsprintf( sz, "Error opening file\n%s.", FName );
		DoHalt(sz);        
    }

    mptr = (TModel*) _HeapAlloc(Heap, 0, sizeof(TModel));

    ReadFile( hfile, &mptr->VCount,      4,         &l, NULL );
	ReadFile( hfile, &mptr->FCount,      4,         &l, NULL );
    ReadFile( hfile, &OCount,            4,         &l, NULL );
	ReadFile( hfile, &mptr->TextureSize, 4,         &l, NULL );
	ReadFile( hfile, mptr->gFace,        mptr->FCount<<6, &l, NULL );
	ReadFile( hfile, mptr->gVertex,      mptr->VCount<<4, &l, NULL );
	ReadFile( hfile, gObj,               OCount*48, &l, NULL );

    int ts = mptr->TextureSize;
	if (HARD3D) mptr->TextureHeight = 256;
          else  mptr->TextureHeight = mptr->TextureSize>>9;    
    mptr->TextureSize = mptr->TextureHeight*512;

    mptr->lpTexture = (WORD*) _HeapAlloc(Heap, 0, mptr->TextureSize);

    ReadFile(hfile, mptr->lpTexture, ts, &l, NULL);

    for (int v=0; v<mptr->VCount; v++) {
     mptr->gVertex[v].x*=2.f;
     mptr->gVertex[v].y*=2.f;
     mptr->gVertex[v].z*=-2.f;
    }

	CorrectModel(mptr);
        
    DATASHIFT(mptr->lpTexture, mptr->TextureSize);        
	GenerateModelMipMaps(mptr);
	GenerateAlphaFlags(mptr);
}




void LoadWav(char* FName, TSFX &sfx)
{
  DWORD l;  

  HANDLE hfile = CreateFile(FName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if( hfile==INVALID_HANDLE_VALUE ) {		
        char sz[512];
        wsprintf( sz, "Error opening file\n%s.", FName );
		DoHalt(sz);        
    }
  
   _HeapFree(Heap, 0, (void*)sfx.lpData);
   sfx.lpData = NULL;

   SetFilePointer( hfile, 36, NULL, FILE_BEGIN );

   char c[5]; c[4] = 0;

   for ( ; ; ) {
      ReadFile( hfile, c, 1, &l, NULL );
      if( c[0] == 'd' ) {
         ReadFile( hfile, &c[1], 3, &l, NULL );
         if( !lstrcmp( c, "data" ) ) break;
          else SetFilePointer( hfile, -3, NULL, FILE_CURRENT );
      }
   }

   ReadFile( hfile, &sfx.length, 4, &l, NULL );

   sfx.lpData = (short int*) 
    _HeapAlloc( Heap, 0, sfx.length );
   
   ReadFile( hfile, sfx.lpData, sfx.length, &l, NULL );  
   CloseHandle(hfile);    
}


WORD conv_565(WORD c)
{
	return (c & 31) + ( (c & 0xFFE0) << 1 );
}

void conv_pic(TPicture &pic)
{
	if (!HARD3D) return;
	for (int y=0; y<pic.H; y++)
		for (int x=0; x<pic.W; x++)
			*(pic.lpImage + x + y*pic.W) = conv_565(*(pic.lpImage + x + y*pic.W));
}




void LoadPicture(TPicture &pic, LPSTR pname)
{
    int C;
    byte fRGB[800][3];
    BITMAPFILEHEADER bmpFH;
    BITMAPINFOHEADER bmpIH;
    DWORD l;
    HANDLE hfile;

    hfile = CreateFile(pname, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if( hfile==INVALID_HANDLE_VALUE ) {		
        char sz[512];
        wsprintf( sz, "Error opening file\n%s.", pname );
		DoHalt(sz);        
    }

    ReadFile( hfile, &bmpFH, sizeof( BITMAPFILEHEADER ), &l, NULL );
    ReadFile( hfile, &bmpIH, sizeof( BITMAPINFOHEADER ), &l, NULL );

	
	_HeapFree(Heap, 0, (void*)pic.lpImage);
	pic.lpImage = NULL;
	
	pic.W = bmpIH.biWidth;
    pic.H = bmpIH.biHeight;
	pic.lpImage = (WORD*) _HeapAlloc(Heap, 0, pic.W * pic.H * 2);



    for (int y=0; y<pic.H; y++) {      
      ReadFile( hfile, fRGB, 3*pic.W, &l, NULL );
      for (int x=0; x<pic.W; x++) {     
       C = ((int)fRGB[x][2]/8<<10) + ((int)fRGB[x][1]/8<< 5) + ((int)fRGB[x][0]/8) ;
       *(pic.lpImage + (pic.H-y-1)*pic.W+x) = C;
      }
    }
   
    CloseHandle( hfile );    
}



void LoadPictureTGA(TPicture &pic, LPSTR pname)
{
    DWORD l;
	WORD w,h;
    HANDLE hfile;

	
    hfile = CreateFile(pname, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if( hfile==INVALID_HANDLE_VALUE ) {		
        char sz[512];
        wsprintf( sz, "Error opening file\n%s.", pname );
		DoHalt(sz);        
    }

	SetFilePointer(hfile, 12, 0, FILE_BEGIN);

    ReadFile( hfile, &w, 2, &l, NULL );
    ReadFile( hfile, &h, 2, &l, NULL );

	SetFilePointer(hfile, 18, 0, FILE_BEGIN);
	
	_HeapFree(Heap, 0, (void*)pic.lpImage);
	pic.lpImage = NULL;

	pic.W = w;
    pic.H = h;
	pic.lpImage = (WORD*) _HeapAlloc(Heap, 0, pic.W * pic.H * 2);

    for (int y=0; y<pic.H; y++) 
      ReadFile( hfile, (void*)(pic.lpImage + (pic.H-y-1)*pic.W), 2*pic.W, &l, NULL );
   
    CloseHandle( hfile );    
}



void LoadTextFile(TText &txt, char* FName)
{
	FILE *stream;
    txt.Lines = 0;
	stream = fopen(FName, "r");
    if (!stream) return;
    while (fgets( txt.Text[txt.Lines], 128, stream) ) {
        txt.Text[txt.Lines][ strlen(txt.Text[txt.Lines]) - 1 ] = 0;
		txt.Lines++;
	}

	fclose( stream );
}


void CreateMipMapMT(WORD* dst, WORD* src, int H)
{
    for (int y=0; y<H; y++) 
        for (int x=0; x<127; x++) {
        int C1 = *(src + (x*2+0) + (y*2+0)*256);
        int C2 = *(src + (x*2+1) + (y*2+0)*256);
        int C3 = *(src + (x*2+0) + (y*2+1)*256);
        int C4 = *(src + (x*2+1) + (y*2+1)*256);

        if (!HARD3D) { C1>>=1; C2>>=1; C3>>=1; C4>>=1; }

         /*if (C1 == 0 && C2!=0) C1 = C2;
         if (C1 == 0 && C3!=0) C1 = C3;
         if (C1 == 0 && C4!=0) C1 = C4;*/

         if (C1 == 0) { *(dst + x + y*128) = 0; continue; }

         //C4 = C1; 

         if (!C2) C2=C1;
         if (!C3) C3=C1;
         if (!C4) C4=C1;

         int B = ( ((C1>> 0) & 31) + ((C2 >>0) & 31) + ((C3 >>0) & 31) + ((C4 >>0) & 31) +2 ) >> 2;
         int G = ( ((C1>> 5) & 31) + ((C2 >>5) & 31) + ((C3 >>5) & 31) + ((C4 >>5) & 31) +2 ) >> 2;
         int R = ( ((C1>>10) & 31) + ((C2>>10) & 31) + ((C3>>10) & 31) + ((C4>>10) & 31) +2 ) >> 2;
         if (!HARD3D) *(dst + x + y * 128) = HiColor(R,G,B)*2;
                 else *(dst + x + y * 128) = HiColor(R,G,B);
        }
}



void CreateMipMapMT2(WORD* dst, WORD* src, int H)
{
    for (int y=0; y<H; y++) 
        for (int x=0; x<63; x++) {
        int C1 = *(src + (x*2+0) + (y*2+0)*128);
        int C2 = *(src + (x*2+1) + (y*2+0)*128);
        int C3 = *(src + (x*2+0) + (y*2+1)*128);
        int C4 = *(src + (x*2+1) + (y*2+1)*128);

		if (!HARD3D) { C1>>=1; C2>>=1; C3>>=1; C4>>=1; }         

        if (C1 == 0) { *(dst + x + y*64) = 0; continue; }

        //C2 = C1; 

         if (!C2) C2=C1;
         if (!C3) C3=C1;
         if (!C4) C4=C1;

	     int B = ( ((C1>> 0) & 31) + ((C2 >>0) & 31) + ((C3 >>0) & 31) + ((C4 >>0) & 31) +2 ) >> 2;
         int G = ( ((C1>> 5) & 31) + ((C2 >>5) & 31) + ((C3 >>5) & 31) + ((C4 >>5) & 31) +2 ) >> 2;
         int R = ( ((C1>>10) & 31) + ((C2>>10) & 31) + ((C3>>10) & 31) + ((C4>>10) & 31) +2 ) >> 2;
         if (!HARD3D) *(dst + x + y * 64) = HiColor(R,G,B)*2;
		         else *(dst + x + y * 64) = HiColor(R,G,B);
        }
}



void GetObjectCaracteristics(TModel* mptr, int& ylo, int& yhi)
{
   ylo = 10241024;
   yhi =-10241024;
   for (int v=0; v<mptr->VCount; v++) {
    if (mptr->gVertex[v].y < ylo) ylo = (int)mptr->gVertex[v].y;
    if (mptr->gVertex[v].y > yhi) yhi = (int)mptr->gVertex[v].y;
   }      
   if (yhi<ylo) yhi=ylo+1;   
}



void GenerateAlphaFlags(TModel *mptr)
{
#ifdef _d3d  
	  
  int w;
  BOOL Opacity = FALSE;
  WORD* tptr = mptr->lpTexture;    

  for (w=0; w<mptr->FCount; w++)
	  if ((mptr->gFace[w].Flags & sfOpacity)>0) Opacity = TRUE;

  if (Opacity) {
   for (w=0; w<256*256; w++)
	   if ( *(tptr+w)>0 ) *(tptr+w)=(*(tptr+w)) + 0x8000; }
  else 
   for (w=0; w<256*256; w++)
	 *(tptr+w)=(*(tptr+w)) + 0x8000;

  tptr = mptr->lpTexture2;    
  if (tptr==NULL) return;

  if (Opacity) {
   for (w=0; w<128*128; w++)
	   if ( (*(tptr+w))>0 ) *(tptr+w)=(*(tptr+w)) + 0x8000; }
  else 
   for (w=0; w<128*128; w++)
	 *(tptr+w)=(*(tptr+w)) + 0x8000;

  tptr = mptr->lpTexture3;    
  if (tptr==NULL) return;

  if (Opacity) {
   for (w=0; w<64*64; w++)
	   if ( (*(tptr+w))>0 ) *(tptr+w)=(*(tptr+w)) + 0x8000; }
  else 
   for (w=0; w<64*64; w++)
	 *(tptr+w)=(*(tptr+w)) + 0x8000;

#endif  
}




void GenerateModelMipMaps(TModel *mptr)
{
  int th = (mptr->TextureHeight) / 2;        
  mptr->lpTexture2 = 
       (WORD*) _HeapAlloc(Heap, HEAP_ZERO_MEMORY , (1+th)*128*2);                
  CreateMipMapMT(mptr->lpTexture2, mptr->lpTexture, th);        

  th = (mptr->TextureHeight) / 4;        
  mptr->lpTexture3 = 
       (WORD*) _HeapAlloc(Heap, HEAP_ZERO_MEMORY , (1+th)*64*2);
  CreateMipMapMT2(mptr->lpTexture3, mptr->lpTexture2, th);        
}


void GenerateMapImage()
{
  int YShift = 23;
  int XShift = 11;
  int lsw = MapPic.W;
  for (int y=0; y<256; y++)
   for (int x=0; x<256; x++) {
	int t = TMap1[y<<1][x<<1];
	WORD c;
	if (t) c = Textures[t]->DataC[(y & 31)*32+(x&31)];
	  else c = Textures[t]->DataD[(y & 15)*16+(x&15)];
    if (!HARD3D) c=c>>1; else c=conv_565(c);
	*((WORD*)MapPic.lpImage + (y+YShift)*lsw + x + XShift) = c;
   }
}



void ReleaseResources()
{
	HeapReleased=0;
    for (int t=0; t<255; t++) 
	 if (Textures[t]) {
      _HeapFree(Heap, 0, (void*)Textures[t]);
	  Textures[t] = NULL;
	 } else break;

	
    for (int m=0; m<255; m++) {
     TModel *mptr = MObjects[m].model;
     if (mptr) {
		if (MObjects[m].vtl.FramesCount>0) {
		 _HeapFree(Heap, 0, MObjects[m].vtl.aniData);
		 MObjects[m].vtl.aniData = NULL;
		 }

        _HeapFree(Heap,0,mptr->lpTexture);   mptr->lpTexture  = NULL;
        _HeapFree(Heap,0,mptr->lpTexture2);  mptr->lpTexture2 = NULL;
        _HeapFree(Heap,0,mptr->lpTexture3);  mptr->lpTexture3 = NULL;
        _HeapFree(Heap,0,MObjects[m].model);              
        MObjects[m].model = NULL;
		MObjects[m].vtl.FramesCount = 0;
     } else break;
    }
	
	for (int a=0; a<255; a++) {
	  if (!Ambient[a].sfx.lpData) break;
	  _HeapFree(Heap, 0, Ambient[a].sfx.lpData);	  
	  Ambient[a].sfx.lpData = NULL;
	}

	for (int r=0; r<255; r++) {
		if (!RandSound[r].lpData) break;	  
		_HeapFree(Heap, 0, RandSound[r].lpData);
		RandSound[r].lpData = NULL;	  
		RandSound[r].length = 0;
	}

	
}



void LoadResources()
{
    int tc,mc;
    char MapName[128],RscName[128];
	HeapAllocated=0;
	
    wsprintf(MapName,"%s%s", ProjectName, ".map");
    wsprintf(RscName,"%s%s", ProjectName, ".rsc");

    ReleaseResources();

    hfile = CreateFile(RscName,
        GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hfile==INVALID_HANDLE_VALUE) {
        char sz[512];
        wsprintf( sz, "Error opening resource file\n%s.", RscName );
		DoHalt(sz);                
        return;   }
    
    ReadFile(hfile, &tc, 4, &l, NULL);
    ReadFile(hfile, &mc, 4, &l, NULL);

    ReadFile(hfile, &SkyR, 4, &l, NULL); 
    ReadFile(hfile, &SkyG, 4, &l, NULL); 
    ReadFile(hfile, &SkyB, 4, &l, NULL); 

	ReadFile(hfile, &SkyTR, 4, &l, NULL); 
    ReadFile(hfile, &SkyTG, 4, &l, NULL); 
    ReadFile(hfile, &SkyTB, 4, &l, NULL); 

	/*SkyTR = 80;
    SkyTG =122;
    SkyTB =166;*/
    
    for (int tt=0; tt<tc; tt++) 
        LoadTexture(Textures[tt]);
	

    for (int mm=0; mm<mc; mm++) {
        ReadFile(hfile, &MObjects[mm].info, 64, &l, NULL);
        MObjects[mm].info.Radius*=2;
        MObjects[mm].info.YLo*=2;
        MObjects[mm].info.YHi*=2;
		MObjects[mm].info.linelenght = (MObjects[mm].info.linelenght / 128) * 128;
        LoadModel(MObjects[mm].model);
		if (MObjects[mm].info.flags & ofANIMATED)
		  LoadAnimation(MObjects[mm].vtl);
        GenerateModelMipMaps(MObjects[mm].model);
		GenerateAlphaFlags(MObjects[mm].model);
	}

	

    LoadSky();
    LoadSkyMap();

	int FgCount;
	ReadFile(hfile, &FgCount, 4, &l, NULL); 
    ReadFile(hfile, &FogsList[1], FgCount * sizeof(TFogEntity), &l, NULL);
	
	int RdCount, AmbCount;

	ReadFile(hfile, &RdCount, 4, &l, NULL); 
   	for (int r=0; r<RdCount; r++) {
	  ReadFile(hfile, &RandSound[r].length, 4, &l, NULL); 
	  RandSound[r].lpData = (short int*) _HeapAlloc(Heap,0,RandSound[r].length);	  
	  ReadFile(hfile, RandSound[r].lpData, RandSound[r].length, &l, NULL); 
	}

	ReadFile(hfile, &AmbCount, 4, &l, NULL); 
	for (int a=0; a<AmbCount; a++) {
	  ReadFile(hfile, &Ambient[a].sfx.length, 4, &l, NULL); 
	  Ambient[a].sfx.lpData = (short int*) _HeapAlloc(Heap,0,Ambient[a].sfx.length);	  
	  ReadFile(hfile, Ambient[a].sfx.lpData, Ambient[a].sfx.length, &l, NULL); 

	  ReadFile(hfile, Ambient[a].rdata, sizeof(Ambient[a].rdata), &l, NULL); 
	  ReadFile(hfile, &Ambient[a].RSFXCount, 4, &l, NULL); 
	  ReadFile(hfile, &Ambient[a].AVolume, 4, &l, NULL); 

	  if (Ambient[a].RSFXCount)
		  Ambient[a].RndTime = (Ambient[a].rdata[0].RFreq / 2 + rRand(Ambient[a].rdata[0].RFreq)) * 1000;
	}


    CloseHandle(hfile);
    
    memcpy(Textures[255], 
           Textures[0], 
           sizeof(TEXTURE));
   
//================ Load MAPs file ==================//
    hfile = CreateFile(MapName,
        GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hfile==INVALID_HANDLE_VALUE)
      DoHalt("Error opening map file.");                  

    ReadFile(hfile, HMap, 512*512, &l, NULL);
    ReadFile(hfile, TMap1, 512*512, &l, NULL);
    ReadFile(hfile, TMap2, 512*512, &l, NULL);
    ReadFile(hfile, OMap, 512*512, &l, NULL);
    ReadFile(hfile, FMap, 512*512, &l, NULL);
    ReadFile(hfile, LMap, 512*512, &l, NULL);
    ReadFile(hfile, HMap2, 512*512, &l, NULL);
    ReadFile(hfile, HMapO, 512*512, &l, NULL);
	ReadFile(hfile, FogsMap, 256*256, &l, NULL);
	ReadFile(hfile, AmbMap, 256*256, &l, NULL);
	
	if (FogsList[1].YBegin>1.f)
     for (int x=0; x<255; x++)
 	  for (int y=0; y<255; y++)
	   if (!FogsMap[y][x]) 
		if (HMap[y*2+0][x*2+0]<FogsList[1].YBegin || HMap[y*2+0][x*2+1]<FogsList[1].YBegin || HMap[y*2+0][x*2+2] < FogsList[1].YBegin ||
		    HMap[y*2+1][x*2+0]<FogsList[1].YBegin || HMap[y*2+1][x*2+1]<FogsList[1].YBegin || HMap[y*2+1][x*2+2] < FogsList[1].YBegin ||
			HMap[y*2+2][x*2+0]<FogsList[1].YBegin || HMap[y*2+2][x*2+1]<FogsList[1].YBegin || HMap[y*2+2][x*2+2] < FogsList[1].YBegin)
			  FogsMap[y][x] = 1;
	  
   
    CloseHandle(hfile);
	


//======== load calls ==============//
	char name[128];
    wsprintf(name,"HUNTDAT\\SOUNDFX\\CALLS\\call%d_a.wav", TargetDino+1);
	LoadWav(name, fxCall[0]);

    wsprintf(name,"HUNTDAT\\SOUNDFX\\CALLS\\call%d_b.wav", TargetDino+1);
	LoadWav(name, fxCall[1]);

	wsprintf(name,"HUNTDAT\\SOUNDFX\\CALLS\\call%d_c.wav", TargetDino+1);
	LoadWav(name, fxCall[2]);

	switch (TargetWeapon) {
	case 0:	
		LoadCharacterInfo(Weapon.chinfo, "HUNTDAT\\WEAPON1.CAR");    
		LoadPictureTGA(BulletPic, "HUNTDAT\\MENU\\WEPPIC\\bullet1.tga");
		break;
	case 1:
		LoadCharacterInfo(Weapon.chinfo, "HUNTDAT\\WEAPON2.CAR");    		
		LoadPictureTGA(BulletPic, "HUNTDAT\\MENU\\WEPPIC\\bullet2.tga");
		break;
	case 2:
		LoadCharacterInfo(Weapon.chinfo, "HUNTDAT\\WEAPON3.CAR");    
		LoadPictureTGA(BulletPic, "HUNTDAT\\MENU\\WEPPIC\\bullet3.tga");
		break;
	case 3:
		LoadCharacterInfo(Weapon.chinfo, "HUNTDAT\\WEAPON4.CAR");    
		LoadPictureTGA(BulletPic, "HUNTDAT\\MENU\\WEPPIC\\bullet4.tga");
		break;
	}


	conv_pic(BulletPic);

//======= Post load rendering ==============//
    CreateTMap();
    RenderLightMap();
	PlaceHunter();

	LoadPictureTGA(MapPic, "HUNTDAT\\MENU\\mapframe.tga");        
	conv_pic(MapPic);

    if (TrophyMode)	PlaceTrophy();    
	           else PlaceCharacters();    
	GenerateMapImage();

	if (TrophyMode) LoadPictureTGA(TrophyPic, "HUNTDAT\\MENU\\trophy.tga");
	           else LoadPictureTGA(TrophyPic, "HUNTDAT\\MENU\\trophy_g.tga");
	conv_pic(TrophyPic);



	LockLanding = FALSE;
	Wind.alpha = rRand(1024) * 2.f * pi / 1024.f;
	Wind.speed = 10;
	MyHealth = MAX_HEALTH;
	ShotsLeft = WeapInfo[TargetWeapon].Shots;
	if (ObservMode) ShotsLeft = 0;

	Weapon.state = 0;
	Weapon.FTime = 0;
	PlayerAlpha = 0;
    PlayerBeta  = 0;

    ExpCount = 0;
	BINMODE = FALSE;
	OPTICMODE = FALSE;
	EXITMODE = FALSE;
	PAUSE = FALSE;

	Ship.pos.x = PlayerX;
	Ship.pos.z = PlayerZ;
	Ship.pos.y = GetLandUpH(Ship.pos.x, Ship.pos.z) + 2048;
	Ship.State = -1;
	Ship.tgpos.x = Ship.pos.x;
	Ship.tgpos.z = Ship.pos.z + 60*256;
	Ship.cindex  = -1;
	Ship.tgpos.y = GetLandUpH(Ship.tgpos.x, Ship.tgpos.z) + 2048;
	ShipTask.tcount = 0;

	if (!TrophyMode) {
	  TrophyRoom.Last.smade = 0;
	  TrophyRoom.Last.ssucces = 0;
	  TrophyRoom.Last.path  = 0;
	  TrophyRoom.Last.time  = 0;
	}

	DemoPoint.DemoTime = 0;
	RestartMode = FALSE;
	TrophyTime=0;
	answtime = 0;
	ExitTime = 0;
}




void ReleaseCharacterInfo(TCharacterInfo &chinfo)
{
	if (!chinfo.mptr) return;
	
	_HeapFree(Heap, 0, chinfo.mptr);
	chinfo.mptr = NULL;

	for (int c = 0; c<64; c++) {
     if (!chinfo.Animation[c].aniData) break;
	 _HeapFree(Heap, 0, chinfo.Animation[c].aniData);
     chinfo.Animation[c].aniData = NULL;
	}

	for (c = 0; c<64; c++) {
     if (!chinfo.SoundFX[c].lpData) break;
	 _HeapFree(Heap, 0, chinfo.SoundFX[c].lpData);
     chinfo.SoundFX[c].lpData = NULL;
	}

	chinfo.AniCount = 0;
	chinfo.SfxCount = 0;
}




void LoadCharacterInfo(TCharacterInfo &chinfo, char* FName)
{
   ReleaseCharacterInfo(chinfo);

   HANDLE hfile = CreateFile(FName,
      GENERIC_READ, FILE_SHARE_READ,
	  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

   if (hfile==INVALID_HANDLE_VALUE) {
      char sz[512];
      wsprintf( sz, "Error opening character file:\n%s.", FName );
      DoHalt(sz);
    }

    ReadFile(hfile, chinfo.ModelName, 32, &l, NULL);
    ReadFile(hfile, &chinfo.AniCount,  4, &l, NULL);
    ReadFile(hfile, &chinfo.SfxCount,  4, &l, NULL);

//============= read model =================//

    chinfo.mptr = (TModel*) _HeapAlloc(Heap, 0, sizeof(TModel));

    ReadFile( hfile, &chinfo.mptr->VCount,      4,         &l, NULL );
    ReadFile( hfile, &chinfo.mptr->FCount,      4,         &l, NULL );
    ReadFile( hfile, &chinfo.mptr->TextureSize, 4,         &l, NULL );
    ReadFile( hfile, chinfo.mptr->gFace,        chinfo.mptr->FCount<<6, &l, NULL );
    ReadFile( hfile, chinfo.mptr->gVertex,      chinfo.mptr->VCount<<4, &l, NULL );

    int ts = chinfo.mptr->TextureSize;
	if (HARD3D) chinfo.mptr->TextureHeight = 256;
          else  chinfo.mptr->TextureHeight = chinfo.mptr->TextureSize>>9;    
    chinfo.mptr->TextureSize = chinfo.mptr->TextureHeight*512;

    chinfo.mptr->lpTexture = (WORD*) _HeapAlloc(Heap, 0, chinfo.mptr->TextureSize);    

    ReadFile(hfile, chinfo.mptr->lpTexture, ts, &l, NULL);
    
    DATASHIFT(chinfo.mptr->lpTexture, chinfo.mptr->TextureSize);
    GenerateModelMipMaps(chinfo.mptr);
	GenerateAlphaFlags(chinfo.mptr);
	//ApplyAlphaFlags(chinfo.mptr->lpTexture, 256*256);
	//ApplyAlphaFlags(chinfo.mptr->lpTexture2, 128*128);
//============= read animations =============//
    for (int a=0; a<chinfo.AniCount; a++) {
      ReadFile(hfile, chinfo.Animation[a].aniName, 32, &l, NULL);
      ReadFile(hfile, &chinfo.Animation[a].aniKPS, 4, &l, NULL);
      ReadFile(hfile, &chinfo.Animation[a].FramesCount, 4, &l, NULL);
      chinfo.Animation[a].AniTime = (chinfo.Animation[a].FramesCount * 1000) / chinfo.Animation[a].aniKPS;
      chinfo.Animation[a].aniData = (short int*) 
          _HeapAlloc(Heap, 0, (chinfo.mptr->VCount*chinfo.Animation[a].FramesCount*6) );

      ReadFile(hfile, chinfo.Animation[a].aniData, (chinfo.mptr->VCount*chinfo.Animation[a].FramesCount*6), &l, NULL);
    }

//============= read sound fx ==============//
	BYTE tmp[32];
    for (int s=0; s<chinfo.SfxCount; s++) {
      ReadFile(hfile, tmp, 32, &l, NULL);
      ReadFile(hfile, &chinfo.SoundFX[s].length, 4, &l, NULL);
       chinfo.SoundFX[s].lpData = (short int*) _HeapAlloc(Heap, 0, chinfo.SoundFX[s].length);
      ReadFile(hfile, chinfo.SoundFX[s].lpData, chinfo.SoundFX[s].length, &l, NULL);
    }

   for (int v=0; v<chinfo.mptr->VCount; v++) {
     chinfo.mptr->gVertex[v].x*=2.f;
     chinfo.mptr->gVertex[v].y*=2.f;
     chinfo.mptr->gVertex[v].z*=-2.f;
    }

   CorrectModel(chinfo.mptr);
   
   
   ReadFile(hfile, chinfo.Anifx, 64*4, &l, NULL);
   if (l!=256)
	   for (l=0; l<64; l++) chinfo.Anifx[l] = -1;
   CloseHandle(hfile);
}














void ScrollWater()
{
        int WaterShift = RealTime / 40;
        int xpos = (-WaterShift) & 127;

        for (int y=0; y<128; y++) {          
          int ypos = (y-WaterShift*2) & 127;

          memcpy((void*)&Textures[0]->DataA[y*128], 
                 (void*)&Textures[255]->DataA[ypos*128 + xpos ], (128 - xpos) * 2 );

          if (xpos) 
             memcpy((void*)&Textures[0]->DataA[y*128+(128-xpos)], 
                    (void*)&Textures[255]->DataA[ypos*128], xpos * 2 );
        } 
        
        xpos/=2;

        for (y=0; y<64; y++) {          
          int ypos = (y*2-WaterShift*2) & 127;
          ypos/=2;

          memcpy((void*)&Textures[0]->DataB[y*64], 
                 (void*)&Textures[255]->DataB[ypos*64 + xpos ], (64 - xpos) * 2 );

          if (xpos) 
             memcpy((void*)&Textures[0]->DataB[y*64+(64-xpos)], 
                    (void*)&Textures[255]->DataB[ypos*64], xpos * 2 );
        } 

        xpos/=2;

        for (y=0; y<32; y++) {          
          int ypos = (y*4-WaterShift*2) & 127;
          ypos/=4;

          memcpy((void*)&Textures[0]->DataC[y*32], 
                 (void*)&Textures[255]->DataC[ypos*32 + xpos ], (32 - xpos) * 2 );

          if (xpos) 
             memcpy((void*)&Textures[0]->DataC[y*32+(32-xpos)], 
                    (void*)&Textures[255]->DataC[ypos*32], xpos * 2 );
        }
} 









//================ light map ========================//



void FillVector(int x, int y, Vector3d& v)
{
   v.x = (float)x*256;
   v.z = (float)y*256;
   v.y = (float)((int)HMap[y][x])*ctHScale;
}

BOOL TraceVector(Vector3d v, Vector3d lv)
{
  v.y+=4;  
  NormVector(lv,64);
  for (int l=0; l<32; l++) {
    v.x-=lv.x; v.y-=lv.y/6; v.z-=lv.z;
	if (v.y>255 * ctHScale) return TRUE;
	if (GetLandH(v.x, v.z) > v.y) return FALSE;
  } 
  return TRUE;
}


void AddShadow(int x, int y, int d)
{
  if (x<0 || y<0 || x>511 || y>511) return;
  LMap[y][x]+=d;
  if (LMap[y][x] > 56) LMap[y][x] = 56;
}

void RenderShadowCircle(int x, int y, int R, int D)
{
  int cx = x / 256;
  int cy = y / 256;
  int cr = 1 + R / 256;
  for (int yy=-cr; yy<=cr; yy++)
   for (int xx=-cr; xx<=cr; xx++) {
     int tx = (cx+xx)*256;
     int ty = (cy+yy)*256;
     int r = (int)sqrt( (tx-x)*(tx-x) + (ty-y)*(ty-y) );
     if (r>R) continue;
	 AddShadow(cx+xx, cy+yy, D * (R-r) / R);     
   }
}

void RenderLightMap()
{

  Vector3d lv;
  int x,y;

  lv.x = - 412;
  lv.z = - 412;
  lv.y = - 1024;
  NormVector(lv, 1.0f);
    
  for (y=1; y<ctMapSize-1; y++) 
    for (x=1; x<ctMapSize-1; x++) {
     int ob = OMap[y][x];
     if (ob == 255) continue;

     int l = MObjects[ob].info.linelenght / 128;
     if (l>0) RenderShadowCircle(x*256+128,y*256+128, 256, MObjects[ob].info.lintensity / 2);
     for (int i=1; i<l; i++) {
	   AddShadow(x+i, y+i, MObjects[ob].info.lintensity);              
     }

     l = MObjects[ob].info.linelenght * 2;
     RenderShadowCircle(x*256+128+l,y*256+128+l,
            MObjects[ob].info.circlerad*2,
            MObjects[ob].info.cintensity);  
    }

}





void SaveScreenShot()
 { 
 
    HANDLE hf;                  /* file handle */ 
    BITMAPFILEHEADER hdr;       /* bitmap file-header */ 
    BITMAPINFOHEADER bmi;       /* bitmap info-header */     
    DWORD dwTmp; 
 
    
  //MessageBeep(0xFFFFFFFF);
    CopyHARDToDIB();

    bmi.biSize = sizeof(BITMAPINFOHEADER); 
    bmi.biWidth = WinW;
    bmi.biHeight = WinH;
    bmi.biPlanes = 1; 
    bmi.biBitCount = 24; 
    bmi.biCompression = BI_RGB;   
    
    bmi.biSizeImage = WinW*WinH*3;
    bmi.biClrImportant = 0;    
    bmi.biClrUsed = 0;



    hdr.bfType = 0x4d42;      
    hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + 
                 bmi.biSize + bmi.biSizeImage);  
    hdr.bfReserved1 = 0; 
    hdr.bfReserved2 = 0;      
    hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + 
                    bmi.biSize; 


    char t[12];
    wsprintf(t,"HUNT%004d.BMP",++_shotcounter);
    hf = CreateFile(t,
                   GENERIC_READ | GENERIC_WRITE, 
                   (DWORD) 0, 
                   (LPSECURITY_ATTRIBUTES) NULL, 
                   CREATE_ALWAYS, 
                   FILE_ATTRIBUTE_NORMAL, 
                   (HANDLE) NULL); 
      
    
      
    WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER), (LPDWORD) &dwTmp, (LPOVERLAPPED) NULL);
             
    WriteFile(hf, &bmi, sizeof(BITMAPINFOHEADER), (LPDWORD) &dwTmp, (LPOVERLAPPED) NULL);    
 
    byte fRGB[1024][3];

    for (int y=0; y<WinH; y++) {
     for (int x=0; x<WinW; x++) {
      WORD C = *((WORD*)lpVideoBuf + (WinEY-y)*1024+x);
      fRGB[x][0] = (C       & 31)<<3;
      if (HARD3D) {
       fRGB[x][1] = ((C>> 5) & 63)<<2;
       fRGB[x][2] = ((C>>11) & 31)<<3; 
      } else {
       fRGB[x][1] = ((C>> 5) & 31)<<3;
       fRGB[x][2] = ((C>>10) & 31)<<3; 
      }
     }
     WriteFile( hf, fRGB, 3*WinW, &dwTmp, NULL );     
    }    
 
    CloseHandle(hf);    
  //MessageBeep(0xFFFFFFFF);
} 




HANDLE hlog;
void CreateLog()
{
	//DeleteFile("carnivor.log");
	hlog = CreateFile("carnivor.log", 
		               GENERIC_WRITE, 
					   FILE_SHARE_READ, NULL, 
					   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

#ifdef _d3d
	PrintLog("Carnivores D3D. Build v1.03. Dec. 18 1998.\n");
#endif

#ifdef _3dfx
	PrintLog("Carnivores 3DFX. Build v1.03. Nov. 24 1998.\n");
#endif

#ifdef _soft
	PrintLog("Carnivores Soft. Build v1.03. Nov. 24 1998.\n");
#endif
}


void PrintLog(LPSTR l)
{
	DWORD w;
	
	if (l[strlen(l)-1]==0x0A) {
		BYTE b = 0x0D;
		WriteFile(hlog, l, strlen(l)-1, &w, NULL);
		WriteFile(hlog, &b, 1, &w, NULL);
		b = 0x0A;
		WriteFile(hlog, &b, 1, &w, NULL);
	} else
		WriteFile(hlog, l, strlen(l), &w, NULL);

}

void CloseLog()
{
	CloseHandle(hlog);
}