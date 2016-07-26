###############################################################################
# Targets available:
# - make release (or just run "make")
# - make debug
# - make test
# - make windows-cross-compile-release (Windows cross compilation using mingw)
# - make osx-release
# - make osx-debug
# - make clean
#
# NOTE: Building on Windows via a Makefile is currently NOT supported (although
#       perhaps it will be in the future), use the Code::Blocks project for this
#       (see README.md for instructions).
#

###############################################################################
# Directories
###############################################################################
SRC_DIR            = src
INC_DIR            = include
TARGET_DIR         = target
OBJ_DIR            = obj
ASSETS_DIR         = assets
RL_UTILS_DIR       = rl_utils
RL_UTILS_SRC_DIR   = $(RL_UTILS_DIR)/src
RL_UTILS_INC_DIR   = $(RL_UTILS_DIR)/include
TEST_DIR           = test
TEST_SRC_DIR       = $(TEST_DIR)/src

# Only used for Windows cross compilation on Linux
SDL_BASE_DIR       = SDL
SDL_DIR            = $(SDL_BASE_DIR)/SDL2-2.0.4
SDL_IMAGE_DIR      = $(SDL_BASE_DIR)/SDL2_image-2.0.1
SDL_MIXER_DIR      = $(SDL_BASE_DIR)/SDL2_mixer-2.0.1

SDL_ARCH           = i686-w64-mingw32

SDL_INC_DIR        = $(SDL_DIR)/$(SDL_ARCH)/include
SDL_IMAGE_INC_DIR  = $(SDL_IMAGE_DIR)/$(SDL_ARCH)/include
SDL_MIXER_INC_DIR  = $(SDL_MIXER_DIR)/$(SDL_ARCH)/include

SDL_LIB_DIR        = $(SDL_DIR)/$(SDL_ARCH)/lib
SDL_IMAGE_LIB_DIR  = $(SDL_IMAGE_DIR)/$(SDL_ARCH)/lib
SDL_MIXER_LIB_DIR  = $(SDL_MIXER_DIR)/$(SDL_ARCH)/lib

SDL_BIN_DIR        = $(SDL_DIR)/$(SDL_ARCH)/bin
SDL_IMAGE_BIN_DIR  = $(SDL_IMAGE_DIR)/$(SDL_ARCH)/bin
SDL_MIXER_BIN_DIR  = $(SDL_MIXER_DIR)/$(SDL_ARCH)/bin


###############################################################################
# Common setup
###############################################################################
# Includes
INCLUDES = \
  -I $(INC_DIR) \
  -I $(RL_UTILS_INC_DIR) \
  #

# Compiler flags
# NOTE:
# - The debug trace system generates warnings for trace levels higher than the
#   compiled level, so sadly Wunused-value cannot be used as it is.
# - The Mersenne Twister implementation in mersenne_twister.hpp gives warnings
#   about deprecated register functionality, so this warning needs to be
#   suppressed as long as that implementation is used (should use the now
#   standard C++ implementation instead though).
CXXFLAGS += \
  -std=c++11 \
  -Wall \
  -Wextra \
  -Werror \
  -Wno-unused-value \
  -fno-rtti \
  -fno-exceptions \
  -Wno-deprecated-register \
  #

# Linker flags
LD_FLAGS =


###############################################################################
# Linux specific
###############################################################################
# Compiler for linux builds
release debug test: CXX ?= g++

# Linux specific compiler flags
release debug test: CXXFLAGS += $(shell sdl2-config --cflags)

# Linux release specific compiler flags
release: CXXFLAGS += \
  -O2 \
  -DNDEBUG \
  #

# Linux debug and test specific compiler flags
debug test: CXXFLAGS += \
  -O0 \
  -g \
  #

# Linux specific linker flags
release debug test: LD_FLAGS = \
  $(shell sdl2-config --libs) \
  -lSDL2_image \
  -lSDL2_mixer \
  #

# Executable
LINUX_EXE = $(TARGET_DIR)/ia

# Test executable
LINUX_TEST_EXE = $(TARGET_DIR)/test


###############################################################################
# Windows cross compilation specific
###############################################################################
# Cross compiler to build Windows releases on Linux
windows-cross-compile-release: CXX = i686-w64-mingw32-g++-win32

# Windows specific includes
windows-cross-compile-release: INCLUDES += \
  -I $(SDL_INC_DIR) \
  -I $(SDL_IMAGE_INC_DIR) \
  -I $(SDL_MIXER_INC_DIR) \
  #

# Windows specific compiler flags
windows-cross-compile-release: CXXFLAGS += \
  -O2 \
  -DNDEBUG \
  #

# Windows specific linker flags
windows-cross-compile-release: LD_FLAGS += \
  -L $(SDL_LIB_DIR) \
  -L $(SDL_IMAGE_LIB_DIR) \
  -L $(SDL_MIXER_LIB_DIR) \
  -lSDL2 \
  -lSDL2main \
  -lSDL2_image \
  -lSDL2_mixer \
  -static-libgcc \
  -static-libstdc++ \
  #

WINDOWS_EXE = $(TARGET_DIR)/ia.exe


