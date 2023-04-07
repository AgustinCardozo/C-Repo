/*
 ============================================================================
 Name        : memoria.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include "memoria.h"

t_log* logger;
t_config* config;
datos_config datos;
int server_fd;
int* cliente_fd;
int main(void) {
	pthread_t hilo_atender_modulos;

	logger = iniciar_logger("memoria.log","Memoria");;
	config = iniciar_config("memoria.config");

	datos.puerto_escucha = config_get_string_value(config,"PUERTO_ESCUCHA");

	int server_fd = iniciar_servidor(logger,"127.0.0.1",datos.puerto_escucha);
	log_info(logger, "Memoria listo para recibir a los modulos");

	cliente_fd = malloc(sizeof(int));
//
	while(1){
		cliente_fd = esperar_cliente(logger,server_fd);
		pthread_create(&hilo_atender_modulos,NULL,(void*) atender_modulos,cliente_fd);
		pthread_detach(hilo_atender_modulos);
	}

	log_destroy(logger);
	config_destroy(config);
	close(server_fd);
}

void atender_modulos(void* data){
	int cliente_fd = *((int*)data);
	t_list* lista;
	while(1){

		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
			case MENSAJE:
				recibir_mensaje(logger,cliente_fd);
				break;
			case PAQUETE:
				lista = recibir_paquete(cliente_fd);
				log_info(logger, "Me llegaron los siguientes valores:\n");
				list_iterate(lista, (void*) iterator);
				break;
			case -1:
				log_error(logger, "el cliente se desconecto. Terminando servidor");
				log_destroy(logger);
				config_destroy(config);
				close(cliente_fd);
				close(server_fd);
				free(data);
				//return EXIT_FAILURE;
				exit(1);
			default:
				log_warning(logger,"Operacion desconocida. No quieras meter la pata");
				break;
		}
	}

}


void iterator(char* value) {
	log_info(logger,"%s", value);
}
