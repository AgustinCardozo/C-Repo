/*
 ============================================================================
 Name        : fileSystem.c
 Author      : EstaEsLaVencida
 Version     :
 Copyright   : Your copyright notice
 Description : Modulo de FileSystem
 ============================================================================
 */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "fileSystem.h"

int main(void) {
	logger = iniciar_logger(FS_LOG, FS_NAME);;
	config = iniciar_config(FS_CONFIG);
	lista_fcb = list_create();

	inicializar_config();
	crear_estructura_fs(contenido_superbloque);

	log_info(logger,"PATH_SUPERBLOQUE: %s", datos.path_superbloque);
	log_info(logger,"PATH_BITMAP: %s", datos.path_bitmap);
	log_info(logger,"PATH_BLOQUES: %s", datos.path_bloques);
	log_info(logger,"PATH_FCB: %s\n", datos.path_fcb);



	//prueba();

	diretorio_FCB = opendir(datos.path_fcb);

	void* dato1="Console";
	log_info(logger, "El dato1 es %s", dato1);
	escribir_dato_en_bloque(dato1,0,sizeof(char)*8);

	void* dato = leer_dato_del_bloque(32,80);

	log_info(logger, "El dato es %p", dato);

	pthread_create(&hilo_conexion_kernel,NULL,(void*) atender_kernel,NULL);
	pthread_create(&hilo_conexion_memoria,NULL,(void*) atender_memoria,NULL);

	pthread_join(hilo_conexion_kernel,NULL);
	pthread_join(hilo_conexion_memoria,NULL);

	liberar_lista_bloques();
	cerrar_bitmap();
	finalizar_fs();
	return EXIT_SUCCESS;
}

// ---------------------------------- INICIALIZAR ----------------------------------- //

void inicializar_config(){
	datos.ip_memoria = config_get_string_value(config,"IP_MEMORIA");
	datos.puerto_memoria = config_get_string_value(config,"PUERTO_MEMORIA");
	datos.puerto_escucha = config_get_string_value(config,"PUERTO_ESCUCHA");
	datos.path_superbloque = config_get_string_value(config,"PATH_SUPERBLOQUE");
	datos.path_bitmap = config_get_string_value(config,"PATH_BITMAP");
	datos.path_bloques = config_get_string_value(config,"PATH_BLOQUES");
	datos.path_fcb = config_get_string_value(config,"PATH_FCB");
	datos.ret_acceso_bloque = config_get_int_value(config,"RETARDO_ACCESO_BLOQUE");
}

void crear_estructura_fs(const char *contenidos[]){
	int valor = mkdir(PATH_FS, 0777);
	if(valor < 0){
		log_warning(logger, "Ya existe el directorio: (%s)", PATH_FS);
	}
	int num_contenidos = sizeof(contenido_superbloque) / sizeof(contenido_superbloque[0]);
	crear_archivo(datos.path_superbloque, contenido_superbloque, num_contenidos);
	config_superbloque = iniciar_config_test(datos.path_superbloque);
	// config_fcb = iniciar_config_test(datos.path_fcb);
	
	valor = mkdir(datos.path_fcb, 0777);
	if(valor < 0){
		log_warning(logger, "Ya existe el directorio: %s", datos.path_fcb);
	}
	inicializar_superbloque();
	crear_archivo_bloques();
	crear_archivo_bitmap();
}

void crear_archivo(const char *nombre_archivo, const char *contenidos[], int num_contenidos) {
    FILE *archivo;

    archivo = fopen(nombre_archivo, "r");
    log_info(logger, "Paso por crear archivo: %s", nombre_archivo);

    if (archivo == NULL) {
    	//log_info(logger, "Entro por NULL: %s", nombre_archivo);
        archivo = fopen(nombre_archivo, "w");
        if (archivo != NULL) {
            for (int i = 0; i < num_contenidos; i++) {
                fprintf(archivo, "%s\n", contenidos[i]);
            }
            fclose(archivo);
            log_info(logger, "Archivo creado exitosamente: %s\n", nombre_archivo);
        } else {
            log_error(logger,"No se pudo crear el archivo: %s", nombre_archivo);
        }
    } else {
        log_warning(logger, "El archivo ya existe: %s", nombre_archivo);
        fclose(archivo);
    }
}

