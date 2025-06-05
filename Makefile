# -----------------------------------------------------------------------------
# Makefile recursivo para MiProyectoVisión
# - Encuentra todos los .cpp en src/ y todos los .h en include/
# - Target "main" compila solo CORE_CPP (Helpers + Vision + main.cpp)
# - Target "all" compila TODO (incluye UI con Qt)
# -----------------------------------------------------------------------------

# 1) Rutas a librerías externas
OpenCvIncludeDir := /usr/include/opencv4
OpenCvLibraryDir := /usr/lib/x86_64-linux-gnu
OpenCvLibraries  := \
    -lopencv_core \
    -lopencv_highgui \
    -lopencv_imgproc \
    -lopencv_imgcodecs \
    -lopencv_video \
    -lopencv_videoio

ItkIncludeDir   := /usr/include/ITK-5.3
ItkLibraryDir   := /usr/lib/x86_64-linux-gnu
ItkLibraries    := \
    -lITKCommon-5.3 \
    -lITKMetaIO-5.3 \
    -lITKIOImageBase-5.3 \
    -lITKIOTIFF-5.3 \
    -lITKIOJPEG-5.3 \
    -lITKIOPNG-5.3 \
    -lITKImageIntensity-5.3 \
    -lITKColormap-5.3 \
    -lITKBiasCorrection-5.3 \
    -lITKConvolution-5.3 \
    -lITKIOSpatialObjects-5.3 \
    -lITKIOStimulate-5.3

# Librería NIfTI y VNL y itksys
ItkNiftiLibrary := -lITKIONIFTI-5.3
ItkVnlLibraries := -litkvnl-5.3 -litkvnl_algo-5.3
ItkSysLibrary   := -litksys-5.3

# 2) Flags de compilación
CXX      := g++
CXXFLAGS := -std=c++17 -Wall -fPIC \
            -I$(OpenCvIncludeDir) \
            -I$(ItkIncludeDir) \
            `pkg-config --cflags Qt5Widgets` \
            -Iinclude

# 3) Flags de enlace para build completo (Qt + OpenCV + ITK + etc.)
LinkerFlags := \
    -L$(OpenCvLibraryDir) $(OpenCvLibraries) \
    -L$(ItkLibraryDir) \
        $(ItkLibraries) \
        $(ItkNiftiLibrary) \
        $(ItkVnlLibraries) \
        $(ItkSysLibrary) \
    `pkg-config --libs Qt5Widgets`

# 4) Flags de enlace para target “main” (solo OpenCV + ITK + NIfTI + VNL + itksys, sin Qt)
LinkerFlagsMain := \
    -L$(OpenCvLibraryDir) $(OpenCvLibraries) \
    -L$(ItkLibraryDir) \
        $(ItkLibraries) \
        $(ItkNiftiLibrary) \
        $(ItkVnlLibraries) \
        $(ItkSysLibrary)

# 5) Carpetas del proyecto
SRC_DIR     := src
INCLUDE_DIR := include
UI_DIR      := ui
OUTPUT_DIR  := output

# 6) Encontrar todos los .cpp y .h recursivamente
ALL_CPP := $(shell find $(SRC_DIR) -type f -name '*.cpp')
ALL_HDR := $(shell find $(INCLUDE_DIR) -type f -name '*.h')

# 7) Separar fuentes de UI (Qt) de las fuentes core
UI_CPP   := $(shell find $(SRC_DIR)/UI -type f -name '*.cpp' 2>/dev/null || echo "")
CORE_CPP := $(filter-out $(UI_CPP), $(ALL_CPP))

# 8) Convertir listas de fuentes a listas de objetos (.o)
ALL_OBJS  := $(ALL_CPP:.cpp=.o)
CORE_OBJS := $(CORE_CPP:.cpp=.o)

# 9) Nombre del ejecutable
TARGET := Proyecto_saquicela

# -----------------------------------------------------------------------------
.PHONY: all main clean

# ============================================================================
# Target “all” → compila TODO: UI + Helpers + Vision + main.cpp (con Qt)
# ============================================================================
all: $(TARGET)

# ----------------------------------------------------------------------------
# 9a) Regla para generar ui/ui_MainWindow.h a partir de ui/MainWindow.ui
# ----------------------------------------------------------------------------
$(UI_DIR)/ui_%.h: $(UI_DIR)/%.ui
	@echo " ---> Generando $@ a partir de $<"
	uic $< -o $@

# ----------------------------------------------------------------------------
# 9b) Compilar cada .cpp → .o (para ALL_OBJS)
#     Depende de todos los headers (helpers, Volumetrics, MainWindow, ui_*.h, etc.)
# ----------------------------------------------------------------------------
%.o: %.cpp $(ALL_HDR) $(shell find $(UI_DIR) -type f -name 'ui_*.h')
	@echo "Compilando $<"
	$(CXX) $(CXXFLAGS) \
	      -I$(INCLUDE_DIR) \
	      -I$(UI_DIR) \
	      -c $< -o $@

# ----------------------------------------------------------------------------
# 9c) Link final: todos los objetos + Qt + OpenCV + ITK
# ----------------------------------------------------------------------------
$(TARGET): $(shell find $(UI_DIR) -type f -name 'ui_*.h') $(ALL_OBJS)
	@echo "Linkeando $@"
	$(CXX) $(ALL_OBJS) -o $@ $(LinkerFlags)

# ============================================================================
# Target “main” → compila SOLO CORE_OBJS (Helpers + Vision + main.cpp),
# sin UI ni Qt. Sirve para depurar la parte de Vision/Itk/OpenCV.
# ============================================================================
main:
	@echo "Compilando sin interfaz Qt (OpenCV + ITK + NIfTI + VNL + itksys) → coreTest …"
	$(CXX) $(CXXFLAGS) \
	      $(CORE_CPP) \
	      -I$(INCLUDE_DIR) \
	      -o coreTest \
	      $(LinkerFlagsMain) \
	      && echo "  → coreTest compilado correctamente." \
	      || (echo "  ¡Error compilando coreTest!" && exit 1)
	@echo "Ejecutando ./coreTest …"
	@./coreTest

# ============================================================================
# Target “clean” → elimina todos los objetos (.o), los headers generados
# por uic ( ui/ui_*.h ), y los ejecutables Proyecto_saquicela y coreTest
# ============================================================================
clean:
	@echo "Limpiando objetos y ejecutables…"
	rm -f $(ALL_OBJS) $(UI_DIR)/ui_*.h $(TARGET) coreTest
	@echo "¡Limpieza completada!"