#include "math.h"
#include "windows.h"
#include "winuser.h"

#include "resource.h"

#include "ddraw.h"

#ifdef _d3d
#include <d3d.h>
#endif

#define ctHScale  32
#define PMORPHTIME 256
#define HiColor(R,G,B) ( ((R)<<10) + ((G)<<5) + (B) )

#define TCMAX ((128<<16)-62024)
#define TCMIN ((000<<16)+62024)


#ifdef _MAIN_
 #define _EXTORNOT 
#else
 #define _EXTORNOT extern
#endif

#define pi 3.1415926535f
#define ctMapSize 512




typedef struct tagMessageList {
   int timeleft;
   char mtext[256];
} TMessageList;

typedef struct tagTRGB {
     BYTE B;
     BYTE G;
     BYTE R;
} TRGB;

typedef struct _Animation {
  char aniName[32];
  int aniKPS, FramesCount, AniTime;
  short int* aniData;
} TAni;

typedef struct _VTLdata {  
  int aniKPS, FramesCount, AniTime;
  short int* aniData;
} TVTL;

typedef struct _SoundFX {
  int  length;
  short int* lpData;
} TSFX;

typedef struct _TText {
  int  Lines;
  char Text[64][128];
} TText;


typedef struct _TRD {
  int RNumber, RVolume, RFreq, RTmp;
} TRD;

typedef struct _TAmbient {
  TSFX sfx;
  TRD  rdata[16];
  int  RSFXCount;
  int  AVolume;
  int  RndTime;
} TAmbient;


typedef struct TagTEXTURE  {
  WORD DataA[128*128];
  WORD DataB[64*64];
  WORD DataC[32*32];
  WORD DataD[16*16];
  WORD SDataC[2][32*32];
} TEXTURE;



typedef struct TagVector3d {
 float x,y,z;
} Vector3d;

typedef struct TagPoint3di {
 int x,y,z;
} TPoint3di;

typedef struct TagVector2di {
 int x,y;
} Vector2di;

typedef struct TagVector2df {
 float x,y;
} Vector2df;


typedef struct TagScrPoint {
 int x,y, tx,ty,
	 Light, z, r2, r3;
} ScrPoint;

typedef struct TagMScrPoint {
 int x,y, tx,ty;	 
} MScrPoint;

typedef struct tagClipPlane {
   Vector3d v1,v2,nv;   
} CLIPPLANE;





typedef struct TagEPoint {
  Vector3d v;
  int  DFlags, scrx, scry, Light;
  float Fog;
} EPoint;


typedef struct TagClipPoint {
  EPoint ev;
  float tx, ty;
} ClipPoint;


//================= MODEL ========================
typedef struct _Point3d {
	float x; 
	float y; 
	float z;
	short owner; 
	short hide;
} TPoint3d;



typedef struct _Face {
   int v1, v2, v3;   
#ifdef _d3d
   float tax, tbx, tcx, tay, tby, tcy;
#else
   int   tax, tbx, tcx, tay, tby, tcy;
#endif
   WORD Flags,DMask;
   int Distant, Next, group;
   char reserv[12];  
} TFace;


typedef struct _Facef {
   int v1, v2, v3;   
   float tax, tbx, tcx, tay, tby, tcy;
   WORD Flags,DMask;
   int Distant, Next, group;
   char reserv[12];  
} TFacef;



typedef struct _Obj {
   char OName [32];
   float ox; 
   float oy;
   float oz;
   short owner; 
   short hide;
} TObj;


typedef struct TagMODEL {
    int VCount, FCount, TextureSize, TextureHeight;
    TPoint3d gVertex[1024];    
	union {
     TFace    gFace  [1024];
	 TFacef   gFacef [1024];
	};
    WORD     *lpTexture, *lpTexture2, *lpTexture3;
#ifdef _d3d
	int      VLight[1024];
#else
	float    VLight[1024];
#endif
} TModel;


//=========== END MODEL ==============================//


typedef struct _ObjInfo {
   int  Radius;
   int  YLo, YHi;
   int  linelenght, lintensity;
   int  circlerad, cintensity;
   int  flags;
   int  GrRad;
   int  LastAniTime;
   BYTE res[24];
} TObjInfo;