void crear_archivo_fcb(char* nombreArchivo){

	char *path_archivo_completo = concatenar_path(nombreArchivo);

	int num_contenidos = sizeof(contenido_fcb) / sizeof(contenido_fcb[0]);
	crear_archivo(path_archivo_completo, contenido_fcb, num_contenidos);
	t_config* config_nuevo_fcb = iniciar_config_test(path_archivo_completo);

	config_set_value(config_nuevo_fcb, "NOMBRE_ARCHIVO", nombreArchivo);
	config_save_in_file(config_nuevo_fcb, path_archivo_completo);
}


t_config* iniciar_config_test(char * path_config) {
	t_config* config = iniciar_config(path_config);
	
	if(config == NULL) {
		log_error(logger, "No se pudo leer el path %s", path_config);
	}

	return config;
}

void inicializar_superbloque(){
	superbloque.block_count = config_get_int_value(config_superbloque, "BLOCK_COUNT");
	superbloque.block_size = config_get_int_value(config_superbloque, "BLOCK_SIZE");
}

void crear_archivo_bloques(){
	
	int f_bloques;
	f_bloques = open(datos.path_bloques, O_RDWR);
	ftruncate(f_bloques,tam_fs);

//    F_BLOCKS = fopen(datos.path_bloques,"a");

	tam_fs = superbloque.block_count * superbloque.block_size;
	set_tamanio_archivo(F_BLOCKS, tam_fs);

	memoria_file_system = mmap(NULL,tam_fs, PROT_READ | PROT_WRITE, MAP_SHARED, f_bloques, 0);

	// crear_lista_bloques(tam_fs);
}

void set_tamanio_archivo(FILE* archivo, int tamanioArchivo){
	ftruncate(fileno(archivo), tamanioArchivo);
}

void crear_archivo_bitmap(){
	F_BITMAP = fopen(datos.path_bitmap, "a"); //TODO: revisar el modo, no deberia de ser "a+"?

	int tam_bits_bloque = convertir_byte_a_bit(superbloque.block_count);
	TAM_BITMAP = tam_bits_bloque;
	set_tamanio_archivo(F_BITMAP, TAM_BITMAP);

	crear_bitmap();
}

int convertir_byte_a_bit(int block_size){
	return block_size/8;
}

void crear_bitmap(){
	int fd;

    /* Abrimos el archivo en la direccion pasada por parametro y ajustamos el tamanio del mismo */
	fd = open(path_bitmap, O_RDWR | O_CREAT , S_IRUSR | S_IWUSR);
    if(fd==EXIT_FAILURE){
	    log_error(logger, "No se pudo abrir el archivo: %s", path_bitmap);
		// exit(EXIT_FAILURE);
	}

    ftruncate(fd, TAM_BITMAP);

	/* Creamos la estructura para modificar */
	datos_memoria = mmap(NULL, TAM_BITMAP*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, fd, 0);
	if(datos_memoria==MAP_FAILED){
	    log_error(logger, "No se pudo inicializar el mmap");
	    // exit(EXIT_FAILURE);
	}

	bitmap = bitarray_create_with_mode(datos_memoria, TAM_BITMAP, LSB_FIRST);

	for(int i = 0; i<bitmap->size; i++){
	   bitarray_clean_bit(bitmap, i);
	}

	//leer_bitmap();

	close(fd);
}


// ------------------------------- FUNCIONES - BITMAP --------------------------------------- //

