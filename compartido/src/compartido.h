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

typedef enum {
    PCB_NEW,
    PCB_READY,
    PCB_BLOCKED,
    PCB_EXEC,
    PCB_EXIT,
} t_process_status;

typedef struct{
	uint32_t nro_segmento;
	uint32_t tamanio_Segmento;
	uint32_t id_tabla_pagina;
}t_segmento;

typedef enum
{
	SET,
	ADD,
	MOV_IN,
	MOV_OUT,
	IO,
	EXIT
}op_instruct;

typedef enum{
	AX,
	BX,
	CX,
	DX,
	DISCO,
	IMPRESORA,
	PANTALLA
}param_instruct;

typedef struct {
    int                    id;
    t_process_status       status;
    uint32_t               registros_cpu[3];
    int                    program_counter;
    t_list*                tabla_segmentos;
    t_list*                instructions;

}t_pcb;

typedef struct{
	op_instruct nombre;
	t_list* parametros;
}t_instruccion;

void saludo();

#endif /* SRC_COMPARTIDO_H_ */
