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
#include <stdlib.h>
#include "fileSystem.h"

size_t TAM_BITMAP;

char* str1 = "/";
char* path_bitmap = "/home/utnso/fs/BITMAPA.dat";

int main(void) {
	logger = iniciar_logger(FS_LOG, FS_NAME);;
	config = iniciar_config(FS_CONFIG);

	inicializar_config();
	inicializar_superbloque();
	iniciar_estructura_fs(contenido_superbloque);

	log_info(logger,"PATH_SUPERBLOQUE: %s", datos.path_superbloque);
	log_info(logger,"PATH_BITMAP: %s", datos.path_bitmap);
	log_info(logger,"PATH_BLOQUES: %s", datos.path_bloques);
	log_info(logger,"PATH_FCB: %s", datos.path_fcb);

    int tam_bits_bloque = convertir_byte_a_bit(superbloque.block_size);
	TAM_BITMAP = tam_bits_bloque*superbloque.block_count;
	set_tamanio_archivo(F_BITMAP, TAM_BITMAP);

	crear_bitmap();
		
    crear_archivo_bloques(F_BLOCKS);

	prueba();

    //---------------------------------------------//
    char* nombre_archivo = "test_fcb";    
    char resultado[50];
	char directorio[100];

	strcpy(directorio, datos.path_fcb);
    strcpy(resultado, str1); 
    strcat(resultado, nombre_archivo); 
	strcpy(resultado, strcat(directorio, resultado));

	log_info(logger, "%s", resultado);
	log_info(logger,"PATH_FCB: %s", datos.path_fcb);
    //--------------------------------------------//

	pthread_create(&hilo_conexion_kernel,NULL,(void*) atender_kernel,NULL);
	pthread_create(&hilo_conexion_memoria,NULL,(void*) atender_memoria,NULL);

	pthread_join(hilo_conexion_kernel,NULL);
	pthread_join(hilo_conexion_memoria,NULL);

	cerrar_bitmap();
	finalizar_fs();
	return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------//
	//modificar_puntero_BITMAP(0,1);

	//limpiar_bitarray(bitarray);
    
	// log_info(logger, "tamanio del bitmap: %i", (int)bitarray_test_bit(bitarray, 8192+10));
	// bitarray_clean_bit(bitarray, 8192+10);
	// log_info(logger, "tamanio del bitmap: %i", (int)bitarray_test_bit(bitarray, 8192+10));
    // -----------------------------------------------------------------------------//

// ---------------------------------- INICIALIZAR ----------------------------------- //

void iniciar_estructura_fs(const char *contenidos[]){
	int valor = mkdir(PATH, 0777);
	if(valor < 0){
		log_warning(logger, "Ya existe el directo(%s)", PATH);
	}
	int num_contenidos = sizeof(contenido_superbloque) / sizeof(contenido_superbloque[0]);
	crear_archivo(datos.path_superbloque, contenido_superbloque, num_contenidos);
	config_superbloque = iniciar_config_fs(datos.path_superbloque);
	// config_bitmap = iniciar_config_fs(datos.path_bitmap);
	// config_bloques = iniciar_config_fs(datos.path_bloques);
	// config_fcb = iniciar_config_fs(datos.path_fcb);
	
	//Si existe la carpeta crea los archivos
	F_BITMAP = fopen(datos.path_bitmap,"a");
	crear_archivo_bloques(F_BLOCKS);
	// bloques = fopen(datos.path_bloques,"a");
	valor = mkdir(datos.path_fcb, 0777);
	if(valor < 0){
		log_warning(logger, "Ya existe el directo(%s)", PATH);
	}
}

t_config* iniciar_config_fs(char * path) {
	t_config* config = iniciar_config(path);
	
	if(config == NULL) {
		log_error(logger, "No se pudo leer el path (%s)", path);
	}

	return config;
}

void inicializar_config(){
	datos.ip_memoria = config_get_string_value(config,"IP_MEMORIA");
	datos.puerto_memoria = config_get_string_value(config,"PUERTO_MEMORIA");
	datos.puerto_escucha = config_get_string_value(config,"PUERTO_ESCUCHA");
	datos.path_superbloque = config_get_string_value(config,"PATH_SUPERBLOQUE");
	datos.path_bitmap = config_get_string_value(config,"PATH_BITMAP");
	datos.path_bloques = config_get_string_value(config,"PATH_BLOQUES");
	datos.path_fcb = config_get_string_value(config,"PATH_FCB");
	datos.ret_acceso_bloque = config_get_int_value(config,"RETARDO_ACCESO_BLOQUE");

	// reemplazar_y_concatenar_palabra(datos.path_superbloque, "~/fs/sbloque.dat", "/home/utnso/fs/", "superbloque.dat");
	// reemplazar_y_concatenar_palabra(datos.path_bitmap, "~/fs", "/home/utnso/fs/", "bitmap.dat");
	// reemplazar_y_concatenar_palabra(datos.path_bloques, "~/fs", "/home/utnso/fs/", "bloques.dat");
	// reemplazar_y_concatenar_palabra(datos.path_fcb, "~/fs", "/home/utnso/fs/", "F_FBC");

}

void inicializar_superbloque(){
	superbloque.block_count = config_get_int_value(config_superbloque, "BLOCK_COUNT");
	superbloque.block_size = config_get_int_value(config_superbloque, "BLOCK_SIZE");
}

// ------------------------------- FUNCIONES - BITMAP --------------------------------------- //

void modificar_bit_BITMAP(int* punteros, int bit_presencia){
  //TODO: revisar luego
}

void crear_bitmap(){
	int fd;

    /* Abrimos el archivo en la direccion pasada por parametro y ajustamos el tamanio del mismo */
	fd = open(path_bitmap, O_RDWR | O_CREAT , S_IRUSR | S_IWUSR);
    if(fd==EXIT_FAILURE){
	    log_info(logger, "NO se pudo abrir el archivo del F_BITMAP");
		exit(EXIT_FAILURE);
	}

	leer_bitmap();

	/* Creamos la estructura para modificar */
	datos_memoria = mmap(NULL, TAM_BITMAP*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_POPULATE, fd, 0);
	if(datos_memoria==MAP_FAILED){
	    log_info(logger, "NO se pudo inicializar el mmap");
	    exit(EXIT_FAILURE);
	}

	bitmap = bitarray_create_with_mode(datos_memoria, TAM_BITMAP, LSB_FIRST);

	fclose(fd);
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

void leer_bitmap(){
	FILE* archivo;
	size_t ret;
	unsigned int buffer[TAM_BITMAP];

	archivo = fopen(datos.path_bitmap, "rb");
	if (!archivo){
        log_error(logger, "No se pudo abrir el archivo");
    }
	
    ret = fread(buffer, sizeof(*buffer), (sizeof(buffer)/sizeof((buffer)[0])), archivo);
    
	for(size_t i = 0; i < ret; i++){
	    log_info(logger, "Pos. %i - Datos: %i\n", (int)i , buffer[i]);	
	}

	fclose(archivo);
}

// ---------------------------------- FUNCIONES - ARCHIVOS - GENERAL -------------------------- //

void leer_archivo(FILE* archivo, size_t tamArchivo, char* path){
	size_t ret;
	unsigned char buffer[tamArchivo];
	int dif;

	archivo = fopen(path, "rb");
	if (!archivo){
        log_error(logger, "No se pudo abrir el archivo");
    }
	
	// Obtenemos la cantidad de valores leidos con ret y los almacenamos en el buffer
    ret = fread(buffer, sizeof(*buffer), (sizeof(buffer)/sizeof((buffer)[0])), archivo);

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

	fclose(archivo);
}

void set_tamanio_archivo(FILE* archivo, int tamanioArchivo){
    if (ftruncate(fileno(archivo), tamanioArchivo) == -1) {
        perror("Error al establecer el tamaño del archivo");
        exit(EXIT_FAILURE);
    }
	ftruncate(archivo, tamanioArchivo);
}

FILE* abrir_archivo(char* path, char* modo){
	FILE* file =fopen(path, modo);
	return file; 
}

//--------------------------- FUNCIONES - ARCHIVO DE BLOQUES ----------------------------- //

void crear_archivo_bloques(FILE* archivo){
    archivo = fopen(datos.path_bloques,"a");
	
	int tamanioArchivo = superbloque.block_count * superbloque.block_size;
	set_tamanio_archivo(archivo, tamanioArchivo);

	fclose(archivo);
}

// ---------------------------------------------------------------------------------------- //

void prueba(){
	int* punteros = {2,3,4,5};
	modificar_bit_BITMAP(punteros, 1);
	leer_archivo(F_BITMAP, TAM_BITMAP, datos.path_bitmap);
	
}

/*
void crear_archivo(char* path, const char *contenidos[], int num_contenidos){
	FILE *archivo;
	archivo = fopen(path,"w");
	for(int i = 0; i < num_contenidos; i++){
		fprintf(archivo, "%s\n", contenidos[i]);
	}
	fclose(archivo);
}
*/

//----------------------------------------------------------------------//

//TODO: revisar el uso de la funcion
/*
char* abrir_archivo_fcb(char* directorio, char* nombre_archivo){
	// return access(path_name, F_OK) == 0 ? "ERROR" : "OK";
    DIR* dir = opendir(directorio);

    if (dir == NULL) {
        log_error(logger, "No se pudo abrir el directorio: %s", directorio);
        return "ERROR";
    }

    struct dirent* entry;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, nombre_archivo) == 0) {
            return "OK";
            closedir(dir);
        }
    }

    return "ERROR";
    closedir(dir);

}
*/

void finalizar_fs(){
	log_destroy(logger);
	config_destroy(config);
	close(server_fd);
	close(conexion_memoria);
	fclose(F_BITMAP);
	fclose(F_BLOCKS);
	fclose(F_FCB);
}

// ----------------------------------- ADICIONALES ----------------------------------- //
char* concatenar_path(char* nombre_archivo){
	char* str1 = "/";
    char resultado[50];
	char directorio[100];

	strcpy(directorio, datos.path_fcb);
    strcpy(resultado, str1); 
    strcat(resultado, nombre_archivo); 
	strcpy(resultado, strcat(directorio, resultado));

	return resultado;
}

void limpiar_bitarray(t_bitarray* bitarray){
 	for(int i = 0; i < bitarray_get_max_bit(bitarray); i++){
 		bitarray_clean_bit(bitarray, i);
 	}
}

int convertir_byte_a_bit(int block_size){
	return block_size*8;
}

int cantidad_punteros_por_bloques(int block_size){
	return block_size/TAMANIO_DE_PUNTERO; 
}

int tamanio_maximo_teorico_archivo(t_fcb* fcb, int block_size){
	//TODO: implementar
	int CPB = cantidad_punteros_por_bloques(block_size);
	return 0;
}

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
				
				log_info(logger,"El id es %d", pcb->pid);

				if(buscar_archivo_fcb(pcb->arch_a_abrir)){
					//Encontro el archivo en el directorio
					log_info(logger,"El archivo existe");

					if(buscar_fcb(pcb->arch_a_abrir)==0){
					   log_info(logger, "Se agrega a la lista de F_FCB");
					   list_add(lista_fcb, obtener_fcb(pcb->arch_a_abrir));

					   send(server_fd,EXISTE_ARCHIVO,sizeof(op_code),0);
					}
				}else{
					log_info(logger,"El archivo no existe");
					send(server_fd,CREAR_ARCHIVO,sizeof(op_code),0);
				}
				break;
			case CREAR_ARCHIVO:
				buffer=desempaquetar(paquete,*cliente_fd);
				pcb = deserializar_pcb(buffer);
				log_info(logger, "Crear Archivo: %s", pcb->arch_a_abrir);
				log_info(logger, "El id es %d", pcb->pid);
			    
				fcb = crear_fcb(pcb);
				
				list_add(lista_fcb, fcb);

				break;
			case MODIFICAR_TAMANIO:
				buffer=desempaquetar(paquete,*cliente_fd);
				pcb = deserializar_pcb(buffer);
                
				log_info(logger, "Truncar Archivo: %s - Tamaño: %i", pcb->arch_a_abrir, pcb->dat_tamanio);

				fcb = obtener_fcb(pcb);
				
				if(fcb->tamanio_archivo < pcb->dat_tamanio){
					log_info(logger, "Agrandar archivo: %s", pcb->arch_a_abrir);
					agrandar_archivo(fcb, pcb->dat_tamanio);
				}else{
					log_info(logger, "Achica archivo: %s", pcb->arch_a_abrir);
					achicar_archivo(fcb, pcb->dat_tamanio);
				}

				send(server_fd, 1, sizeof(int), 0);

				break;
			case LEER_ARCHIVO:
				buffer=desempaquetar(paquete,*cliente_fd);
				pcb = deserializar_pcb(buffer);
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
	    return NULL;
	}else{
		t_fcb* fcb = crear_fcb(pcb);
		actualizar_lista_fcb(fcb);
		return fcb;
	}
}

