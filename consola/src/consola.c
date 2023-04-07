/*
 ============================================================================
 Name        : consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================

 */

#include <stdio.h>
#include <stdlib.h>
#include "consola.h"

t_log* logger;

int main(void) {
	puts("Esto es la consola, proband"); /* prints !!!Hello World!!! */
	saludo();
	//char* ip;
	//char* puerto;
	logger = iniciar_logger("consola.log","Consola");

	log_info(logger,"Esto es la consola");

	log_destroy(logger);

	return EXIT_SUCCESS;
}