typedef struct TagObject {
   TObjInfo info;
   TModel  *model;
   TVTL    vtl;
} TObject;


typedef struct _TCharacterInfo {
  char ModelName[32];
  int AniCount,SfxCount;
  TModel* mptr;
  TAni Animation[64];
  TSFX SoundFX[64];
  int  Anifx[64];
} TCharacterInfo;

typedef struct _TWeapon {
  TCharacterInfo chinfo;
  int state, FTime;
  float shakel;
} TWeapon;



typedef struct _TExplosion {
    Vector3d pos, rpos;
    int EType, FTime;
} TExplosion;




typedef struct _TCharacter  {
  int CType;
  TCharacterInfo *pinfo;
  int StateF;
  int State;
  int NoWayCnt, NoFindCnt, AfraidTime, tgtime;
  int PPMorphTime, PrevPhase,PrevPFTime, Phase, FTime;
  
  float vspeed, rspeed, bend, scale;
  int Slide; 
  float slidex, slidez;
  float tgx, tgz;

  Vector3d pos, rpos;
  float tgalpha, alpha, beta, 
        tggamma,gamma, 
        lookx, lookz;
  int Health;
} TCharacter;



typedef struct tagPlayer {  
  BOOL Active;
  unsigned int IPaddr;
  Vector3d pos;
  float alpha, beta, vspeed;
  int kbState;
  char NickName[16];
} TPlayer;


typedef struct _TDemoPoint {
  Vector3d pos;
  int DemoTime, CIndex;
} TDemoPoint;

typedef struct tagLevelDef {  
    char FileName[64];
    char MapName[128];
    DWORD DinosAvail;
    WORD *lpMapImage;
} TLevelDef;


typedef struct tagShipTask {
  int tcount;
  int clist[255];
} TShipTask;

typedef struct tagShip {  
  Vector3d pos, rpos, tgpos, retpos;
  float alpha, tgalpha, speed, rspeed, DeltaY;
  int State, cindex, FTime;
} TShip;


typedef struct tagLandingList {
  int PCount;
  Vector2di list[64];
} TLandingList;


typedef struct _TPlayerR {
	char PName[128];
	int  RegNumber;
	int  Score, Rank;
} TPlayerR;

typedef struct _TTrophyItem {
  int ctype, weapon, phase,
	  height, weight, score,
	  date, time;
  float scale, range;
  int r1, r2, r3, r4;
} TTrophyItem;


typedef struct _TStats {
	int smade, ssucces;
	float path, time;
} TStats;


typedef struct _TTrophyRoom {
  char PlayerName[128];
  int  RegNumber;
  int  Score, Rank;

  TStats Last, Total;
    
  TTrophyItem Body[24];
} TTrophyRoom;



typedef struct _TDinoInfo {
	LPSTR Name;
	int Health0;
	BOOL DangerCall;
	float Mass, Length, Radius, 
		  SmellK, HearK, LookK,
		  ShDelta;
	int   Scale0, ScaleA, BaseScore;
} TDinoInfo;


typedef struct _TWeapInfo {
	LPSTR Name;
	float Power, Prec, Loud, Rate;
	int Shots;
} TWeapInfo;



typedef struct _TFogEntity {
  int fogRGB;
  float YBegin;
  BOOL  Mortal;
  float Transp, FLimit; 
} TFogEntity;


typedef struct _TWind {
   float alpha;
   float speed;
   Vector3d nv;
} TWind;


typedef struct _TPicture {
   int W,H;
   WORD* lpImage;
} TPicture;




//============= functions ==========================//

void HLineTxB( void );
void HLineTxC( void );
void HLineTxGOURAUD( void );


void HLineTxModel25( void );
void HLineTxModel75( void );
void HLineTxModel50( void );

void HLineTxModel3( void );
void HLineTxModel2( void );
void HLineTxModel( void );

void HLineTDGlass75( void );
void HLineTDGlass50( void );
void HLineTDGlass25( void );
void HLineTBGlass25( void );

void SetFullScreen();
void SetVideoMode(int, int);
void SetMenuVideoMode();
void CreateDivTable();
void DrawTexturedFace();
int GetTextW(HDC, LPSTR);
void InitMenu();
void wait_mouse_release();

