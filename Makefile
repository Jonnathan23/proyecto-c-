# -----------------------------------------------------------------------------
# Makefile unificado para MiProyectoVisión
# Usa las rutas de OpenCV e ITK que ya tienes, y añade un target “main”
# para compilar y ejecutar la lógica sin Qt (solo OpenCV + ITK + VNL + itksys + NIfTI).
# -----------------------------------------------------------------------------

# 1) Rutas de OpenCV (tal como las tienes)
OpenCvIncludeDir := /home/visionups/aplicaciones/Librerias/opencv/opencvi/include/opencv4
OpenCvLibraryDir := /home/visionups/aplicaciones/Librerias/opencv/opencvi/lib
OpenCvLibraries  := \
    -lopencv_core \
    -lopencv_highgui \
    -lopencv_imgproc \
    -lopencv_imgcodecs \
    -lopencv_video \
    -lopencv_videoio

# 2) Rutas de ITK 5.3 (tal como las tienes)
ItkIncludeDir    := /usr/include/ITK-5.3
ItkLibraryDir    := /usr/lib/x86_64-linux-gnu

# Módulos que usaremos de ITK 5.3
ItkLibraries     := \
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

#  Agregamos el módulo NIfTI para que NiftiImageIOFactory esté disponible
ItkNiftiLibrary  := -lITKIONIFTI-5.3

#  Añadimos las librerías VNL (necesarias para ciertos algoritmos de ITK)
ItkVnlLibraries  := -litkvnl-5.3 -litkvnl_algo-5.3

#  Añadimos la librería itksys (SystemToolsManager, etc.)
ItkSysLibrary    := -litksys-5.3

# 3) Flags de compilación de C++
CXX              := g++
CXXFLAGS         := -std=c++17 -Wall \
                     -I$(OpenCvIncludeDir) \
                     -I$(ItkIncludeDir) \
                     `pkg-config --cflags Qt5Widgets`

# 4) Flags de enlace para build completo (Qt + OpenCV + ITK)
LinkerFlags      := \
    -L$(OpenCvLibraryDir) $(OpenCvLibraries) \
    -L$(ItkLibraryDir)     \
        $(ItkLibraries) \
        $(ItkNiftiLibrary) \
        $(ItkVnlLibraries) \
        $(ItkSysLibrary) \
    `pkg-config --libs Qt5Widgets`

# 5) Flags de enlace para target “main” (solo OpenCV + ITK + VNL + itksys + NIfTI, sin Qt)
LinkerFlagsMain  := \
    -L$(OpenCvLibraryDir) $(OpenCvLibraries) \
    -L$(ItkLibraryDir)     \
        $(ItkLibraries) \
        $(ItkNiftiLibrary) \
        $(ItkVnlLibraries) \
        $(ItkSysLibrary)

# 6) Estructura de carpetas
SRC_DIR          := src
HELPERS_DIR      := $(SRC_DIR)/Helpers
VISION_DIR       := $(SRC_DIR)/Vision
UI_DIR           := ui

# 7) Archivos fuente y headers
HELPER_SRCS      := $(wildcard $(HELPERS_DIR)/*.cpp)
VISION_SRCS      := $(wildcard $(VISION_DIR)/*.cpp)
UI_SRCS          := $(UI_DIR)/MainWindow.cpp
MAIN_SRC         := $(SRC_DIR)/main.cpp

SOURCES          := $(HELPER_SRCS) $(VISION_SRCS) $(UI_SRCS) $(MAIN_SRC)

HELPER_HEADERS   := $(wildcard $(HELPERS_DIR)/*.h)
VISION_HEADERS   := $(wildcard $(VISION_DIR)/*.h)

# 8) Archivos .ui y headers generados por uic
UI_FILES         := $(UI_DIR)/MainWindow.ui
UIC_HEADERS      := $(UI_FILES:$(UI_DIR)/%.ui=$(UI_DIR)/ui_%.h)

# 9) Cada .cpp produce un .o (para el build con Qt)
OBJECTS          := $(SOURCES:.cpp=.o)

# 10) Nombre del ejecutable completo (con Qt)
TARGET           := MiProyectoQt

# -----------------------------------------------------------------------------
.PHONY: all clean main

# ==============================================
#   Target “all” → compila el proyecto completo
#   (Qt + OpenCV + ITK + itksys + NIfTI)
# ==============================================
all: $(TARGET)

# -----------------------------------------------------------------------------
#   1) Generar headers de Qt a partir de .ui
#      ui/MainWindow.ui → ui/ui_MainWindow.h
# -----------------------------------------------------------------------------
$(UI_DIR)/ui_%.h: $(UI_DIR)/%.ui
	@echo " ---> Generando $@ a partir de $<"
	uic $< -o $@

# -----------------------------------------------------------------------------
#   2) Compilar cada .cpp → .o
#      (depende de los headers generados por uic y los headers Helpers/Vision)
# -----------------------------------------------------------------------------
%.o: %.cpp $(UIC_HEADERS) $(HELPER_HEADERS) $(VISION_HEADERS)
	@echo "Compilando $<"
	$(CXX) $(CXXFLAGS) \
	      -Iinclude \
	      -I$(UI_DIR) \
	      -I$(HELPERS_DIR) \
	      -I$(VISION_DIR) \
	      -c $< -o $@

# -----------------------------------------------------------------------------
#   3) Link final para build completo: unir todos los .o y enlazar Qt, OpenCV, ITK, itksys, NIfTI
# -----------------------------------------------------------------------------
$(TARGET): $(UIC_HEADERS) $(OBJECTS)
	@echo "Linkeando $@"
	$(CXX) $(OBJECTS) -o $@ $(LinkerFlags)

# =============================================================================
#   Target “main” → compilar y ejecutar solo la lógica de Helpers/Vision + main.cpp
#                     (sin Qt, solo OpenCV + ITK + VNL + itksys + NIfTI)
#
#   Genera un ejecutable “coreTest” que es útil para probar la parte de visión
#   sin depender de la interfaz Qt. Si falla algo, no se mezcla con el código Qt.
# =============================================================================
main:
	@echo "Compilando sin interfaz Qt (OpenCV + ITK + itksys + NIfTI) → coreTest …"
	$(CXX) $(CXXFLAGS) \
	      $(HELPER_SRCS) $(VISION_SRCS) $(MAIN_SRC) \
	      -Iinclude -I$(HELPERS_DIR) -I$(VISION_DIR) \
	      -o coreTest \
	      $(LinkerFlagsMain) \
	      && echo "  → coreTest compilado correctamente." \
	      || (echo "  ¡Error compilando coreTest!" && exit 1)
	@echo "Ejecutando ./coreTest …"
	@./coreTest

# -----------------------------------------------------------------------------
#   4) Limpiar binarios, objetos y headers generados por uic
# -----------------------------------------------------------------------------
clean:
	@echo "Limpiando objetos y ejecutables…"
	rm -f $(OBJECTS) $(UI_DIR)/ui_*.h $(TARGET) coreTest
	@echo "¡Limpieza completada!"
