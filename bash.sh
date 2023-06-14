#!/bin/bash

git_USUARIO = "USUARIO"
git_PASSWORD = "TOKEN"

testfunction(){

   echo "Ingresa USUARIO:"
   read git_USUARIO

   echo "Ingresa PASSWORD/TOKEN:"
   read git_PASSWORD

   cd $1
   git install https://github.com/sisoputnfrba/so-commons-library.git $1
   git_USUARIO $1
   git_PASSWORD $1
   echo "Se instalaron las commons"

   cd so-commons-library/ $1
   make all $1
   echo "Se generaron las estructuras de las COMMONS"


   cd $1
   cd tp-2023-1c-EstaEsLaVencida/compartido/Debug/ $1
   make all $1
   cd .. $1
   cd src/ $1
   make install $1
   utnso $1
   echo "Se generaron las estrucuras de COMPARTIDO"

   cd $1
   cd tp-2023-1c-EstaEsLaVencida/memoria/Debug/ $1
   make all $1

   cd $1
   cd tp-2023-1c-EstaEsLaVencida/cpu/Debug/ $1
   make all $1

   cd $1
   cd tp-2023-1c-EstaEsLaVencida/fileSystem/Debug/ $1
   make all $1

   cd $1
   cd tp-2023-1c-EstaEsLaVencida/kernel/Debug/ $1
   make all $1

   cd $1
   cd tp-2023-1c-EstaEsLaVencida/consola/Debug/ $1
   make all $1

   echo "todo listo para ejecucion"
}
testfunction
