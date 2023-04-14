/*
 * compartido.h
 *
 *  Created on: Apr 6, 2023
 *      Author: utnso
 */

#ifndef SRC_COMPARTIDO_H_
#define SRC_COMPARTIDO_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<unistd.h>
#include<netdb.h>

typedef enum
{
	MENSAJE,
	PAQUETE,
	INSTRUCCIONES
}op_code;

typedef struct{
	int cod;
	t_list* lista;
}bloque;

typedef struct
{
	int size;
	void* stream;
}t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
}t_paquete;

typedef enum
{
	SET,
	MOV_IN,
	MOV_OUT,
	IO,
	F_OPEN,
	F_CLOSE,
	F_SEEK,
	F_READ,
	F_WRITE,
	F_TRUNCATE,
	WAIT,
	SIGNAL,
	CREATE_SEGMENT,
	DELETE_SEGMENT,
	YIELD,
	EXIT
}op_instruct;

typedef struct{
	op_instruct nombre;
	t_list* parametros;
}t_instruccion;

typedef struct{
	int id;
	int direccion_base;
	int tamanio;
}tabla_segmento; //TODO Preguntar como puede ser

typedef struct{
	int pid;
	t_list* lista_instrucciones;
	int program_counter;
	tabla_segmento segmentos;
	int estimacion;
	//TODO Preguntar como se usa el timestap
	char** archivos_abiertos;
}t_pcb;



void saludo();
void test();

#endif /* SRC_COMPARTIDO_H_ */
