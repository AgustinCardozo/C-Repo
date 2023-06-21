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
	hueco_libre* hueco;
	int indice;
}t_hueco;

typedef struct{
	int pid;
	t_list* segments;
}tabla_de_proceso;
bool ordenar_tamanios(void* data1,void* data2);
void mostrar_tabla_huecos_libres();
void asignar_hueco_segmento_0(int tamanio);
op_asignacion devolver_metodo_asignacion(char* asignacion);
void mostrar_tablas_de_segmentos();
void atender_modulos(void* data);

void* memoria_usuario;

t_list* tabla_huecos_libres;
t_list* tabla_general;

sem_t mutex_modulos;

t_hueco buscar_por_first(int seg_tam);
t_hueco buscar_por_best(int seg_tam);
t_hueco buscar_por_worst(int seg_tam);
int crear_segmento(int pid,int seg_id,int seg_tam);

seg_aux deserializar_segmento_auxiliar(t_buffer* buffer);
seg_aux deserializar_segmento_a_eliminar(t_buffer* buffer);
void eliminar_segmento(int pid, int seg_id);

int pid;
int main(void) {
	pthread_t hilo_atender_modulos;
	sem_init(&mutex_modulos,0,1);
	tabla_huecos_libres = list_create();
	tabla_general = list_create();
	pid = 0;
	logger = iniciar_logger(MEMORIA_LOG, MEMORIA_NAME);
	config = iniciar_config(MEMORIA_CONFIG);

	iniciar_config_memoria();

	memoria_usuario = malloc(datos.tam_memoria);

	hueco_libre* hueco = malloc(sizeof(hueco_libre));

	hueco->base = 0;
	hueco->tamanio = datos.tam_memoria;

	list_add(tabla_huecos_libres,hueco);

	asignar_hueco_segmento_0(datos.tam_segmento);

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
		sem_wait(&mutex_modulos);
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
				int resultado;
				log_info(logger,"Crear segmento con pid: %i id: %i con tamanio: %i ",
						datos_aux.pid,datos_aux.segmento.id,datos_aux.segmento.tamanio);

				resultado = crear_segmento(datos_aux.pid, datos_aux.segmento.id, datos_aux.segmento.tamanio);
				send(cliente_fd, &resultado, sizeof(int), 0);

				break;
			case ELIMINAR_SEGMENTO:
				buffer = desempaquetar(paquete, cliente_fd);
				datos_aux = deserializar_segmento_auxiliar(buffer);

				log_info(logger,"Eliminar segmento con pid: %i id: %i", datos_aux.pid, datos_aux.segmento.id);
				eliminar_segmento(datos_aux.pid,datos_aux.segmento.id);
				resultado = 0;
				send(cliente_fd, &resultado, sizeof(int), 0);
				break;
			case REALIZAR_COMPACTACION:
				break;
			case ACCEDER_PARA_LECTURA:
				buffer = desempaquetar(paquete, cliente_fd);
				int df;
				memcpy(&df, buffer, sizeof(int));
				log_info(logger,"La df es %i",df);
				df = 1;
				send(cliente_fd, &df, sizeof(int), 0);
				//Ejemplo de lectura
				break;
			case ACCEDER_PARA_ESCRITURA:
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
		sem_post(&mutex_modulos);
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

	if(strcmp(asignacion,"FIRST")==0){
		cod = FIRST;
	} else if(strcmp(asignacion,"BEST")==0){
		cod = BEST;
	} else if(strcmp(asignacion,"WORST")==0){
		cod = WORST;
	} else {
		log_warning(logger,"Codigo invalido");
	}

	return cod;
}

void asignar_hueco_segmento_0(int tamanio){
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
}

int crear_segmento(int pid,int seg_id,int seg_tam){

	t_hueco hueco_i;

	switch(datos.algoritmo){
		case FIRST:
			hueco_i = buscar_por_first(seg_tam);
			break;
		case BEST:
			hueco_i = buscar_por_best(seg_tam);
			break;
		case WORST:
			hueco_i = buscar_por_worst(seg_tam);
			break;
	}

	for(int i = 0; i < list_size(tabla_general);i++){
		tabla_de_proceso* proc = list_get(tabla_general,i);
		if(proc->pid == pid){
			segmento* seg= malloc(sizeof(segmento));
			seg->id = seg_id;
			seg->tamanio = seg_tam;
			seg->direccion_base = hueco_i.hueco->base;

			hueco_i.hueco->base += seg_tam;
			hueco_i.hueco->tamanio -= seg_tam;

			list_replace(tabla_huecos_libres,hueco_i.indice,hueco_i.hueco);

			list_add(proc->segments,seg);

		}
	}
	list_sort(tabla_huecos_libres,ordenar_tamanios);
	mostrar_tabla_huecos_libres();
	mostrar_tablas_de_segmentos();


	return 0;
}

