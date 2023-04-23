/*
 * serializacion.c
 *
 *  Created on: Apr 18, 2023
 *      Author: utnso
 */

#include "serializacion.h"

t_paquete* crear_paquete_pcb(t_pcb* pcb, op_code codigo){
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo;
	paquete->buffer = serializar_pcb(pcb);

	return paquete;
}


t_buffer* serializar_pcb(t_pcb* pcb){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	t_buffer* instrucciones = serializar_lista_de_instrucciones(pcb->lista_instrucciones);
	int offset = 0;
	buffer->size = sizeof(int)*9 + instrucciones->size;

	buffer->stream = malloc(buffer->size);

	memcpy(buffer->stream + offset, &pcb->pid, sizeof(int));
	offset += sizeof(int);

	memcpy(buffer->stream + offset, &pcb->program_counter, sizeof(int));
	offset += sizeof(int);

	memcpy(buffer->stream + offset, &pcb->segmentos.id, sizeof(int));
	offset += sizeof(int);

	memcpy(buffer->stream + offset, &pcb->segmentos.direccion_base, sizeof(int));
	offset += sizeof(int);

	memcpy(buffer->stream + offset, &pcb->segmentos.tamanio, sizeof(int));
	offset += sizeof(int);

	memcpy(buffer->stream + offset, &pcb->estimacion, sizeof(int));
	offset += sizeof(int);

	memcpy(buffer->stream + offset, &pcb->conexion_consola, sizeof(int));
	offset += sizeof(int);

	memcpy(buffer->stream + offset, &(instrucciones->size), sizeof(int));
	offset += sizeof(int);

	if (instrucciones->size > 0) {
		memcpy(buffer->stream + offset, instrucciones->stream, instrucciones->size);
		offset += instrucciones->size;
	}

	return buffer;

}



t_buffer* serializar_lista_de_instrucciones(t_list* lista_instrucciones){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	int i_count;
	int	p_count;
	int stream_size;
	int offset = 0;
	t_list_iterator* i_iterator;
	t_list_iterator* p_iterator;
	t_instruccion*	instruccion;
	char* parametro;
	uint32_t size_parametro;

	i_iterator = list_iterator_create(lista_instrucciones);
	stream_size = 0;
	i_count = 0;

	while (list_iterator_has_next(i_iterator)) {
			instruccion = (t_instruccion*)list_iterator_next(i_iterator);

			// codigo, cantidad de parametros
			stream_size += (sizeof(uint32_t) * 2);
			p_count = list_size(instruccion->parametros);

			if (p_count > 0) {
				p_iterator = list_iterator_create(instruccion->parametros);

				while (list_iterator_has_next(p_iterator)) {
					// longitud del parametro, valor del parametro
					stream_size += sizeof(uint32_t) + strlen((char*)list_iterator_next(p_iterator)) + 1;
				}

				list_iterator_destroy(p_iterator);
			}

			i_count++;
		}

	list_iterator_destroy(i_iterator);

	buffer->size = stream_size + sizeof(uint32_t);
	buffer->stream = malloc(buffer->size);

	memcpy(buffer->stream + offset, &(i_count), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	i_iterator = list_iterator_create(lista_instrucciones);

	while (list_iterator_has_next(i_iterator)) {
			instruccion = (t_instruccion*)list_iterator_next(i_iterator);

			// codigo de instruccion
			memcpy(buffer->stream + offset, &(instruccion->nombre), sizeof(uint32_t));
			offset += sizeof(uint32_t);

			// cantidad de parametros
			p_count = list_size(instruccion->parametros);
			memcpy(buffer->stream + offset, &(p_count), sizeof(uint32_t));
			offset += sizeof(uint32_t);

			if (p_count > 0) {
				p_iterator = list_iterator_create(instruccion->parametros);

				while (list_iterator_has_next(p_iterator)) {
					parametro = (char*)list_iterator_next(p_iterator);
					size_parametro = strlen(parametro) + 1;

					// longitud del parametro
					memcpy(buffer->stream + offset, &(size_parametro), sizeof(uint32_t));
					offset += sizeof(uint32_t);

					// valor del parametro
					memcpy(buffer->stream + offset, parametro, size_parametro);
					offset += size_parametro;
				}

				list_iterator_destroy(p_iterator);
			}
		}

	list_iterator_destroy(i_iterator);
	if (offset != buffer->size){
			printf("serializar_lista_instrucciones: El tamaño del buffer[%d] no coincide con el offset final[%d]\n", buffer->size, offset);
	}
	return buffer;

}

t_list* deserializar_lista_instrucciones(t_buffer* buffer){

	t_list*	i_list;
	t_instruccion* instruccion;
	uint32_t i_count;
	uint32_t p_count;
	int offset;
	int	p_size;
	char* param;
	op_instruct	codigo;

	i_list = list_create();
	offset = 0;

	memcpy(&i_count, buffer->stream + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	//log_info(logger,"Esta deserializando %i",i_count);

	for(int i=0; i < i_count; i++) {

			// codigo de la instruccion
			memcpy(&(codigo), buffer->stream + offset, sizeof(uint32_t));
			offset += sizeof(uint32_t);
			instruccion = malloc(sizeof(t_instruccion));
			instruccion->nombre = codigo;
			instruccion->parametros = list_create();

			// cantidad de parametros
			memcpy(&(p_count), buffer->stream + offset, sizeof(uint32_t));
			offset += sizeof(uint32_t);

			if (p_count > 0) {
				for (int p = 0; p < p_count; p++) {
					// longitud del parametro
					memcpy(&p_size, buffer->stream + offset, sizeof(uint32_t));
					offset += sizeof(uint32_t);

					// valor del parametro
					param = malloc(p_size);
					memcpy(param, buffer->stream + offset, p_size);

					list_add(instruccion->parametros, param);
					offset += p_size;
				}
			}

			list_add(i_list, instruccion);

		}

		return i_list;
}

void enviar_paquete_a(t_paquete* paquete,int conexion){
	void* a_enviar = malloc(paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t));
	int offset = 0;

	memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));

	offset += sizeof(op_code);
	memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
	offset += paquete->buffer->size;
	send(conexion, a_enviar, paquete->buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}
