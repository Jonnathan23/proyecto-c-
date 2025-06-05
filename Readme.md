# MiProyectoVisión

Este proyecto utiliza un Makefile para compilar y ejecutar la aplicación.

## Requisitos

* OpenCV
* ITK
* Qt
* NIfTI
* VNL
* itksys

## Compilación y Ejecución

Para compilar y ejecutar la aplicación, sigue los siguientes pasos:

1. Abre una terminal en el directorio raíz del proyecto.
2. Ejecuta el comando `make` para compilar la aplicación.
3. Una vez compilada, ejecuta el comando `./Proyecto_saquicela` para ejecutar la aplicación.

## Opciones de Compilación

* `make all`: Compila la aplicación con todas las dependencias (Qt, OpenCV, ITK, etc.).
* `make main`: Compila la aplicación solo con las dependencias de OpenCV e ITK, sin Qt.

## Limpieza

Para eliminar los archivos objeto y ejecutables, ejecuta el comando `make clean`.

## Detalles

* El proyecto utiliza un Makefile recursivo para encontrar todos los archivos `.cpp` y `.h` en los directorios `src` e `include`.
* La aplicación utiliza la biblioteca ITK para leer y procesar imágenes NIfTI.
* La aplicación utiliza la biblioteca OpenCV para realizar operaciones de visión artificial.
* La aplicación utiliza la biblioteca Qt para crear la interfaz gráfica de usuario.