void escribir_bitmap(t_list* list, int bit_presencia){
 int cantElementos = list_size(list);

 switch(bit_presencia){
    case 1:
	  log_info(logger, "Log: %i", cantElementos);
	  for(int i = 0; i < cantElementos; i++){
		  char* pos = (char*)list_get(list, i);
		  int intPos = atoi(pos);
		  off_t offset = (off_t)intPos;

		  log_info(logger, "Posicion: %i", intPos);

		  bitarray_set_bit(bitmap, offset);
	  	  log_info(logger, "Acceso a Bitmap - Bloque: %i - Estado: 1", intPos);
	  }
	  msync(bitmap->bitarray, bitmap->size,MS_SYNC);
	  break;
    case 0:
	  for(int i = 0; i < cantElementos; i++){
	  	  char* pos = (char*)list_get(list, i);
	  	  int intPos = atoi(pos);
	  	  off_t offset = (off_t)intPos;

	  	  log_info(logger, "Posicion: %i", intPos);

		  bitarray_clean_bit(bitmap, offset);
	  	  log_info(logger, "Acceso a Bitmap - Bloque: %i - Estado: 0", intPos);
	  }
	  msync(bitmap->bitarray, bitmap->size,MS_SYNC);
	  break;
    default:
	  log_error(logger, "El valor del bit de presencia no es ni 0 ni 1");
	  exit(EXIT_FAILURE);
	  break;
  }
}

void cerrar_bitmap(){
	int rc = munmap(datos_memoria, TAM_BITMAP);
	if(rc == -1){
		log_info(logger, "No se pudo cerrar el F_BITMAP - Nro. Error:");
		exit(EXIT_FAILURE);
	}
	munmap(datos_memoria, TAM_BITMAP);
	bitarray_destroy(bitmap);
}

void leer_bitarray(t_bitarray* array){
	for(size_t i = 0; i<=array->size; i++){
		off_t off_i = (off_t)i;
		int bit = (int)bitarray_test_bit(array, off_i);
		log_info(logger, "Pos. %i - Datos: %i", (int)i, bit);
	}
}

void leer_pos_bitarray(t_bitarray* array, t_list* listPos){
	for(int i = 0; i < list_size(listPos); i++){
		char* pos = (char*)list_get(listPos, (int)i);
		int intPos = atoi(pos);

		off_t off_i = (off_t)intPos;

		int bit = (int)bitarray_test_bit(array, off_i);
		log_info(logger, "Pos. %i - Datos: %i", intPos, bit);
	}
}

void leer_bitmap(){
	size_t ret;
	void *buffer = malloc(TAM_BITMAP);
	int fd;

	fd = open(path_bitmap, O_RDWR | O_CREAT , S_IRUSR | S_IWUSR);
	if (!fd){
        log_error(logger, "No se pudo abrir el archivo");
    }

	ret = read(fd, buffer, TAM_BITMAP);

	for(size_t i = 0; i < ret; i++){
		//Manipulo bytes individuales del buffer
		char* bytePtrBuffer = (char*)buffer;
		//Calculo la direccion de memoria que quiero
		char* ptrObtenido = bytePtrBuffer + i;
	    log_info(logger, "Pos. %i - Datos: %d", (int)i, *((int*)ptrObtenido));
	}

	free(buffer);
	close(fd);
}

// ---------------------------------- FUNCIONES - ARCHIVOS - GENERAL -------------------------- //

void leer_archivo(FILE* archivo, size_t tamArchivo, char* path){
	size_t ret;
	char* buffer = malloc((int)tamArchivo);
	int dif;
    int fd;

	fd = open(path, O_RDWR | O_CREAT , S_IRUSR | S_IWUSR);
	if (!fd){
        log_error(logger, "No se pudo abrir el archivo");
    }
	
	// Obtenemos la cantidad de valores leidos con ret y los almacenamos en el buffer
    ret = read(fd, buffer, tamArchivo);

    //Lectura del archivo cada 4 valores hexadecimales 
	for(size_t i = 0; i < ret; i+=4){
	    log_info(logger, "Lec. %i - Dato: %02X %02X %02X %02X", (int)i , buffer[i], buffer[i+1], buffer[i+2], buffer[i+3]);
		dif = (int)ret - i;
	}
    
	//Leo valores hexadecimales restantes del archivo si me quedasen 
	if(dif != 0){
	    for(size_t i = dif; i < ret; i++){
	        log_info(logger, "Lec. %i - Dato: %02X", (int)i , buffer[i]);	
	    }
	}

	free(buffer);
	close(fd);
}

