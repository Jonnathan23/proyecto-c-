# MiProyectoVisión

Este proyecto utiliza un Makefile para compilar y ejecutar la aplicación.

## Estructura del Proyecto

* `src/`: Directorio que contiene los archivos fuente de la aplicación.
    + `helpers/`: Directorio que contiene archivos de ayuda para la aplicación.
        - `Volumetrics.cpp`: Archivo que contiene la implementación de la clase `Volumetrics`.
    + `main.cpp`: Archivo que contiene la función principal de la aplicación.
* `include/`: Directorio que contiene los archivos de cabecera de la aplicación.
    + `Volumetrics.h`: Archivo que contiene la declaración de la clase `Volumetrics`.
    + `MainWindow.h`: Archivo que contiene la declaración de la clase `MainWindow`.
* `ui/`: Directorio que contiene los archivos de interfaz gráfica de usuario.
    + `MainWindow.ui`: Archivo que contiene la definición de la interfaz gráfica de usuario.
* `Makefile`: Archivo que contiene las instrucciones para compilar y ejecutar la aplicación.

## Compilación y Ejecución

Para compilar y ejecutar la aplicación, sigue los siguientes pasos:

1. Abre una terminal en el directorio raíz del proyecto.
2. Ejecuta el comando `make` para compilar la aplicación.
3. Una vez compilada, ejecuta el comando `./Proyecto_saquicela` para ejecutar la aplicación.

## Opciones de Compilación

* `make clean`: Elimina los archivo compilados anteriormente
* `make`: Compila la aplicación con todas las dependencias (Qt, OpenCV, ITK, etc.).
* `make run`: Ejecuta el programa


## Limpieza

Para eliminar los archivos objeto y ejecutables, ejecuta el comando `make clean`.

## Detalles

* El proyecto utiliza un Makefile recursivo para encontrar todos los archivos `.cpp` y `.h` en los directorios `src` e `include`.
* La aplicación utiliza la biblioteca ITK para leer y procesar imágenes NIfTI.
* La aplicación utiliza la biblioteca OpenCV para realizar operaciones de visión artificial.
* La aplicación utiliza la biblioteca Qt para crear la interfaz gráfica de usuario.