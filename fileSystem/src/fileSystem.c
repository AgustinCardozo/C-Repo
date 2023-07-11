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

char* str1 = "/";
char* path_bitmap = "/home/utnso/fs/BITMAPA.dat";

int main(void) {
	logger = iniciar_logger(FS_LOG, FS_NAME);;
	config = iniciar_config(FS_CONFIG);

	inicializar_config();
	iniciar_estructura_fs(contenido_superbloque);

	log_info(logger,"PATH_SUPERBLOQUE: %s", datos.path_superbloque);
	log_info(logger,"PATH_BITMAP: %s", datos.path_bitmap);
	log_info(logger,"PATH_BLOQUES: %s", datos.path_bloques);
	log_info(logger,"PATH_FCB: %s", datos.path_fcb);

	int block_size = config_get_int_value(config_superbloque, "BLOCK_SIZE");
	int block_count = config_get_int_value(config_superbloque, "BLOCK_COUNT");

	int tamanio_bitmap = convertir_byte_a_bit(block_count);
	set_tamanio_archivo(bitmap, convertir_byte_a_bit(block_count));

	crear_bitmap(tamanio_bitmap);

	limpiar_bitarray(bitarray);
    
	log_info(logger, "tamanio del bitmap: %i", (int)bitarray_test_bit(bitarray, 8192+10));
	bitarray_clean_bit(bitarray, 8192+10);
	log_info(logger, "tamanio del bitmap: %i", (int)bitarray_test_bit(bitarray, 8192+10));

    char* nombre_archivo = "test_fcb";    
    char resultado[50];
	char directorio[100];

	strcpy(directorio, datos.path_fcb);
    strcpy(resultado, str1); 
    strcat(resultado, nombre_archivo); 
	strcpy(resultado, strcat(directorio, resultado));
    // char* resultado = concatenar_path(nombre_archivo);

	log_info(logger, "%s", resultado);
	log_info(logger,"PATH_FCB: %s", datos.path_fcb);
	//TODO
	if(abrir_archivo_fcb(datos.path_fcb, nombre_archivo) == "ERROR"){
		log_error(logger, "No existe el archivo: %s", nombre_archivo);
	};
	char* lala = crear_archivo_fcb(resultado);
	printf("%s\n", lala);
	log_info(logger, "Crear Archivo: %s", "test_fcb");
	lala = abrir_archivo_fcb(datos.path_fcb, nombre_archivo);
	printf("%s\n", lala);

	pthread_create(&hilo_conexion_kernel,NULL,(void*) atender_kernel,NULL);
	pthread_create(&hilo_conexion_memoria,NULL,(void*) atender_memoria,NULL);

	pthread_join(hilo_conexion_kernel,NULL);
	pthread_join(hilo_conexion_memoria,NULL);

	finalizar_fs();
	return EXIT_SUCCESS;
}

void limpiar_bitarray(t_bitarray* bitarray){
 	for(int i = 0; i < bitarray_get_max_bit(bitarray); i++){
 		bitarray_clean_bit(bitarray, i);
 	}
}

int convertir_byte_a_bit(int block_count){
	return block_count/8;
}

int cantidad_punteros_por_bloques(int block_size){
	return block_size/TAMANIO_DE_PUNTERO; 
}

int tamanio_maximo_teorico_archivo(){
	//TODO: implementar
	return 0;
}

void crear_bitmap(int tamanio_bitmap){
    int fd = fd = open(path_bitmap, O_RDWR | O_CREAT , S_IRUSR | S_IWUSR);
	ftruncate(fd, tamanio_bitmap);

	int *map = mmap(NULL, tamanio_bitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	
    leer_archivo(fd);

	if (map == MAP_FAILED) {
		log_error(logger, "Error al mapear el archivo");
		exit(EXIT_FAILURE);
    }

	void* puntero_a_bits = malloc(tamanio_bitmap);
	bitarray_create(puntero_a_bits, 1);
}

void modificar_bitmap(char* path, int tamanio_bitmap, char* op){
	switch(op){
       case: "ESCRITURA":
	    break;
	   case: "LECTURA":
	    break;
		default: 
	}
}

void leer_archivo(int fd){
    fd = read(fd, contenido_superbloque, sizeof(contenido_superbloque));
	log_info(logger, "Contenido del archivo: %s", contenido_superbloque);	
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
	// reemplazar_y_concatenar_palabra(datos.path_fcb, "~/fs", "/home/utnso/fs/", "fcb");

}

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

void set_tamanio_archivo(FILE* bloques, int tamanioBloque){
    if (ftruncate(fileno(bloques), tamanioBloque) == -1) {
        perror("Error al establecer el tamaño del archivo");
        exit(EXIT_FAILURE);
    }
}

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
	bitmap = fopen(datos.path_bitmap,"a");
	crear_bloque(bloques, config_superbloque);
	// bloques = fopen(datos.path_bloques,"a");
	valor = mkdir(datos.path_fcb, 0777);
	if(valor < 0){
		log_warning(logger, "Ya existe el directo(%s)", PATH);
	}
}

void crear_bloque(FILE* bloques, t_config* config_superbloque){
	bloques = fopen(datos.path_bloques,"a");
	int block_size = config_get_int_value(config_superbloque, "BLOCK_SIZE");
	int block_count = config_get_int_value(config_superbloque, "BLOCK_COUNT");

	int tamanioBloque = block_size * block_count;
	set_tamanio_archivo(bloques, tamanioBloque);
}

//TODO: revisar
char* crear_archivo_fcb(char path_name[]){
	FILE* fcb = crear_estructuras(path_name);
	set_tamanio_archivo(fcb, 0);
	fclose(fcb);
	return "OK";
}

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

t_config* iniciar_config_fs(char * path) {
	t_config* config = iniciar_config(path);
	
	if(config == NULL) {
		log_error(logger, "No se pudo leer el path (%s)", path);
	}

	return config;
}

FILE* crear_estructuras(char* path){
	FILE* file =fopen(path,"a");
	return file; 
}

void finalizar_fs(){
	log_destroy(logger);
	config_destroy(config);
	close(server_fd);
	close(conexion_memoria);
	fclose(bitmap);
	fclose(bloques);
	fclose(fcb);
}

void* atender_kernel(void){
	server_fd = iniciar_servidor(logger,datos.puerto_escucha);
	log_info(logger, "Fyle System listo para recibir al kernel");
	int *cliente_fd = esperar_cliente(logger,server_fd);
	while(1){
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
}

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