void* leer_dato_del_bloque(int offset, int size){
	void* dato = malloc(size);
	log_info(logger, "Paso por leer");
	memcpy(dato,memoria_file_system + offset, size);
	msync(memoria_file_system,tam_fs,MS_SYNC);
	//sleep();

	return dato;
}

void escribir_dato_en_bloque(void* dato, int offset, int size){
	log_info(logger, "El el Tam en ESCRIBIR es %i", tam_fs);
	memoria_file_system = mmap(NULL,tam_fs, PROT_READ | PROT_WRITE, MAP_SHARED, F_BLOCKS, 0);
	log_info(logger, "El el Tam en ESCRIBIR es %i", tam_fs);
	memcpy(memoria_file_system,dato, size);
	log_info(logger, "El el Tam en ESCRIBIR es %i", tam_fs);
	log_info(logger, "El dato en ESCRIBIR es %s", dato);
	log_info(logger, "El el Tam en ESCRIBIR es %i", tam_fs);
	msync(memoria_file_system,tam_fs,MS_SYNC);
	//sleep();
}

//--------------------------- FUNCIONES - ARCHIVO DE BLOQUES ----------------------------- //
// void crear_lista_bloques(){
// 	//int tamanioBloque = ceil(tamanioArchivo/superbloque.block_size);
// 	lista_bloques = list_create();

// 	for(int i = 0; i <= superbloque.block_count; i++){
// 		/*
// 		t_block* bloque = malloc(sizeof(t_block));
// 		bloque->pos=i;
// 		bloque->fid=fid_generator;

// 		fid_generator += 1;

// 		list_add(lista_bloques, bloque);
// 		*/
// 	}
// }

void liberar_lista_bloques(){
	for(int i = 0; i <= superbloque.block_count; i++){
		t_block* bloque = list_get(lista_bloques, i);
		free(bloque);
	}
}

// ---------------------------------------------------------------------------------------- //
/*
void prueba(){
	t_list* pos_bloques = malloc(sizeof(int)*4);//Posiblemente sea este el problema que lea mal los datos cuando escribe
	list_add(pos_bloques, "0");
	list_add(pos_bloques, "1");
	list_add(pos_bloques, "2");
	list_add(pos_bloques, "3");
	escribir_bitmap(pos_bloques, 1);

	leer_bitarray(bitmap);

	leer_pos_bitarray(bitmap, pos_bloques);

	int tamanio = tamanio_maximo_real_archivo();

	log_info(logger, "Tamanio Maximo Real: %i", tamanio);

	int count = cant_bloques_disponibles_bitmap();

	log_info(logger, "Cant. de BLoques Disponibles: %i", count);

	free(pos_bloques);
}
*/
/*
int* transf_list_a_ptr(t_list* lista){
	int* ptr = malloc(list_size(lista)*sizeof(int));
	for(int i = 0; i<list_size(lista);i++){
		ptr[i] = list_get(lista, i);
	}

	return ptr;
}
*/

void finalizar_fs(){
	log_destroy(logger);
	config_destroy(config);
	close(server_fd);
	close(conexion_memoria);
	fclose(F_BITMAP);
	fclose(F_BLOCKS);
	// fclose(F_FCB);
	closedir(diretorio_FCB);
}

// ----------------------------------- ADICIONALES ----------------------------------- //

char* concatenar_path(char* nombre_archivo){
	char *unaPalabra = string_new();
	string_append(&unaPalabra, datos.path_fcb);
	string_append(&unaPalabra, "/");
	string_append(&unaPalabra, nombre_archivo);
	string_append(&unaPalabra, ".dat");

    return unaPalabra;
}

void limpiar_bitarray(t_bitarray* bitarray){
 	for(int i = 0; i < bitarray_get_max_bit(bitarray); i++){
 		bitarray_clean_bit(bitarray, i);
 	}
}

int cantidad_punteros_por_bloques(int block_size){
	return block_size/TAMANIO_DE_PUNTERO; 
}

