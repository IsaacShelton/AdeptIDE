
ifeq ($(OS), Windows_NT)
	CC=x86_64-w64-mingw32-gcc
	CXX=x86_64-w64-mingw32-g++
	RES_C=C:/Users/isaac/Projects/mingw64/bin/windres
	WIN_ICON=obj/icon.res
	WIN_ICON_SRC=resource/icon.rc
	LINKER=x86_64-w64-mingw32-g++
else
	CC=gcc
	CXX=g++
	LINKER=g++
endif

CFLAGS=-c -Wall -I"include" -I"src/INSIGHT/include" -O0 # -fmax-errors=5 -Werror
CXXFLAGS=-c -Wall -I"include" -I"src/INSIGHT/include" -std=c++11 -DGLEW_STATIC -DADEPT_INSIGHT_BUILD -O0 # -fmax-errors=5 -Werror
CXXDEBUGFLAGS=-g
LDFLAGS=
SRCDIR=src
OBJDIR=obj
SOURCES=$(wildcard $(SRCDIR)/INTERFACE/*.cpp) $(wildcard $(SRCDIR)/OPENGL/*.cpp) $(wildcard $(SRCDIR)/PROCESS/*.cpp) $(wildcard $(SRCDIR)/UTIL/*.cpp)
C_SOURCES=
OBJECTS=$(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
C_OBJECTS=$(C_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
DEBUG_OBJECTS=$(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/debug/%.o)
INSIGHT=obj/insight.a

ifeq ($(OS), Windows_NT)
EXECUTABLE=bin/AdeptIDE.exe
DEBUG_EXECUTABLE=bin/AdeptIDE_debug.exe
DEPENDENCIES=-Ldependencies -lglfw3 -lgdi32 -lopengl32 -lcomdlg32 -lwsock32 -lws2_32 $(INSIGHT)

develop: directories $(SOURCES) $(C_SOURCES) $(EXECUTABLE)

release: LDFLAGS += -mwindows
release: develop

debug: CFLAGS += -g
debug: directories $(SOURCES) $(C_SOURCES) $(DEBUG_EXECUTABLE)

$(EXECUTABLE): $(INSIGHT) $(OBJECTS) $(C_OBJECTS) $(WIN_ICON)
	$(LINKER) $(LDFLAGS) $(OBJECTS) $(C_OBJECTS) $(WIN_ICON) $(DEPENDENCIES) -o $@

$(DEBUG_EXECUTABLE): $(INSIGHT) $(DEBUG_OBJECTS) $(C_OBJECTS) $(WIN_ICON)
	$(LINKER) $(LDFLAGS) $(DEBUG_OBJECTS) $(C_OBJECTS) $(WIN_ICON) $(DEPENDENCIES) -o $@

directories:
	@if not exist bin mkdir bin
	@if not exist obj mkdir obj
	@if not exist obj\INTERFACE mkdir obj\INTERFACE
	@if not exist obj\debug\INTERFACE mkdir obj\debug\INTERFACE
	@if not exist obj\OPENGL mkdir obj\OPENGL
	@if not exist obj\debug\OPENGL mkdir obj\debug\OPENGL
	@if not exist obj\PROCESS mkdir obj\PROCESS
	@if not exist obj\debug\PROCESS mkdir obj\debug\PROCESS
	@if not exist obj\UTIL mkdir obj\UTIL
	@if not exist obj\debug\UTIL mkdir obj\debug\UTIL
else
EXECUTABLE=bin/AdeptIDE
DEBUG_EXECUTABLE=bin/AdeptIDE_debug
MACOS_APP_EXECUTABLE_LOCATION=bin/AdeptIDE.app/Contents/MacOS/AdeptIDE
DEPENDENCIES=-lglfw -framework Cocoa -framework OpenGL -framework IOKit -framework CoreVideo -framework Foundation $(INSIGHT)
MAC_DIALOG_MM=$(SRCDIR)/UTIL/macdialog.mm
MAC_DIALOG_O=obj/UTIL/macdialog.o

develop: directories $(SOURCES) $(C_SOURCES) $(MAC_DIALOG_MM) $(EXECUTABLE)

release: develop

debug: CFLAGS += -g
debug: directories $(SOURCES) $(C_SOURCES) $(DEBUG_EXECUTABLE)

$(EXECUTABLE): $(INSIGHT)  $(OBJECTS) $(C_OBJECTS) $(MAC_DIALOG_O)
	$(LINKER) $(LDFLAGS) $(OBJECTS) $(C_OBJECTS) $(MAC_DIALOG_O) $(DEPENDENCIES) -o $@

$(DEBUG_EXECUTABLE): $(INSIGHT) $(DEBUG_OBJECTS) $(C_OBJECTS) $(MAC_DIALOG_O)
	$(LINKER) $(LDFLAGS) $(DEBUG_OBJECTS) $(C_OBJECTS) $(MAC_DIALOG_O) $(DEPENDENCIES) -o $@

$(MAC_DIALOG_O): $(MAC_DIALOG_MM)
	$(CC) $(CFLAGS) $(MAC_DIALOG_MM) -o $(MAC_DIALOG_O)

directories:
	@mkdir -p bin
	@mkdir -p obj
	@mkdir -p obj/debug
	@mkdir -p obj/INTERFACE
	@mkdir -p obj/debug/INTERFACE
	@mkdir -p obj/OPENGL
	@mkdir -p obj/debug/OPENGL
	@mkdir -p obj/PROCESS
	@mkdir -p obj/debug/PROCESS
	@mkdir -p obj/UTIL
	@mkdir -p obj/debug/UTIL
endif

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

$(C_OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

$(DEBUG_OBJECTS): $(OBJDIR)/debug/%.o : $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXDEBUGFLAGS) $< -o $@

$(WIN_ICON):
	$(RES_C) -J rc -O coff -i $(WIN_ICON_SRC) -o $(WIN_ICON)

$(INSIGHT): directories
	$(MAKE) -C src/INSIGHT

deepclean: clean
	$(MAKE) clean -C src/INSIGHT

clean:
ifeq ($(OS), Windows_NT)
	del bin\*.exe /Q
	del obj\*.* /Q
	del obj\debug\*.* /Q
	del obj\INTERFACE\*.* /Q
	del obj\OPENGL\*.* /Q
	del obj\PROCESS\*.* /Q
	del obj\UTIL\*.* /Q
	del obj\debug\INTERFACE\*.* /Q
	del obj\debug\OPENGL\*.* /Q
	del obj\debug\PROCESS\*.* /Q
	del obj\debug\UTIL\*.* /Q
else
	rm -f 2> /dev/null bin/*.exe
	rm -f 2> /dev/null obj/*.*
	rm -f 2> /dev/null obj/debug/*.*
	rm -f 2> /dev/null obj/INTERFACE/*.*
	rm -f 2> /dev/null obj/OPENGL/*.*
	rm -f 2> /dev/null obj/PROCESS/*.*
	rm -f 2> /dev/null obj/UTIL/*.*
	rm -f 2> /dev/null obj/debug/INTERFACE/*.*
	rm -f 2> /dev/null obj/debug/OPENGL/*.*
	rm -f 2> /dev/null obj/debug/PROCESS/*.*
	rm -f 2> /dev/null obj/debug/UTIL/*.*
endif
