#include <iostream>
#include <string.h>

#include "FileSystem.h"

void separador(const std::string& texto) {
  std::cout << "\n========== " << texto << " ==========\n";
}

int main() {
  FileSystem* fs= new FileSystem() ;

  // --- CREAR ARCHIVOS ---
  separador("Prueba 1: Crear archivos");
  fs->crearInodo("a.dat") ;
  fs->agregar("a.dat", 'a') ;
  fs->crearInodo("b.dat") ;
  fs->agregar("b.dat",'b') ;
  fs->crearInodo("c.dat") ;
  fs->agregar("c.dat",'c') ;
  fs->imprimir() ;

  // --- ESCRIBIR Y LEER ---
  separador("Prueba 2: Escribir y leer");
  fs->escribir("a.dat", "hola!");
  char* datos = fs->leer("a.dat", 5);
  std::cout << "Datos leidos de a.dat: [" << datos << "]" << std::endl;
  delete[] datos;
  fs->imprimir();

  // --- RENOMBRAR ---
  separador("Prueba 3: Renombrar");
  bool renombrado = fs->renombrar("a.dat", "ballena.dat");
  std::cout << "Renombrado: " << renombrado << std::endl;
  fs->imprimir();

  // --- LEER RENOMBRADO ---
  separador("Prueba 4: Leer archivo renombrado");
  datos = fs->leer("ballena.dat", 11);
  if (datos) {
    std::cout << "Datos leidos de ballena.dat: [" << datos << "]" << std::endl;
    delete[] datos;
  } else {
    std::cout << "ERR: no se pudo leer de ballena.dat" << std::endl;
  }

  // --- REEMPLAZAR ---
  separador("Prueba 5: Reemplazar ballena.dat");
  bool reemplazado = fs->reemplazar("ballena.dat", "ballenaaaaaaaaaa");
  std::cout << "Reemplazo: " << reemplazado << std::endl;
  datos = fs->leer("ballena.dat", strlen("ballenaaaaaaaaaa"));
  if (datos) {
    std::cout << "Nuevo contenido de ballena.dat: [" << datos << "]" << std::endl;
    delete[] datos;
  }
  fs->imprimir();

  // --- ELIMINAR ---
  separador("Prueba 6: Eliminar b.dat");
  bool eliminado = fs->eliminar("b.dat");
  std::cout << "Eliminacion: " << eliminado << std::endl;
  fs->imprimir();

  // --- LEER ELIMINADO ---
  separador("Prueba 7: Leer archivo eliminado");
  datos = fs->leer("b.dat", 10);
  if (datos) {
    std::cout << "ERR: b.dat todavia existe (se pudo leer): [" << datos << "]" << std::endl;
    delete[] datos;
  } else {
    std::cout << "No se pudo leer de 'b.dat' (no existe)" << std::endl;
  }

  // --- CREAR DESPUES DE ELIMINAR ---
  separador("Prueba 8: Crear archivo despues de haber eliminado otro");
  fs->crearInodo("d.dat");
  fs->escribir("d.dat", "d.dat se acaba de crear");
  fs->imprimir();

  // --- ELIMINAR TODO ---
  separador("Prueba 9: Eliminar 'ballena.dat' y 'c.dat'");
  fs->eliminar("ballena.dat");
  fs->eliminar("c.dat");
  fs->eliminar("d.dat");
  fs->imprimir();

  // --- ELIMINAR ARCHIVO YA ELIMINADO ---
  separador("Prueba 10: Eliminar 'ballena.dat' despues de haberla elminado ya");
  fs->eliminar("ballena.dat");
  fs->imprimir();

  // --- AGREGAR ---
  separador("Prueba 11: Agregar a un archivo");
  fs->crearInodo("agregar.dat");
  fs->escribir("agregar.dat", "TEXTO INICIAL");
  for (int i = 0; i < 100; i++) {
    fs->agregar("agregar.dat", 'X');
  }
  int largo = strlen("TEXTO INICIAL") + 100;
  datos = fs->leer("agregar.dat", largo);
  if (datos) {
    std::cout << "Contenido agregado:" << std::endl;
    for (int i = 0; i < largo && datos[i] != '\0'; i++) {
      std::cout << datos[i];
    }
    std::cout << "]" << std::endl;
    delete[] datos;
  }
  fs->imprimir();

  separador("FIN");
  delete fs;
  return 0;
}