//Obtiene el fcb asociado al pcb
t_fcb* obtener_fcb(t_pcb* pcb){
	int i=1;
	while(i<list_size(lista_fcb)){
		t_fcb* fcb = list_get(lista_fcb,i);
		if(strcmp(fcb->nombre_archivo,pcb->arch_a_abrir)==1){
			return fcb;
		}
		i++;
	}
	if(buscar_archivo_fcb(pcb->arch_a_abrir)==0){
		log_info(logger, "No se encontro el archivo: %s", pcb->arch_a_abrir);
	    exit(EXIT_FAILURE);
	}else{
		t_fcb* fcb = crear_fcb(pcb);
		actualizar_lista_fcb(fcb);
		return fcb;
	}
}

void actualizar_lista_fcb(t_fcb* fcb){
	log_info(logger, "Se actualiza la lista de fcbs");
	if(buscar_fcb(fcb->nombre_archivo)==0){
	    list_add(lista_fcb,fcb);
	}else{
		list_remove(lista_fcb, fcb);
	}
}


// ------------------------- MODIFICAR TAMANIO ---------------------- //
//TODO: revisar (generado con IA)
void agrandar_archivo(char* nombre_archivo, int tamanio_pedido){//TODO: Queda pendiente agregar los bloques en los punteros hasta tener las funciones LEER  y ESCRIBIR

	char *path_archivo_completo = concatenar_path(nombre_archivo);
	t_config* config_fcb = iniciar_config_test(path_archivo_completo);

    int tamanio_archivo = config_get_int_value(config_fcb,"TAMANIO_ARCHIVO");

    int diferencia = tamanio_pedido - tamanio_archivo;

	int cant_bloques_a_asignar = ceil((double) diferencia / superbloque.block_size);
	int cant_bloques_actual = ceil((double) tamanio_archivo/superbloque.block_size);

    if(cant_bloques_a_asignar<=cant_bloques_disponibles_bitmap()){
		char* tamanio_pedido_str;
    	sprintf(tamanio_pedido_str, "%d", tamanio_pedido);

    	config_set_value(config_fcb, "TAMANIO_ARCHIVO", tamanio_pedido_str);
    	config_save_in_file(config_fcb, path_archivo_completo);
		set_tamanio_archivo(nombre_archivo, tamanio_pedido); //TODO: verificar el 2do parametro

    	for(int i = cant_bloques_actual; i<cant_bloques_a_asignar; i++){
			log_info(logger, "FOR...");
    		asignar_bloque_a_archivo(nombre_archivo, config_fcb);
    	}

    }else{
    	log_warning(logger, "Se piden %i bloques para asignarle a %s, pero solo hay %i disponibles \n",cant_bloques_a_asignar,nombre_archivo,cant_bloques_actual);
    }

}

int cant_bloques_disponibles_bitmap(){
	int count = -1;
	int j = bitarray_get_max_bit(bitmap);

	for(int i = 0; i < j; i++){
		if(bitarray_test_bit(bitmap, i)==0){
		  count += 1;
		}
	}

	return count;
}

void asignar_bloque_a_archivo(char* nombreArchivo, t_config* config_fcb){

}
//----------------------------------- AUXILIARES -----------------------------------//

FILE* obtener_archivo(char* nombreArchivo){

	char *path_archivo_completo = concatenar_path(nombreArchivo);

	log_info(logger, "Entro en obtener_archivo");
	FILE* archivo = fopen(path_archivo_completo, "r");
	if(archivo==NULL){
		log_warning(logger, "Tenes un archivo en NULL");
	}
	return archivo;
}

//TODO: revisar
t_fcb* crear_fcb(t_pcb* pcb){
	t_fcb* fcb = malloc(sizeof(t_fcb*));

	char *path_archivo_completo = concatenar_path(pcb->arch_a_abrir);

//	char* pf = strcat(PATH_FCB, pcb->arch_a_abrir);
//	char* path_archivo = strcat(pf, ".dat");

	if(buscar_archivo_fcb(pcb->arch_a_abrir)==0){
	   crear_archivo_fcb(pcb->arch_a_abrir);
	}

	FILE* archivo = obtener_archivo(pcb->arch_a_abrir);

	fcb->archivo=archivo;
	fcb->path_archivo=path_archivo_completo;
	fcb->nombre_archivo=pcb->arch_a_abrir;
	fcb->tamanio_archivo=pcb->dat_tamanio;
    //set_tamanio_archivo(fcb->archivo, pcb->dat_tamanio);
    //log_info(logger, "la cosa va por aca");
	fcb->puntero_directo=0;
	fcb->puntero_indirecto=0;

    list_add(lista_fcb, fcb);

    //log_info(logger, "Se agregó el fcb a la lista de fcbs");

	return fcb;
}
// 