// pcb, conexion del socket, codigo para saber que va a realizar
void enviar_pcb_a(t_pcb* pcb,int conexion, op_code codigo){

	t_paquete* paquete = crear_paquete_pcb(pcb,codigo);

	enviar_paquete_a(paquete,conexion);
}

t_buffer* desempaquetar(t_paquete* paquete, int cliente_fd){
	recv(cliente_fd, &(paquete->buffer->size), sizeof(uint32_t), 0);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(cliente_fd, paquete->buffer->stream, paquete->buffer->size, 0);

	return paquete->buffer;
}


t_pcb* deserializar_pcb(t_buffer* buffer){
	t_pcb* pcb = malloc(sizeof(t_pcb));

	int offset = 0;

	memcpy(&(pcb->pid),buffer->stream + offset,sizeof(int));
	offset += sizeof(int);
	memcpy(&(pcb->program_counter),buffer->stream + offset,sizeof(int));
	offset += sizeof(int);
	memcpy(&(pcb->segmentos.id),buffer->stream + offset,sizeof(int));
	offset += sizeof(int);
	memcpy(&(pcb->segmentos.direccion_base),buffer->stream + offset,sizeof(int));
	offset += sizeof(int);
	memcpy(&(pcb->segmentos.tamanio),buffer->stream + offset,sizeof(int));
	offset += sizeof(int);
	memcpy(&(pcb->estimacion),buffer->stream + offset,sizeof(int));
	offset += sizeof(int);
	memcpy(&(pcb->conexion_consola),buffer->stream + offset,sizeof(int));
	offset += sizeof(int);

	t_buffer* buffer_i = malloc(sizeof(t_buffer));

	memcpy(&(buffer_i->size), buffer->stream + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer_i->stream = malloc(buffer_i->size);
	memcpy(buffer_i->stream, buffer->stream + offset, buffer_i->size);
	offset += buffer_i->size;

	t_list* lista_instrucciones = deserializar_lista_instrucciones(buffer_i);

	pcb->lista_instrucciones=lista_instrucciones;

	return pcb;
}


