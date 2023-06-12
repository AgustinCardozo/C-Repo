#!/bin/bash
testfunction(){
   echo "My first function"
   cd $1
   cd tp-2023-1c-EstaEsLaVencida/compartido/Debug/ $1
   make all $1

   cd $1
   cd tp-2023-1c-EstaEsLaVencida/memoria/Debug/ $1
   make all $1

   cd $1
   cd tp-2023-1c-EstaEsLaVencida/cpu/Debug/ $1
   make all $1

   cd $1
   cd tp-2023-1c-EstaEsLaVencida/kernel/Debug/ $1
   make all $1

   cd $1
   cd tp-2023-1c-EstaEsLaVencida/fileSystem/Debug/ $1
   make all $1

   cd $1
   cd tp-2023-1c-EstaEsLaVencida/consola/Debug/ $1
   make all $1
}
testfunction