###############################################################################
# Mac OS X
###############################################################################
osx-release osx-debug: CXX ?= c++

osx-release osx-debug: INCLUDES += \
  -F/Library/Frameworks \
  -I/Library/Frameworks/SDL2.framework/Headers \
  -I/Library/Frameworks/SDL2_image.framework/Headers \
  -I/Library/Frameworks/SDL2_mixer.framework/Headers \
  #

osx-release osx-debug: LD_FLAGS = \
  -F/Library/Frameworks \
  -framework Cocoa \
  -framework SDL2 \
  -framework SDL2_image \
  -framework SDL2_mixer \
  #

osx-release osx-debug: CXXFLAGS += \
  -DMACOSX \
  #

osx-release: CXXFLAGS += \
  -O2 \
  -DNDEBUG \
  #

osx-debug: CXXFLAGS += \
  -O0 \
  -g \
  #


###############################################################################
# Common sources files
###############################################################################
SRC              = $(wildcard $(SRC_DIR)/*.cpp)
RL_UTILS_SRC     = $(wildcard $(RL_UTILS_SRC_DIR)/*.cpp)


###############################################################################
# Test source files
###############################################################################
TEST_SRC         = $(wildcard $(TEST_SRC_DIR)/*.cpp)


###############################################################################
# Object files
###############################################################################
OBJECTS          = $(SRC:$(SRC_DIR)%.cpp=$(OBJ_DIR)%.o)
RL_UTILS_OBJECTS = $(RL_UTILS_SRC:$(RL_UTILS_SRC_DIR)%.cpp=$(OBJ_DIR)%.o)
TEST_OBJECTS     = $(TEST_SRC:$(TEST_SRC_DIR)%.cpp=$(OBJ_DIR)%.o)


###############################################################################
# Targets and recipes
###############################################################################
all: release

release debug: $(LINUX_EXE)

test : $(LINUX_TEST_EXE)

# The Windows version needs to copy some DLLs and licenses
windows-cross-compile-release: $(WINDOWS_EXE)
	cp \
	  $(SDL_BIN_DIR)/SDL2.dll \
	  $(SDL_IMAGE_BIN_DIR)/SDL2_image.dll \
	  $(SDL_IMAGE_BIN_DIR)/zlib1.dll \
	  $(SDL_IMAGE_BIN_DIR)/libpng16-16.dll \
	  $(SDL_IMAGE_BIN_DIR)/LICENSE.zlib.txt \
	  $(SDL_IMAGE_BIN_DIR)/LICENSE.png.txt \
	  $(SDL_MIXER_BIN_DIR)/SDL2_mixer.dll \
	  $(SDL_MIXER_BIN_DIR)/libogg-0.dll \
	  $(SDL_MIXER_BIN_DIR)/libvorbis-0.dll \
	  $(SDL_MIXER_BIN_DIR)/libvorbisfile-3.dll \
	  $(SDL_MIXER_BIN_DIR)/LICENSE.ogg-vorbis.txt \
	  $(TARGET_DIR)

osx-release osx-debug: $(LINUX_EXE)

$(LINUX_EXE) $(WINDOWS_EXE): $(OBJECTS) $(RL_UTILS_OBJECTS)
	mkdir -p $(TARGET_DIR)
	$(CXX) $^ -o $@ $(LD_FLAGS)
	cp -r $(ASSETS_DIR)/* $(TARGET_DIR)

define compile-object
mkdir -p $(OBJ_DIR)
$(CXX) -c $(CXXFLAGS) $(INCLUDES) $< -o $@
endef

# Compiling common objects
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(compile-object)

# Compiling rl utils objects
$(OBJ_DIR)/%.o: $(RL_UTILS_SRC_DIR)/%.cpp | check-rl-utils
	$(compile-object)

# Compiling test objects
$(OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.cpp
	$(compile-object)

# Make sure the RL Utils submodule exists
check-rl-utils :
	@if [ -z "$(RL_UTILS_SRC)" ]; then \
	  echo ""; \
	  echo "***********************************************************"; \
	  echo "Error: No source files found in Git submodule \"$(RL_UTILS_DIR)\"!"; \
	  echo ""; \
	  echo "Please execute the following before building IA:"; \
	  echo ""; \
	  echo "  $$ git submodule init"; \
	  echo "  $$ git submodule update"; \
	  echo ""; \
	  echo "(Or use the equivalent GUI function)"; \
	  echo ""; \
	  echo "***********************************************************"; \
	  echo ""; \
	  exit 1; \
	fi

# Optional auto dependency tracking
#  -include depends.mk

# depends: $(DEPENDS)

# %.d:
# 	$(CXX) -MM $(CXXFLAGS) $(INCLUDES) $(@:.d=.cpp) -MF depends.tmp -MT$(@:.d=.o)
# 	cat depends.tmp >> depends.mk
# 	rm -rf depends.tmp

# clean-depends:
# 	rm -rf depends.mk

clean:
	rm -rf $(OBJ_DIR) $(TARGET_DIR)

# Phony targets
.PHONY: all clean check-rl-utils