int buscar_archivo_fcb(char* nombre_archivo){
    //DIR* dir = opendir(datos.path_fcb);

    if (diretorio_FCB == NULL) {
        log_error(logger, "No se pudo abrir el directorio: %s", datos.path_fcb);
        return 0;
    }

    FILE* archivo = fopen(concatenar_path(nombre_archivo), "r+");

    if(archivo!=NULL){
    	log_info(logger,"Se encontro el archivo: %s",nombre_archivo);
    	return 1;
    }

	log_info(logger,"No se encontro el archivo: %s",nombre_archivo);
    return 0;
}

//Obtiene el puntero de la lista de fcbs del fcb buscado
int buscar_fcb(char* nombre_archivo){
	int i=1;
	while(i<list_size(lista_fcb)){
		t_fcb* fcb = list_get(lista_fcb,i);
		if(strcmp(fcb->nombre_archivo,nombre_archivo)==1){
			return i;
		}
		i++;
	}
	return 0;
}

int tamanio_maximo_real_archivo(){
	return ((superbloque.block_size/4) + 1)* superbloque.block_size;;
}

//TODO: revisar (generado con IA)
void achicar_archivo(t_fcb* fcb, int tamanio){
	FILE* archivo = fopen(fcb->nombre_archivo, "r+");
	fseek(archivo, 0, SEEK_END);
	int tamanio_actual = ftell(archivo);
	int diferencia = tamanio_actual - tamanio;
	char* buffer = malloc(diferencia);
	memset(buffer, '\0', diferencia);
	fwrite(buffer, diferencia, 1, archivo);
	fclose(archivo);
	free(buffer);
}

void iterator(char* value) {
	log_info(logger,"%s", value);
}
//----------------------------------- KERNEL -----------------------------------//

void* atender_kernel(void){
	server_fd = iniciar_servidor(logger,datos.puerto_escucha);
	log_info(logger, "Fyle System listo para recibir al kernel");
	t_list* lista;
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));
	int *cliente_fd = esperar_cliente(logger,server_fd);
	t_pcb* pcb;
	t_buffer* buffer;

	t_fcb* fcb;
	PATH_FCB = strcat(datos.path_fcb, "/"); //TODO: revisar

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
			case ABRIR_ARCHIVO:
				buffer=desempaquetar(paquete,*cliente_fd);
				pcb = deserializar_pcb(buffer);
				
				log_info(logger, "Abrir Archivo: %s", pcb->arch_a_abrir);
				//log_info(logger,"El id es %d", pcb->pid);

				if(buscar_archivo_fcb(pcb->arch_a_abrir)){
					//Encontro el archivo en el directorio
					log_info(logger,"El archivo existe");

					//En caso de que no se encuentre en la lista de fcb pero exista el archivo mencionado
					if(buscar_fcb(pcb->arch_a_abrir)==0){
					   log_info(logger, "Se agrega a la lista de F_FCB");

					   crear_fcb(pcb); //TODO: por que creamos? 
					}

					//enviar_pcb_a(pcb,*cliente_fd, EXISTE_ARCHIVO);
					enviar_respuesta_kernel(cliente_fd, ARCHIVO_CREADO);
				}else{
					//log_info(logger,"El archivo %s no existe", pcb->arch_a_abrir);
					//enviar_pcb_a(pcb,*cliente_fd, CREAR_ARCHIVO);

					enviar_respuesta_kernel(cliente_fd, CREAR_ARCHIVO);//TODO: SI NO EXISTE DEBERIA DEVOLVER QUE NO SE PUDE CREAR
				}
				break;
			case CREAR_ARCHIVO:
				buffer=desempaquetar(paquete,*cliente_fd);
				pcb = deserializar_pcb(buffer);

				log_info(logger, "Crear Archivo: %s", pcb->arch_a_abrir);
				//log_info(logger, "El id es %d", pcb->pid);
			    
				crear_archivo_fcb(pcb->arch_a_abrir);

				fcb = crear_fcb(pcb);

				enviar_respuesta_kernel(cliente_fd, ARCHIVO_CREADO);

				//enviar_pcb_a(pcb,*cliente_fd,ARCHIVO_CREADO);

				break;
			case MODIFICAR_TAMANIO:
				buffer=desempaquetar(paquete,*cliente_fd);
				pcb = deserializar_pcb(buffer);

				log_info(logger, "Truncar Archivo: %s - Tamaño: %i", pcb->arch_a_abrir, pcb->dat_tamanio);

				char *path_archivo_completo = concatenar_path(pcb->arch_a_abrir);
				t_config* config_fcb = iniciar_config_test(path_archivo_completo);
			    int tamanio_archivo = config_get_int_value(config_fcb,"TAMANIO_ARCHIVO");