bool ordenar_tamanios(void* data1,void* data2){
	hueco_libre* hueco1 = ((hueco_libre*) data1);
	hueco_libre* hueco2 = ((hueco_libre*) data2);

	if(hueco1->base < hueco2->base){
		return true;

	} else {
		return false;
	}
}

t_hueco buscar_por_first(int seg_tam){
	t_hueco hueco_i;
	for(int i = 0; i<list_size(tabla_huecos_libres);i++){
		hueco_i.hueco = list_get(tabla_huecos_libres,i);

		if(hueco_i.hueco->tamanio >= seg_tam){
			hueco_i.indice = i;
			break;
		}
	}

	return hueco_i;
}

t_hueco buscar_por_best(int seg_tam){


	t_hueco hueco_i;
	for(int i = 0; i<list_size(tabla_huecos_libres);i++){
		hueco_i.hueco = list_get(tabla_huecos_libres,i);

		if(hueco_i.hueco->tamanio == seg_tam){
			hueco_i.indice = i;
			break;
		} else if(hueco_i.hueco->tamanio > seg_tam){
			hueco_i.indice = i;
			for(int j = i+1; j<list_size(tabla_huecos_libres);j++){
				hueco_libre* hueco_j = list_get(tabla_huecos_libres,j);
				if(hueco_j->tamanio < hueco_i.hueco->tamanio && hueco_j->tamanio >= seg_tam){
					hueco_i.hueco = hueco_j;
					hueco_i.indice = j;
					break;
				}
			}
			break;
		}
	}

	return hueco_i;
}

t_hueco buscar_por_worst(int seg_tam){


	t_hueco hueco_i;
	for(int i = 0; i<list_size(tabla_huecos_libres);i++){
		hueco_i.hueco = list_get(tabla_huecos_libres,i);


		if(hueco_i.hueco->tamanio >= seg_tam){
			hueco_i.indice = i;
			for(int j = i+1; j<list_size(tabla_huecos_libres);j++){
				hueco_libre* hueco_j = list_get(tabla_huecos_libres,j);
				if(hueco_j->tamanio > hueco_i.hueco->tamanio){
					hueco_i.hueco = hueco_j;
					hueco_i.indice = j;
					break;
				}
			}
			break;
		}
	}

	return hueco_i;
}

void eliminar_segmento(int pid, int seg_id){
	for(int i = 0; i < list_size(tabla_general);i++){
		tabla_de_proceso* proc = list_get(tabla_general,i);
		if(proc->pid == pid){

			for(int j = 0; j < list_size(proc->segments); j++){
				segmento* seg = list_get(proc->segments,j);
				if(seg->id == seg_id){
					hueco_libre* hueco = malloc(sizeof(hueco_libre));
					hueco->base = seg->direccion_base;
					hueco->tamanio = seg->tamanio;
					list_add(tabla_huecos_libres,hueco);
					list_remove(proc->segments,j);

				}
			}

		}
	}
	list_sort(tabla_huecos_libres,ordenar_tamanios);
	mostrar_tabla_huecos_libres();
	mostrar_tablas_de_segmentos();
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

seg_aux deserializar_segmento_a_eliminar(t_buffer* buffer){
	seg_aux segmento_auxiliar;
	int pid, seg_id;
	int offset = 0;

	memcpy(&pid, buffer->stream + offset, sizeof(int));
	offset+=sizeof(int);
	memcpy(&seg_id,buffer->stream + offset, sizeof(int));
	offset+=sizeof(int);

	segmento_auxiliar.pid=pid;
	segmento_auxiliar.segmento.id=seg_id;
	segmento_auxiliar.segmento.tamanio=0;

	return segmento_auxiliar;
}

char* leer_valor_de_memoria(int df){
	char* valor;

	memcpy(valor,memoria_usuario+df,4);

	return valor;
}

void escribir_valor_de_memoria(char* valor,int df){

	memcpy(memoria_usuario+df,valor,4);
}

