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
t_config* config;
int main(void) {

	char* ip;
	char* puerto;
	logger = iniciar_logger("consola.log","Consola");
	config = iniciar_config("consola.config");
	int conexion;
	ip = config_get_string_value(config,"IP_KERNEL");
	puerto = config_get_string_value(config,"PUERTO_KERNEL");

	log_info(logger,"Esto es la consola y los IP del kernel son %s y el Puerto es %s \n",ip,puerto);

	conexion = crear_conexion(ip,puerto);

	enviar_mensaje("Hola estoy probando cosas para despues",conexion);
	paquete(conexion);
	log_destroy(logger);
	config_destroy(config);
	close(conexion);

	return EXIT_SUCCESS;
}