void actualizar_lista_fcb(t_fcb* fcb){
	list_add(lista_fcb,fcb);
}

// ------------------------- BUSCAR FCB --------------------------- //

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

int buscar_archivo_fcb(char* nombre_archivo){
    DIR* dir = opendir(PATH_FCB);

    if (dir == NULL) {
        log_error(logger, "No se pudo abrir el directorio: %s", PATH_FCB);
        return 0;
    }

    struct dirent* entry;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, nombre_archivo) == 0) {
            return 1;
            closedir(dir);
        }
    }
	log_info(logger,"No se encontro el archivo: %s",nombre_archivo);
    return 0;
	closedir(dir);
}

// ------------------------- MODIFICAR TAMANIO ---------------------- //
//TODO: revisar (generado con IA)
void agrandar_archivo(t_fcb* fcb, int tamanio){
	FILE* archivo = fopen(fcb->nombre_archivo, "r+");
	fseek(archivo, 0, SEEK_END);
	int tamanio_actual = ftell(archivo);
	int diferencia = tamanio - tamanio_actual;
	char* buffer = malloc(diferencia);
	memset(buffer, '\0', diferencia);
	fwrite(buffer, diferencia, 1, archivo);
	fclose(archivo);
	free(buffer);
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

// --------------------------------------------------------------------- //


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

//----------------------------------- AUXILIARES -----------------------------------//
void iterator(char* value) {
	log_info(logger,"%s", value);
}

void reemplazar_y_concatenar_palabra(char *texto, const char *palabra_buscada, const char *palabra_reemplazo, const char *palabra_concatenar) {
    char *posicion;
    int longitud_palabra_concatenar = strlen(palabra_concatenar);

    // Buscar la primera aparición de la palabra buscada
    posicion = strstr(texto, palabra_buscada);

    while (posicion != NULL) {
        // Copiar la palabra de reemplazo en lugar de la palabra buscada
        strcpy(posicion, palabra_reemplazo);

        // Concatenar la palabra adicional después de la palabra de reemplazo
        strcat(posicion, palabra_concatenar);

        // Buscar la siguiente aparición de la palabra buscada
        posicion = strstr(posicion + strlen(palabra_reemplazo) + longitud_palabra_concatenar, palabra_buscada);
    }
}

void crear_archivo(const char *nombre_archivo, const char *contenidos[], int num_contenidos) {
    FILE *archivo;

    archivo = fopen(nombre_archivo, "r");

    if (archivo == NULL) {
        archivo = fopen(nombre_archivo, "w");
        if (archivo != NULL) {
            for (int i = 0; i < num_contenidos; i++) {
                fprintf(archivo, "%s\n", contenidos[i]);
            }
            fclose(archivo);
            log_info(logger, "Archivo creado exitosamente.");
        } else {
            log_error(logger,"No se pudo crear el archivo.");
        }
    } else {
        log_warning(logger, "El archivo ya existe.");
        fclose(archivo);
    }
}

FILE* obtener_archivo(char* nombreArchivo){
	FILE* archivo = fopen(nombreArchivo, "r");
	return archivo;
}

//TODO: revisar
t_fcb* crear_fcb(t_pcb* pcb){
	t_fcb* fcb = malloc(sizeof(t_fcb*));

	crear_archivo(pcb->arch_a_abrir, NULL, 0);

	FILE* archivo = obtener_archivo(pcb->arch_a_abrir);

	fcb->archivo=archivo;
	fcb->nombre_archivo=pcb->arch_a_abrir;
	fcb->tamanio_archivo=pcb->dat_tamanio;
    set_tamanio_archivo(fcb->archivo, pcb->dat_tamanio);
	fcb->puntero_directo=0;
	fcb->puntero_indirecto=0;

    list_add(lista_fcb, fcb);

	return fcb;
}
