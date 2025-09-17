# Sistema de Archivos

## Estudiante:
- Alexa Alpizar Mora, C20281

## Tabla de contenidos
- Instalación
- Manual de usuario y descripción de tareas
- Diseño

## Instalación

Para compilar este código, asegúrate de tener instalado:

- Visual Studio Code (con extensiones de C/C++) para ejecutar el código
- Make o MakeTools para compilar con el Makefile

## Manual de usuario

### Descripción de la tarea

El objetivo principal de este proyecto es implementar un sistema de archivos básico que permita gestionar archivos en un disco virtual. El sistema ofrece funcionalidades para crear un disco, guardar y leer archivos, gestionar espacio mediante un bitmap y mantener un directorio de archivos.

### Cómo usarlo

Para ejecutar este programa, sigue estos pasos:

1. Clona el repositorio en tu terminal.

git clone git@github.com:aalpizarmora/Redes_PI_Oper_2025b-AlexaAlpizar.git

2. Navega al directorio del repositorio.

cd Redes_PI_Oper_2025b-AlexaAlpizar

3. Compila el programa usando el Makefile.

make clean
make

4. Ejecuta el programa.

# Para crear un nuevo disco
./programa 1

# Para usar un disco existente
./programa 0

### Funcionalidades

Una vez ejecutado el programa, aparecerá un menú interactivo con las siguientes opciones:

1. Guardar archivo: Permite copiar un archivo desde el sistema host al disco virtual
2. Leer archivo: Muestra el contenido de un archivo almacenado en el disco virtual
3. Imprimir bitmap: Visualiza el estado del bitmap (espacios usados y libres)
4. Leer directorio: Lista todos los archivos presentes en el directorio virtual
0. Salir: Finaliza la aplicación

## Créditos
[Alexa Alpizar Mora]
