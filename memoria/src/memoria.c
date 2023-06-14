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

typedef struct{
	int base;
	int tamanio;
}hueco_libre;

typedef struct{
	int pid;
	segmento segment;
}tabla_de_proceso;
void mostrar_tabla_huecos_libres();
op_asignacion devolver_metodo_asignacion(char* asignacion);
void* memoria_usuario;

t_list* tabla_huecos_libres;
t_list* tabla_general;

int main(void) {
	pthread_t hilo_atender_modulos;
	tabla_huecos_libres = list_create();
	tabla_general = list_create();

	logger = iniciar_logger(MEMORIA_LOG, MEMORIA_NAME);
	config = iniciar_config(MEMORIA_CONFIG);

	iniciar_config_memoria();

	memoria_usuario = malloc(datos.tam_memoria);

	hueco_libre* hueco = malloc(sizeof(hueco_libre));

	hueco->base = 0;
	hueco->tamanio = datos.tam_memoria-1;

	list_add(tabla_huecos_libres,hueco);
	mostrar_tabla_huecos_libres();
	int server_fd = iniciar_servidor(logger,datos.puerto_escucha);
	log_info(logger, "Memoria listo para recibir a los modulos");

	cliente_fd = malloc(sizeof(int));

	while(1){
		cliente_fd = esperar_cliente(logger,server_fd);
		pthread_create(&hilo_atender_modulos,NULL,(void*) atender_modulos,cliente_fd);
		pthread_detach(hilo_atender_modulos);
	}
	
	finalizar_memoria();
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
			case INICIALIZAR_ESTRUCTURA:
				log_info(logger,"Paso por INICIALIZAR_ESTRUCTURA");
				send(cliente_fd,&datos.tam_segmento,sizeof(int),0);
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

void iniciar_config_memoria(){
	datos.puerto_escucha = config_get_string_value(config,"PUERTO_ESCUCHA");
	datos.tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
	datos.tam_segmento = config_get_int_value(config, "TAM_SEGMENTO_0");
	datos.cant_segmentos = config_get_int_value(config, "CANT_SEGMENTOS");
	datos.ret_memoria = config_get_int_value(config, "RETARDO_MEMORIA");
	datos.ret_compactacion = config_get_int_value(config, "RETARDO_COMPACTACION");
	datos.algoritmo = devolver_metodo_asignacion(config_get_string_value(config, "ALGORITMO_ASIGNACION"));
}

void finalizar_memoria(){
	log_destroy(logger);
	config_destroy(config);
	close(server_fd);
	free(cliente_fd);
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}

void mostrar_tabla_huecos_libres(){
	log_info(logger,"ESTA ES LA TABLA DE HUECOS LIBRES:");
	log_info(logger,"___________________________________");
	log_info(logger,"BASE        |    TAMANIO     ");
	for(int i = 0; i < list_size(tabla_huecos_libres);i++){
		hueco_libre* hueco = list_get(tabla_huecos_libres,i);
		log_info(logger,"%i          |    %i          ",hueco->base,hueco->tamanio);
	}
	log_info(logger,"___________________________________");
}

op_asignacion devolver_metodo_asignacion(char* asignacion){
	op_asignacion cod;

	if(strcmp(asignacion,"FIRST")){
		cod = FIRST;
	} else if(strcmp(asignacion,"BEST")){
		cod = BEST;
	} else if(strcmp(asignacion,"WORST")){
		cod = WORST;
	} else {
		log_warning(logger,"Codigo invalido");
	}

	return cod;
}