//============================== render =================================//
void ShowControlElements();
void InsertModelList(TModel* mptr, float x0, float y0, float z0, int light, float al, float bt);
void ProcessMap(int x, int y, int r);

void DrawTPlane(BOOL);
void DrawTPlaneClip(BOOL);
void ClearVideoBuf();
void DrawTrophyText(int, int);
void DrawHMap();
void RenderCharacter(int);
void RenderExplosion(int);
void RenderShip();
void RenderPlayer(int);
void RenderSkyPlane();
void RenderHealthBar();
void Render_Cross(int, int);
void Render_LifeInfo(int);

void RenderModel         (TModel*, float, float, float, int, float, float);
void RenderModelClipWater(TModel*, float, float, float, int, float, float);
void RenderModelClip     (TModel*, float, float, float, int, float, float);
void RenderNearModel     (TModel*, float, float, float, int, float, float);
void DrawPicture         (int x, int y, TPicture &pic);

void InitClips();
void InitDirectDraw();
void WaitRetrace();

//============= Characters =======================
void Characters_AddSecondaryOne(int ctype);
void AddDeadBody(TCharacter *cptr, int);
void PlaceCharacters();
void PlaceTrophy();
void AnimateCharacters();
void MakeNoise(Vector3d, float);
void CheckAfraid();
void CreateChMorphedModel(TCharacter* cptr);
void CreateMorphedObject(TModel* mptr, TVTL &vtl, int FTime);
void CreateMorphedModel(TModel* mptr, TAni *aptr, int FTime);

//=============================== Math ==================================//
void  NormVector(Vector3d&, float); 
float SGN(float);
void  DeltaFunc(float &a, float b, float d);
void  MulVectorsScal(Vector3d&, Vector3d&, float&);
void  MulVectorsVect(Vector3d&, Vector3d&, Vector3d&);
Vector3d SubVectors( Vector3d&, Vector3d& );
Vector3d RotateVector(Vector3d&);
float VectorLength(Vector3d);
int   siRand(int);
int   rRand(int);
void  CalcHitPoint(CLIPPLANE&, Vector3d&, Vector3d&, Vector3d&);
void  ClipVector(CLIPPLANE& C, int vn);
float FindVectorAlpha(float, float);
float AngleDifference(float a, float b);

int   TraceShot(float ax, float ay, float az,
                float &bx, float &by, float &bz);
int   TraceLook(float ax, float ay, float az,
                float bx, float by, float bz);


void CheckCollision(float&, float&);
float CalcFogLevel(Vector3d v);
//=================================================================//
void AddMessage(LPSTR mt);
void CreateTMap();
void ScrollWater();

void LoadSky();
void LoadSkyMap();
void LoadTexture(TEXTURE*&);
void LoadWav(char* FName, TSFX &sfx);
void LoadTextFile(TText &txt, char* FName);

WORD conv_565(WORD c);
void conv_pic(TPicture &pic);
void LoadPicture(TPicture &pic, LPSTR pname);
void LoadPictureTGA(TPicture &pic, LPSTR pname);
void LoadCharacterInfo(TCharacterInfo&, char*);
void LoadModelEx(TModel* &mptr, char* FName);
void LoadModel(TModel*&);
void LoadResources();


void SaveScreenShot();
void CreateWaterTab();
void CreateFadeTab();
void CreateVideoDIB();
void RenderLightMap();

void MulVectorsVect(Vector3d& v1, Vector3d& v2, Vector3d& r );
void MulVectorsScal(Vector3d& v1,Vector3d& v2, float& r);
Vector3d SubVectors( Vector3d& v1, Vector3d& v2 );
void NormVector(Vector3d& v, float Scale);

LPVOID _HeapAlloc(HANDLE hHeap, DWORD dwFlags, DWORD dwBytes);
BOOL _HeapFree(HANDLE hHeap, DWORD  dwFlags, LPVOID lpMem);

//============ game ===========================//
float GetLandH(float, float);
float GetLandOH(int, int);
float GetLandUpH(float, float);
float GetLandQH(float, float);
float GetLandQHNoObj(float, float);
float GetLandHObj(float, float);