//				fcb = obtener_fcb(pcb);

				if(buscar_archivo_fcb(pcb->arch_a_abrir)!=0){

					if(tamanio_archivo < pcb->dat_tamanio){
						//Si el tamanio del archivo es menor al solicitado...

						if(pcb->dat_tamanio > tamanio_maximo_real_archivo()){
							log_warning(logger, "Tamanio pedido: %i es mayor que Tamanio Max: %i", pcb->dat_tamanio, tamanio_maximo_real_archivo());
						}

						log_info(logger, "Agrandar archivo: %s", pcb->arch_a_abrir);
						agrandar_archivo(pcb->arch_a_abrir, pcb->dat_tamanio);

					}else if(fcb->tamanio_archivo > pcb->dat_tamanio){
						//Si el tamanio del archivo es mayor al solicitado...

						log_info(logger, "Achica archivo: %s", pcb->arch_a_abrir);
						achicar_archivo(fcb, pcb->dat_tamanio);

					}else{
						log_info(logger, "Tiene el mismo tamanio: %s", fcb->nombre_archivo);
					}

				}

		        enviar_respuesta_kernel(cliente_fd, OK);

				break;
			case LEER_ARCHIVO:
				buffer=desempaquetar(paquete,*cliente_fd);
				pcb = deserializar_pcb(buffer);

				log_info(logger, "paso por LEER_ARCHIVO");

				//TODO:LEER_ARCHIVO

				break;
			case ESCRIBIR_ARCHIVO:
				buffer=desempaquetar(paquete,*cliente_fd);
				pcb = deserializar_pcb(buffer);
				break;
			case -1:
				log_error(logger, "el cliente se desconecto. Terminando servidor");
				log_destroy(logger);
				config_destroy(config);
				close(*cliente_fd);
				close(server_fd);
				//return EXIT_FAILURE;
				exit(1);
			default:
				log_warning(logger,"Operacion desconocida. No quieras meter la pata");
				break;
		}
	}
}

void enviar_respuesta_kernel(int *cliente_fd, op_code msj){
	send(*cliente_fd,&msj,sizeof(op_code),0);
}

//----------------------------------- MEMORIA -----------------------------------//


void* atender_memoria(void){
	conexion_memoria = crear_conexion(datos.ip_memoria,datos.puerto_memoria);
	enviar_mensaje("Hola te saludo desde el Fyle System",conexion_memoria);
	while(1){
		t_list* lista;
		while(1){
			int cod_op = recibir_operacion(conexion_memoria);
			switch (cod_op) {
				case MENSAJE:
					recibir_mensaje(logger,conexion_memoria);
					break;
				case PAQUETE:
					lista = recibir_paquete(conexion_memoria);
					log_info(logger, "Me llegaron los siguientes valores:\n");
					list_iterate(lista, (void*) iterator);
					break;
				case -1:
					log_error(logger, "el cliente se desconecto. Terminando servidor");
					log_destroy(logger);
					config_destroy(config);
					close(conexion_memoria);
					//return EXIT_FAILURE;
					exit(1);
				default:
					log_warning(logger,"Operacion desconocida. No quieras meter la pata");
					break;
			}
		}
	}
}
