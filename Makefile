# -----------------------------------------------------------------------------
# Makefile recursivo para MiProyectoVisión (Qt + OpenCV + ITK)
#
# - Genera automáticamente los archivos moc_*.cpp para clases con Q_OBJECT
# - Encuentra todos los .cpp en src/ y todos los .h en include/
# - Target "main" compila solo CORE_CPP (Helpers + Vision + main.cpp) sin Qt
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
    -lopencv_videoio \
    -lopencv_photo

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

# Librería NIfTI, VNL y itksys
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
MOC_DIR     := src       # GENERAR moc_MainWindow.cpp dentro de src
OUTPUT_DIR  := output

# 6) Encontrar todos los .cpp y .h recursivamente
ALL_CPP := $(shell find $(SRC_DIR) -type f -name '*.cpp')
ALL_HDR := $(shell find $(INCLUDE_DIR) -type f -name '*.h')

# 7) Fuente Qt (moc) para MainWindow
#    Si tuvieras más clases con Q_OBJECT, añadirías aquí más reglas similares
MOC_HDR     := include/MainWindow.h
MOC_SRC     := src/moc_MainWindow.cpp

# 8) Separar fuentes core de las fuentes Qt (src/UI no existe, pero lo dejamos para compatibilidad)
UI_CPP   := $(shell find $(SRC_DIR)/UI -type f -name '*.cpp' 2>/dev/null || echo "")
CORE_CPP := $(filter-out $(UI_CPP), $(ALL_CPP))

# 9) Convertir listas de fuentes a listas de objetos (.o)
#    Incluimos moc_MainWindow.o en ALL_OBJS y CORE_OBJS si hace falta
ALL_OBJS  := $(ALL_CPP:.cpp=.o) $(MOC_SRC:.cpp=.o)
CORE_OBJS := $(CORE_CPP:.cpp=.o)

# 10) Nombre del ejecutable
TARGET := Proyecto_saquicela

# -----------------------------------------------------------------------------
.PHONY: all main clean

# ============================================================================
# Target “all” → compila TODO: UI + Helpers + Vision + main.cpp (con Qt)
# ============================================================================
all: $(TARGET)

# ----------------------------------------------------------------------------
# 10a) Regla para generar ui/ui_MainWindow.h a partir de ui/MainWindow.ui
# ----------------------------------------------------------------------------
$(UI_DIR)/ui_%.h: $(UI_DIR)/%.ui
	@echo " ---> Generando $@ a partir de $<"
	uic $< -o $@

# ----------------------------------------------------------------------------
# 10b) Regla para generar el archivo moc_MainWindow.cpp a partir de MainWindow.h
#      (necesario para señales/slots)
# ----------------------------------------------------------------------------
$(MOC_SRC): $(MOC_HDR)
	@echo " ---> Generando MOC para MainWindow.h -> $@"
	moc $(MOC_HDR) -o $@

# ----------------------------------------------------------------------------
# 10c) Compilar cada .cpp → .o (incluye MOC_SRC)
#      Depende de todos los headers y de cualquier ui_*.h generado
# ----------------------------------------------------------------------------
%.o: %.cpp $(ALL_HDR) $(shell find $(UI_DIR) -type f -name 'ui_*.h')
	@echo "Compilando $<"
	$(CXX) $(CXXFLAGS) \
	      -I$(INCLUDE_DIR) \
	      -I$(UI_DIR) \
	      -c $< -o $@

# ----------------------------------------------------------------------------
# 10d) Link final: todos los objetos (incluido moc_MainWindow.o) + Qt + OpenCV + ITK
# ----------------------------------------------------------------------------
$(TARGET): $(shell find $(UI_DIR) -type f -name 'ui_*.h') $(ALL_OBJS)
	@echo "Linkeando $@"
	$(CXX) $(ALL_OBJS) -o $@ $(LinkerFlags)

# ============================================================================
# Target “main” → compila SOLO CORE_CPP (Helpers + Vision + main.cpp),
# sin UI ni Qt (no incluye moc ni ui_*.h). Sirve para depurar la parte
# de Visión/ITK/OpenCV sin instalar Qt.
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
# por uic ( ui/ui_*.h ) y moc ( src/moc_*.cpp ), y los ejecutables Proyecto_saquicela y coreTest
# ============================================================================

run: all
	@echo "Ejecutando Proyecto_saquicela…"
	@./Proyecto_saquicela

clean:
	@echo "Limpiando objetos y ejecutables…"
	rm -f $(ALL_OBJS) $(UI_DIR)/ui_*.h $(MOC_SRC:.cpp=.o) $(MOC_SRC) $(TARGET) coreTest
	@echo "¡Limpieza completada!"