void InitEngine();
void ShutDownEngine();
void ProcessSyncro();
void ProcessMenu();
void AddShipTask(int);
void LoadTrophy();
void LoadPlayersInfo();
void SaveTrophy();
void RemoveCurrentTrophy();
void MakeCall();
void MakeShot(float ax, float ay, float az,
              float bx, float by, float bz);

void AnimateProcesses();
void DoHalt(LPSTR);

_EXTORNOT   char logt[128];
void CreateLog();
void PrintLog(LPSTR l);
void CloseLog();

_EXTORNOT   float BackViewR;
_EXTORNOT   int   BackViewRR;
_EXTORNOT   int   UnderWaterT;



//========== common ==================//
_EXTORNOT   HWND    hwndMain;
_EXTORNOT   HANDLE  hInst;
_EXTORNOT   HANDLE  Heap;
_EXTORNOT   HDC     hdcMain, hdcCMain;
_EXTORNOT   BOOL    blActive;
_EXTORNOT   BYTE    KeyboardState[256];
_EXTORNOT   int     KeyFlags, MenuSelect, LastMenuSelect, _shotcounter;

_EXTORNOT   TMessageList MessageList;
_EXTORNOT   char    ProjectName[128];
_EXTORNOT   int     GameState, _GameState, MenuState;
_EXTORNOT   TSFX    fxMenuGo, fxMenuMov, fxCall[3], fxScream[4];
_EXTORNOT   TSFX    fxMenuAmb, fxUnderwater, fxWaterIn, fxWaterOut, fxJump, fxStep[3], fxRun[3];
//========== map =====================//
_EXTORNOT   byte HMap[ctMapSize][ctMapSize];
_EXTORNOT   byte HMap2[ctMapSize][ctMapSize];
_EXTORNOT   byte HMapO[ctMapSize][ctMapSize];
_EXTORNOT   byte FMap[ctMapSize][ctMapSize];
_EXTORNOT   byte LMap[ctMapSize][ctMapSize];
_EXTORNOT   byte TMap1[ctMapSize][ctMapSize];
_EXTORNOT   byte TMap2[ctMapSize][ctMapSize];
_EXTORNOT   byte OMap[ctMapSize][ctMapSize];

_EXTORNOT   byte FogsMap[256][256];
_EXTORNOT   byte AmbMap[256][256];

_EXTORNOT   TFogEntity  FogsList[256];
_EXTORNOT   TWind       Wind;
_EXTORNOT   TShip       Ship;
_EXTORNOT   TShipTask   ShipTask;

_EXTORNOT   int SkyR, SkyG, SkyB, WaterR, WaterG, WaterB, WaterA,
                SkyTR,SkyTG,SkyTB, CurFogColor;
_EXTORNOT   int RandomMap[32][32];

_EXTORNOT   WORD SkyPic[256*256];
_EXTORNOT   WORD SkyFade[9][128*128];
_EXTORNOT   BYTE SkyMap[128*128];

_EXTORNOT   TEXTURE* Textures[256];
_EXTORNOT   TAmbient Ambient[256];
_EXTORNOT   TSFX     RandSound[256];

//========= GAME ====================//
_EXTORNOT int TargetDino, TargetArea, TargetWeapon, 
              TrophyTime, ObservMode, Tranq, ShotsLeft, ObjectsOnLook;

_EXTORNOT Vector3d answpos;
_EXTORNOT int answtime;

_EXTORNOT BOOL ScentMode, CamoMode, RadarMode, LockLanding, TrophyMode;
_EXTORNOT TTrophyRoom TrophyRoom;
_EXTORNOT TPlayerR PlayerR[16];
_EXTORNOT TPicture LandPic,DinoPic,DinoPicM, MapPic, WepPic;
_EXTORNOT TText DinoText, LandText, WepText, 
                RadarText, ScentText, ComfText, TranqText, ObserText;
_EXTORNOT HFONT fnt_BIG, fnt_Small, fnt_Midd;
_EXTORNOT TLandingList LandingList;

//======== MODEL ======================//
_EXTORNOT TObject  MObjects[256];
_EXTORNOT TModel* mptr;
_EXTORNOT TWeapon Weapon;


_EXTORNOT int   OCount, iModelFade, iModelBaseFade, Current;
_EXTORNOT Vector3d  rVertex[1024];
_EXTORNOT TObj      gObj[1024];
_EXTORNOT Vector2di gScrp[1024];

