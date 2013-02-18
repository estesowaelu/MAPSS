# set Microsoft SDK environment varibles
MSDK_DIR  = C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A
MSDK_PATH = $(MSDK_DIR)\bin
MSDK_INC  = $(MSDK_DIR)\include
MSDK_LIB  = $(MSDK_DIR)\lib
 
# set Visual C++ environment variables
VC_DIR  = C:\Program Files (x86)\Microsoft Visual Studio 10.0
VC_PATH = $(VC_DIR)\vc\bin;$(VC_DIR)\common7\ide
VC_INC  = $(VC_DIR)\vc\include
VC_LIB  = $(VC_DIR)\vc\lib
 
# set Cinder environment variables
CI_DIR = C:\Cinder
CI_INC = $(CI_DIR)\include;$(CI_DIR)\boost
CI_LIB = $(CI_DIR)\lib;$(CI_DIR)\lib\msw
 
CI_LINK = cinder.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib
 
# the Cinder project name
PROJECT = CinderMake
 
# append environment variables
PATH    = $(MSDK_PATH);$(VC_PATH);$(PATH)
INCLUDE = .\include;$(CI_INC);$(MSDK_INC);$(VC_INC);$(INCLUDE)
LIB     = $(CI_LIB);$(MSDK_LIB);$(VC_LIB);$(LIB)
 
# set the compiler flags
# nologo    turns off the Windows logo
# EHsc      enables error handling
# DWIN32    defines the WIN32 preprocessor definition
# DUNICODE  defines the UNICODE preprocessor defintion
CFLAGS = $(CFLAGS) /nologo /EHsc /D WIN32 /D UNICODE
 
# set the source, object, and binary directories
S_DIR = .\src
O_DIR = .\src\obj
B_DIR = .\bin
 
#compile
{$(S_DIR)}.cpp{$(O_DIR)}.obj::
  @echo Compiling...
  $(CC) $(CFLAGS) /Fo$(O_DIR)\ /c $<
 
#link
$(PROJECT): $(O_DIR)\*.obj
  @echo Linking...
  link /OUT:$(B_DIR)\$(PROJECT).exe $(CI_LINK) $(O_DIR)\*.obj
 
clean:
  @if exist $(O_DIR) rmdir /S /Q $(O_DIR)
  @if exist $(B_DIR) rmdir /S /Q $(B_DIR)
 
prep:
  set INCLUDE=$(INCLUDE)
  set LIB=$(LIB)
  @if not exist $(O_DIR) mkdir $(O_DIR)
  @if not exist $(B_DIR) mkdir $(B_DIR)
 
build: $(PROJECT)
 
all: clean prep build