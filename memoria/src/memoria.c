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
	t_list* segments;
}tabla_de_proceso;

void mostrar_tabla_huecos_libres();
segmento* asignar_hueco_segmento_0(int tamanio);
op_asignacion devolver_metodo_asignacion(char* asignacion);
void mostrar_tablas_de_segmentos();
void atender_modulos(void* data);

void* memoria_usuario;

t_list* tabla_huecos_libres;
t_list* tabla_general;

hueco_libre* buscar_por_first(int seg_tam, int *indice);
void crear_segmento(int pid,int seg_id,int seg_tam);

seg_aux deserializar_segmento_auxiliar(t_buffer* buffer);

int pid;
int main(void) {
	pthread_t hilo_atender_modulos;
	tabla_huecos_libres = list_create();
	tabla_general = list_create();
	pid = 0;
	logger = iniciar_logger(MEMORIA_LOG, MEMORIA_NAME);
	config = iniciar_config(MEMORIA_CONFIG);

	iniciar_config_memoria();

	memoria_usuario = malloc(datos.tam_memoria);

	hueco_libre* hueco = malloc(sizeof(hueco_libre));

	hueco->base = 0;
	hueco->tamanio = datos.tam_memoria-1;

	list_add(tabla_huecos_libres,hueco);

	//seg_0 = asignar_hueco_segmento_0(datos.tam_segmento);

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
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));
	t_buffer* buffer;
	t_list* lista;
	seg_aux datos_aux;
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
				pid++;

				segmento * seg = malloc(sizeof(segmento));
				seg->id = 0;
				seg->direccion_base = 0;
				seg->tamanio = datos.tam_segmento;
				tabla_de_proceso* tab = malloc(sizeof(tabla_de_proceso));
				tab->pid = pid;
				tab->segments = list_create();
				list_add(tab->segments,seg);
				list_add(tabla_general,tab);
				send(cliente_fd,&datos.tam_segmento,sizeof(int),0);
				mostrar_tablas_de_segmentos();
				break;
			case CREAR_SEGMENTO:
				buffer = desempaquetar(paquete,cliente_fd);
				datos_aux = deserializar_segmento_auxiliar(buffer);

				log_info(logger,"Crear segmento con pid: %i id: %i con tamanio: %i ",
						datos_aux.pid,datos_aux.segmento.id,datos_aux.segmento.tamanio);

				crear_segmento(datos_aux.pid, datos_aux.segmento.id, datos_aux.segmento.tamanio);

				/*int pid, seg_id, seg_tamanio;
				int offset = 0;

				memcpy(&pid,buffer->stream + offset,sizeof(int));
				offset+=sizeof(int);
				memcpy(&seg_id,buffer->stream + offset,sizeof(int));
				offset+=sizeof(int);
				memcpy(&seg_tamanio,buffer->stream + offset,sizeof(int));
				offset+=sizeof(int);*/
				break;
			case ELIMINAR_SEGMENTO:
				buffer = desempaquetar(paquete, cliente_fd);
				datos_aux = deserializar_segmento_auxiliar(buffer);

				log_info(logger,"Eliminar segmento con pid: %i id: %i", datos_aux.pid, datos_aux.segmento.id);
				eliminar_segmento(datos_aux.pid,datos_aux.segmento.id);
				break;
			case REALIZAR_COMPACTACION:
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

void mostrar_tablas_de_segmentos(){
	log_info(logger,"ESTA ES LA TABLA DE SEGMENTOS:");
	for(int i = 0; i < list_size(tabla_general);i++){
		tabla_de_proceso* pro = list_get(tabla_general,i);
		log_info(logger,"___________________________________");
		log_info(logger,"PROCESO %i",pro->pid);
		for(int j = 0; j < list_size(pro->segments);j++){
			segmento* s = list_get(pro->segments,j);
			log_info(logger,"___________________________________");
			log_info(logger,"Segmento: %i | Base: %i | Tamanio: %i",s->id,s->direccion_base,s->tamanio);
		}
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

segmento* asignar_hueco_segmento_0(int tamanio){
	segmento * seg = malloc(sizeof(segmento));
	for(int i = 0; i < list_size(tabla_huecos_libres);i++){
		hueco_libre* hueco_asignado = list_get(tabla_huecos_libres,i);
		if(hueco_asignado->tamanio > tamanio){
			seg->id = 0;
			seg->direccion_base = 0;
			seg->tamanio = tamanio;

			hueco_asignado->base += tamanio;
			hueco_asignado->tamanio -= tamanio;
			list_replace(tabla_huecos_libres,i,hueco_asignado);
		}
	}

	return seg;
}

void crear_segmento(int pid,int seg_id,int seg_tam){
	hueco_libre* hueco;
	int indice;
	switch(datos.algoritmo){
		case FIRST:
			hueco = buscar_por_first(seg_tam,&indice);
			break;
		case BEST:
			//hueco = buscar_por_best(seg_tam);
			break;
		case WORST:
			//hueco = buscar_por_worst(seg_tam);
			break;
	}

	for(int i = 0; i < list_size(tabla_general);i++){
		tabla_de_proceso* proc = list_get(tabla_general,i);
		if(proc->pid == pid){
			segmento* seg= malloc(sizeof(segmento));
			seg->id = seg_id;
			seg->tamanio = seg_tam;
			seg->direccion_base = hueco->base;

			hueco->base += seg_tam;
			hueco->tamanio -= seg_tam;

			list_replace(tabla_huecos_libres,indice,hueco);

			list_add(proc->segments,seg);

		}
	}
}

hueco_libre* buscar_por_first(int seg_tam, int *indice){
	hueco_libre* hueco;
	for(int i = 0; i<list_size(tabla_huecos_libres);i++){
		hueco = list_get(tabla_huecos_libres,i);

		if(hueco->tamanio >= seg_tam){
			indice = &i;
			break;
		}
	}

	return hueco;
}

void eliminar_segmento(int pid, int seg_id){
	//TODO eliminar segmento
}

seg_aux deserializar_segmento_auxiliar(t_buffer* buffer){
	seg_aux segmento_auxiliar;
	int pid, seg_id, seg_tam;
	int offset = 0;

	memcpy(&pid, buffer->stream + offset, sizeof(int));
	offset+=sizeof(int);
	memcpy(&seg_id,buffer->stream + offset, sizeof(int));
	offset+=sizeof(int);
	memcpy(&seg_tam, buffer->stream + offset, sizeof(int));
	offset+=sizeof(int);

	segmento_auxiliar.pid=pid;
	segmento_auxiliar.segmento.id=seg_id;
	segmento_auxiliar.segmento.tamanio=seg_tam;

	return segmento_auxiliar;
}