//============= Characters ==============//
_EXTORNOT TPicture  PausePic, ExitPic, TrophyExit, TrophyPic, BulletPic;
_EXTORNOT TModel* SunModel;
_EXTORNOT TModel *CompasModel;
_EXTORNOT TModel *Binocular;
_EXTORNOT TDinoInfo DinoInfo[16];
_EXTORNOT TWeapInfo WeapInfo[8];
_EXTORNOT TCharacterInfo ShipModel;
_EXTORNOT int ChCount,ExpCount,
              ShotDino, TrophyBody;
_EXTORNOT TCharacterInfo WindModel;
_EXTORNOT TCharacterInfo ExplodInfo;
_EXTORNOT TCharacterInfo PlayerInfo;
_EXTORNOT TCharacterInfo ChInfo[64];
_EXTORNOT TCharacter     Characters[256];
_EXTORNOT TExplosion     Explosions[256];
_EXTORNOT TDemoPoint     DemoPoint;

_EXTORNOT TPlayer        Players[16];
_EXTORNOT Vector3d       PlayerPos, CameraPos;

//========== Render ==================//
_EXTORNOT   LPDIRECTDRAW lpDD;
_EXTORNOT   LPDIRECTDRAW2 lpDD2;
//_EXTORNOT   LPDIRECTINPUT lpDI;

_EXTORNOT   void* lpVideoRAM;
_EXTORNOT   LPDIRECTDRAWSURFACE lpddsPrimary;
_EXTORNOT   BOOL DirectActive, FULLSCREEN, RestartMode;
_EXTORNOT   BOOL LoDetailSky;
_EXTORNOT   int  WinW,WinH,WinEX,WinEY,VideoCX,VideoCY,iBytesPerLine,ts,r,MapMinY;
_EXTORNOT   float CameraW,CameraH,Soft_Persp_K, stepdy, stepdd;
_EXTORNOT   CLIPPLANE ClipA,ClipB,ClipC,ClipD,ClipZ,ClipW;
_EXTORNOT   int u,vused, CCX, CCY;

_EXTORNOT   DWORD Mask1,Mask2;


_EXTORNOT   EPoint VMap[128][128];
_EXTORNOT   EPoint VMap2[128][128];
_EXTORNOT   EPoint ev[3];

_EXTORNOT   ClipPoint cp[16];
_EXTORNOT   ClipPoint hleft,hright;

_EXTORNOT   void  *HLineT;
_EXTORNOT   int   rTColor;
_EXTORNOT   int   SKYMin, SKYDTime, FadeL, GlassL, ctViewR, 
                  dFacesCount, ReverseOn, TDirection;
_EXTORNOT   WORD  FadeTab[65][0x8000];

_EXTORNOT   BYTE    MenuMap[300][400];


_EXTORNOT   int     PrevTime, TimeDt, T, Takt, RealTime, StepTime, MyHealth, ExitTime,
                    CallLockTime, NextCall;
_EXTORNOT   float   DeltaT;
_EXTORNOT   float   CameraX, CameraY, CameraZ, CameraAlpha, CameraBeta;
_EXTORNOT   float   PlayerX, PlayerY, PlayerZ, PlayerAlpha, PlayerBeta, 
                    HeadY, HeadBackR, HeadBSpeed, HeadAlpha, HeadBeta,
                    SSpeed,VSpeed,RSpeed,YSpeed;      
_EXTORNOT   Vector3d PlayerNv;

_EXTORNOT   float   ca,sa,cb,sb, wpnDAlpha, wpnDBeta;
_EXTORNOT   void    *lpVideoBuf, *lpMenuBuf, *lpMenuBuf2, *lpTextureAddr;
_EXTORNOT   HBITMAP hbmpVideoBuf, hbmpMenuBuf, hbmpMenuBuf2;
_EXTORNOT   HCURSOR hcArrow;
_EXTORNOT   int     DivTbl[10240];

_EXTORNOT   Vector3d  v[3];
_EXTORNOT   ScrPoint  scrp[3];
_EXTORNOT   MScrPoint mscrp[3];
_EXTORNOT   Vector3d  nv, waterclipbase;


