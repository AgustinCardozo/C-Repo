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
int main(void) {
	logger = iniciar_logger("memoria.log","Memoria");;
	config = iniciar_config("memoria.config");

	datos.puerto_escucha = config_get_string_value(config,"PUERTO_ESCUCHA");

	int server_fd = iniciar_servidor(logger,"127.0.0.1",datos.puerto_escucha);
	log_info(logger, "Memoria listo para recibir a las consolas");

	//int cliente_fd = accept(server_fd, NULL, NULL);;
	int *cliente_fd = esperar_cliente(logger,server_fd);
	t_list* lista;
	while(1){

		int cod_op = recibir_operacion(*cliente_fd);
				switch (cod_op) {
				case MENSAJE:
					recibir_mensaje(logger,*cliente_fd);
					break;
				case PAQUETE:
					lista = recibir_paquete(*cliente_fd);
					log_info(logger, "Me llegaron los siguientes valores:\n");
					list_iterate(lista, (void*) iterator);
					break;
				case -1:
					log_error(logger, "el cliente se desconecto. Terminando servidor");
					log_destroy(logger);
					config_destroy(config);
					close(server_fd);
					free(cliente_fd);
					return EXIT_FAILURE;
				default:
					log_warning(logger,"Operacion desconocida. No quieras meter la pata");
					break;
				}
	}
	//free(cliente_fd);
	log_destroy(logger);
	config_destroy(config);
	close(server_fd);
	free(cliente_fd);
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}
