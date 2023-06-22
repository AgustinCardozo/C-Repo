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
#include<time.h>
#include<unistd.h>
#include<netdb.h>

typedef enum
{
	MENSAJE,
	PAQUETE,
	INSTRUCCIONES,
	EJECUTAR,
	EJECUTAR_WAIT,
	EJECUTAR_SIGNAL,
	EJECUTAR_IO,
	DESALOJADO,
	FINALIZAR,
	INICIALIZAR_ESTRUCTURA,
	ACCEDER_PARA_LECTURA,
	ACCEDER_PARA_ESCRITURA,
	CREAR_SEGMENTO,
	ELIMINAR_SEGMENTO,
	REALIZAR_COMPACTACION,
	CREACION_EXITOSA,
	COMPACTAR,
	SIN_MEMORIA
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
}segmento;



typedef struct{
	char AX[4];
	char BX[4];
	char CX[4];
	char DX[4];
	char EAX[8];
	char EBX[8];
	char ECX[8];
	char EDX[8];
	char RAX[16];
	char RBX[16];
	char RCX[16];
	char RDX[16];
}registros_generales;

typedef enum{
	AX,BX,CX,DX,EAX,EBX,ECX,EDX,RAX,RBX,RCX,RDX
}registros_pos;

typedef struct{
	int pid;
	t_list* lista_instrucciones;
	int program_counter;
	t_list* tabla_segmentos;
	registros_generales registro;
	float estimacion;
	int real_ant;
	int llegadaReady;
	int llegadaExec;
	char** archivos_abiertos;
	int conexion_consola;
	int dat_seg;
	int dat_tamanio;
}t_pcb;

typedef struct{
	t_pcb* pcb;
	int est;
}t_est; //TODO Ver si es necesario


void saludo();
void test();

#endif /* SRC_COMPARTIDO_H_ */