_EXTORNOT   struct _t {
              int fkForward, fkBackward, fkUp, fkDown, fkLeft, fkRight, fkFire, fkShow, fkSLeft, fkSRight, fkStrafe, fkJump, fkRun, fkCrouch, fkCall, fkBinoc;
	} KeyMap;


#define kfForward     0x00000001
#define kfBackward    0x00000002
#define kfLeft        0x00000004
#define kfRight       0x00000008
#define kfLookUp      0x00000010
#define kfLookDn      0x00000020
#define kfJump        0x00000040
#define kfDown        0x00000080
#define kfCall        0x00000100

#define kfSLeft       0x00001000
#define kfSRight      0x00002000
#define kfStrafe      0x00004000

#define fmWater   0x80
#define fmReverse 0x40
#define fmNOWAY   0x20

#define sfDoubleSide         1
#define sfDarkBack           2
#define sfOpacity            4
#define sfTransparent        8
#define sfMortal        0x0010

#define sfNeedVC        0x0080
#define sfDark          0x8000

#define ofPLACEWATER       1
#define ofPLACEGROUND      2
#define ofPLACEUSER        4
#define ofANIMATED         0x80000000

#define csONWATER          0x00010000
#define MAX_HEALTH         128000

#define HUNT_EAT      0
#define HUNT_BREATH   1
#define HUNT_FALL     2
#define HUNT_KILL     3 

_EXTORNOT BOOL WATERANI,Clouds,SKY,GOURAUD,
               MODELS,TIMER,BITMAPP,MIPMAP,
               NOCLIP,CLIP3D,NODARKBACK,CORRECTION, LOWRESTX, 
			   FOGENABLE, FOGON, CAMERAINFOG, 
               WATERREVERSE,waterclip,UNDERWATER, NeedWater,
               SWIM, FLY, PAUSE, OPTICMODE, BINMODE, EXITMODE, MapMode, RunMode;
_EXTORNOT int  CameraFogI;
_EXTORNOT int OptAgres, OptDens, OptSens, OptRes, OptText, OptSys, WaitKey, OPT_ALPHA_COLORKEY;
_EXTORNOT BOOL SHADOWS3D,REVERSEMS;

_EXTORNOT BOOL SLOW, DEBUG, MORPHP, MORPHA;






//========== for audio ==============//
int  AddVoice(int, short int*);
int  AddVoicev(int, short int*, int);
int  AddVoice3dv(int, short int*, float, float, float, int);
int  AddVoice3d(int, short int*, float, float, float);

void SetAmbient3d(int, short int*, float, float, float);
void SetAmbient(int, short int*, int);
void AudioSetCameraPos(float, float, float, float);
void InitAudioSystem(HWND);
void Audio_Restore();
void AudioStop();

//========== for 3d hardware =============//
_EXTORNOT BOOL HARD3D;
void ShowVideo();
void Init3DHardware();
void Activate3DHardware();
void ShutDown3DHardware();
void Render3DHardwarePosts();
void CopyHARDToDIB();
void Hardware_ZBuffer(BOOL zb);

#ifdef _MAIN_
_EXTORNOT char KeysName[256][24] = {
"...",
"Esc",
"1",
"2",
"3",
"4",
"5",
"6",
"7",
"8",
"9",
"0",
"-",
"=",
"BSpace",
"Tab",
"Q",
"W",
"E",
"R",
"T",
"Y",
"U",
"I",
"O",
"P",
"[",
"]",
"Enter",
"Ctrl",
"A",
"S",
"D",
"F",
"G",
"H",
"J",
"K",
"L",
";",
"'",
"~",
"Shift",
"\\",
"Z",
"X",
"C",
"V",
"B",
"N",
"M",
",",
".",
"/",
"Shift",
"*",
"Alt",
"Space",
"CLock",
"F1",
"F2",
"F3",
"F4",
"F5",
"F6",
"F7",
"F8",
"F9",
"F10",
"NLock",
"SLock",
"Home",
"Up",
"PgUp",
"-",
"Left",
"Midle",
"Right",
"+",
"End",
"Down",
"PgDn",
"Ins",
"Del",
"",
"",
"",
"F11",
"F12",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"Mouse1",
"Mouse2",
"Mouse3",
"<?>",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""
	};
#else
   _EXTORNOT char KeysName[128][24];
#endif