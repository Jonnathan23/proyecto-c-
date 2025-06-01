# -------------------------------------------------------
# Makefile para compilar tu proyecto con OpenCV y con ITK (sin usar CMake)
# -------------------------------------------------------

# ────────────────────────────────────────────────────────
# 1) Directorios y bibliotecas de OpenCV
# ────────────────────────────────────────────────────────

# Carpeta donde están los headers de OpenCV (include opencv4)
OpenCvIncludeDir := /home/visionups/aplicaciones/Librerias/opencv/opencvi/include/opencv4

# Carpeta donde están las librerías .so de OpenCV
OpenCvLibraryDir := /home/visionups/aplicaciones/Librerias/opencv/opencvi/lib

# Lista de flags para enlazar las bibliotecas de OpenCV
OpenCvLibraries := \
	-lopencv_core \
	-lopencv_highgui \
	-lopencv_imgproc \
	-lopencv_imgcodecs \
	-lopencv_video \
	-lopencv_videoio

# ────────────────────────────────────────────────────────
# 2) Directorios y bibliotecas de ITK 5.3
# ────────────────────────────────────────────────────────

# Carpeta donde están los headers de ITK 5.3
# (ajusta si tu grep de "dpkg -L libinsighttoolkit5-dev | grep '/usr/include/ITK'" te arrojó otro subdirectorio)
ItkIncludeDir := /usr/include/ITK-5.3

# Carpeta donde están las bibliotecas compartidas (.so) de ITK 5.3
# (normalmente /usr/lib/x86_64-linux-gnu en Ubuntu/Lubuntu)
ItkLibraryDir := /usr/lib/x86_64-linux-gnu

# Lista de módulos de ITK que vamos a enlazar.
# Incluye al menos los módulos básicos que suelen necesitarse para leer imágenes y procesarlas.
# Ajusta esta lista según los símbolos que reclamen tus fuentes y los .so que tengas en /usr/lib/x86_64-linux-gnu.
ItkLibraries := \
	-lITKCommon-5.3 \
	-lITKIOImageBase-5.3 \
	-lITKIOJPEG-5.3 \
	-lITKAlgorithms-5.3

# ────────────────────────────────────────────────────────
# 3) Flags generales de compilación
# ────────────────────────────────────────────────────────

# Definimos el compilador de C++
CPlusPlusCompiler := g++

# Flags de compilación: 
#   -std=c++17: usa C++17 (ajusta si usas otra versión de C++)
#   -I$(OpenCvIncludeDir): para que el preprocesador encuentre los headers de OpenCV
#   -I$(ItkIncludeDir): para que el preprocesador encuentre los headers de ITK
CPlusPlusFlags := -std=c++17 \
	-I$(OpenCvIncludeDir) \
	-I$(ItkIncludeDir)

# Flags del enlazador (linker):
#   -L$(OpenCvLibraryDir): carpeta con los .so de OpenCV
#   $(OpenCvLibraries): bibliotecas de OpenCV
#   -L$(ItkLibraryDir): carpeta con los .so de ITK
#   $(ItkLibraries): bibliotecas de ITK
LinkerFlags := -L$(OpenCvLibraryDir) \
	$(OpenCvLibraries) \
	-L$(ItkLibraryDir) \
	$(ItkLibraries)

# ────────────────────────────────────────────────────────
# 4) Objetivos (targets) del Makefile
# ────────────────────────────────────────────────────────

# Nombre del binario principal (que combina Principal.cpp con OpenCV + ITK)
TargetPrincipalBinary := vision.bin

# Nombre del binario de video (que, en este ejemplo, usaremos solo OpenCV)
TargetVideoBinary := visionVideo.bin

# Fuentes de cada target
PrincipalSourceFile := Principal.cpp
VideoSourceFile := PrincipalVideo.cpp

# -------------------------------------------------------
# Regla “por defecto”: compilar todo (ambos binarios)
# -------------------------------------------------------
all: $(TargetPrincipalBinary) $(TargetVideoBinary)

# -------------------------------------------------------
# Regla para compilar vision.bin (usa OpenCV + ITK)
# -------------------------------------------------------
$(TargetPrincipalBinary): $(PrincipalSourceFile)
	@echo "Compilando Principal.cpp ➔ $(TargetPrincipalBinary) (OpenCV + ITK)..."
	$(CPlusPlusCompiler) $(CPlusPlusFlags) \
		$(PrincipalSourceFile) \
		-o $(TargetPrincipalBinary) \
		$(LinkerFlags) > errors.txt 2>&1 && \
		( rm -f errors.txt && echo "Compilación del binario principal exitosa!!!" ) || \
		( cat errors.txt && echo "Error durante la compilación del binario principal" )

# -------------------------------------------------------
#  Regla para compilar visionVideo.bin (solo OpenCV)
# -------------------------------------------------------
$(TargetVideoBinary): $(VideoSourceFile)
	@echo "Compilando PrincipalVideo.cpp ➔ $(TargetVideoBinary) (solo OpenCV)..."
	$(CPlusPlusCompiler) -std=c++17 \
		-I$(OpenCvIncludeDir) \
		$(VideoSourceFile) \
		-o $(TargetVideoBinary) \
		-L$(OpenCvLibraryDir) \
		$(OpenCvLibraries) > errors.txt 2>&1 && \
		( rm -f errors.txt && echo "Compilación del binario de video exitosa!!!" ) || \
		( cat errors.txt && echo "Error durante la compilación del binario de video" )
		
# -------------------------------------------------------
# Regla “run” para ejecutar el binario principal
# -------------------------------------------------------
run: $(TargetPrincipalBinary)
	@echo "Ejecutando ./$(TargetPrincipalBinary)..."
	./$(TargetPrincipalBinary)

# -------------------------------------------------------
# Regla “videoRun” para ejecutar el binario de video
# -------------------------------------------------------
videoRun: $(TargetVideoBinary)
	@echo "Ejecutando ./$(TargetVideoBinary)..."
	./$(TargetVideoBinary)

# -------------------------------------------------------
# Regla para limpiar (borrar ambos binarios)
# -------------------------------------------------------
clean:
	@echo "Eliminando todos los binarios (vision.bin y visionVideo.bin)..."
	rm -f $(TargetPrincipalBinary) $(TargetVideoBinary)
	@echo "¡Limpieza completada!"
