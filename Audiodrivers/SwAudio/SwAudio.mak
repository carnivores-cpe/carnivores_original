# Microsoft Developer Studio Generated NMAKE File, Based on SwAudio.dsp
!IF "$(CFG)" == ""
CFG=SwAudio - Win32 Debug
!MESSAGE No configuration specified. Defaulting to SwAudio - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "SwAudio - Win32 Release" && "$(CFG)" !=\
 "SwAudio - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SwAudio.mak" CFG="SwAudio - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SwAudio - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SwAudio - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "SwAudio - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

!IF "$(RECURSE)" == "0" 

ALL : "..\..\MyProject\Massacre\A_Soft.dll"

!ELSE 

ALL : "..\..\MyProject\Massacre\A_Soft.dll"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\Audio.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\A_Soft.exp"
	-@erase "$(OUTDIR)\A_Soft.lib"
	-@erase "..\..\MyProject\Massacre\A_Soft.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /Gz /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\SwAudio.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SwAudio.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib dsound.lib winmm.lib /nologo /subsystem:windows /dll\
 /incremental:no /pdb:"$(OUTDIR)\A_Soft.pdb" /machine:I386 /def:".\audio.def"\
 /out:"D:\msd50\MyProject\Massacre\A_Soft.dll" /implib:"$(OUTDIR)\A_Soft.lib" 
DEF_FILE= \
	".\audio.def"
LINK32_OBJS= \
	"$(INTDIR)\Audio.obj"

"..\..\MyProject\Massacre\A_Soft.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "SwAudio - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\SwAudio.dll"

!ELSE 

ALL : "$(OUTDIR)\SwAudio.dll"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\Audio.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\SwAudio.dll"
	-@erase "$(OUTDIR)\SwAudio.exp"
	-@erase "$(OUTDIR)\SwAudio.ilk"
	-@erase "$(OUTDIR)\SwAudio.lib"
	-@erase "$(OUTDIR)\SwAudio.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\SwAudio.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SwAudio.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)\SwAudio.pdb" /debug /machine:I386 /def:".\audio.def"\
 /out:"$(OUTDIR)\SwAudio.dll" /implib:"$(OUTDIR)\SwAudio.lib" /pdbtype:sept 
DEF_FILE= \
	".\audio.def"
LINK32_OBJS= \
	"$(INTDIR)\Audio.obj"

"$(OUTDIR)\SwAudio.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "SwAudio - Win32 Release" || "$(CFG)" ==\
 "SwAudio - Win32 Debug"
SOURCE=.\Audio.cpp

!IF  "$(CFG)" == "SwAudio - Win32 Release"

DEP_CPP_AUDIO=\
	".\Audio.h"\
	

"$(INTDIR)\Audio.obj" : $(SOURCE) $(DEP_CPP_AUDIO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "SwAudio - Win32 Debug"


"$(INTDIR)\Audio.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 

