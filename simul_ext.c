#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

// Prototipos de las funciones
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, char *nombre_antiguo, char *nombre_nuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps,
           EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps,
           EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos,
           char *nombre_origen, char *nombre_destino);

int main() {
    // Variables para manejar comandos y argumentos
    char comando[LONGITUD_COMANDO];
    char orden[LONGITUD_COMANDO];
    char argumento1[LONGITUD_COMANDO];
    char argumento2[LONGITUD_COMANDO];

    // Estructuras para manejar el sistema de ficheros
    EXT_SIMPLE_SUPERBLOCK ext_superblock;
    EXT_BYTE_MAPS ext_bytemaps;
    EXT_BLQ_INODOS ext_blq_inodos;
    EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
    EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
    FILE *fent; // Archivo binario que representa la partición
    EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];

    // Abrir el archivo binario de la partición
    fent = fopen("particion.bin", "r+b");
    if (fent == NULL) {
        printf("Error al abrir particion.bin\n");
        return -1;
    }

    // Leer la partición completa en memoria
    fread(datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);

    // Mapear cada parte de la partición en sus respectivas estructuras
    memcpy(&ext_superblock, (EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
    memcpy(&ext_bytemaps, (EXT_BYTE_MAPS *)&datosfich[1], SIZE_BLOQUE);
    memcpy(&ext_blq_inodos, (EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
    memcpy(directorio, (EXT_ENTRADA_DIR *)&datosfich[3], MAX_FICHEROS * sizeof(EXT_ENTRADA_DIR));
    memcpy(memdatos, (EXT_DATOS *)&datosfich[4], MAX_BLOQUES_DATOS * SIZE_BLOQUE);

    // Bucle principal de comandos
    for (;;) {
        printf(">> "); // Prompt para ingresar comandos
        fgets(comando, LONGITUD_COMANDO, stdin);
        sscanf(comando, "%s %s %s", orden, argumento1, argumento2);

        // Comando: info
        if (strcmp(orden, "info") == 0) {
            LeeSuperBloque(&ext_superblock);
            continue;
        }

        // Comando: bytemaps
        if (strcmp(orden, "bytemaps") == 0) {
            Printbytemaps(&ext_bytemaps);
            continue;
        }

        // Comando: dir
        if (strcmp(orden, "dir") == 0) {
            Directorio(directorio, &ext_blq_inodos);
            continue;
        }

        // Comando: rename
        if (strcmp(orden, "rename") == 0) {
            if (strlen(argumento1) == 0 || strlen(argumento2) == 0) {
                printf("Uso: rename <nombre_antiguo> <nombre_nuevo>\n");
                continue;
            }
            Renombrar(directorio, argumento1, argumento2);
            continue;
        }

        // Comando: imprimir
        if (strcmp(orden, "imprimir") == 0) {
            if (strlen(argumento1) == 0) {
                printf("Uso: imprimir <nombre_archivo>\n");
                continue;
            }
            Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1);
            continue;
        }

        // Comando: remove
        if (strcmp(orden, "remove") == 0) {
            if (strlen(argumento1) == 0) {
                printf("Uso: remove <nombre_archivo>\n");
                continue;
            }
            Borrar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1);
            continue;
        }

        // Comando: copy
        if (strcmp(orden, "copy") == 0) {
            if (strlen(argumento1) == 0 || strlen(argumento2) == 0) {
                printf("Uso: copy <nombre_origen> <nombre_destino>\n");
                continue;
            }
            Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos, argumento1, argumento2);
            continue;
        }

        // Comando: salir
        if (strcmp(orden, "salir") == 0) {
            fclose(fent); // Cerrar el archivo binario
            return 0;
        }

        // Comando desconocido
        printf("Comando desconocido: %s\n", orden);
    }

    return 0;
}

// Función: Mostrar información del superbloque
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup) {
    printf("Información del superbloque:\n");
    printf("Número total de inodos: %u\n", psup->s_inodes_count);
    printf("Número total de bloques: %u\n", psup->s_blocks_count);
    printf("Bloques libres: %u\n", psup->s_free_blocks_count);
    printf("Inodos libres: %u\n", psup->s_free_inodes_count);
    printf("Primer bloque de datos: %u\n", psup->s_first_data_block);
    printf("Tamaño del bloque: %u bytes\n", psup->s_block_size);
}

// Función: Mostrar el estado de los bytemaps
void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps) {
    int i;

    printf("Bytemap de bloques (primeros 25 elementos):\n");
    for (i = 0; i < 25; i++) {
        printf("%u ", ext_bytemaps->bmap_bloques[i]);
    }
    printf("\n");

    printf("Bytemap de inodos:\n");
    for (i = 0; i < MAX_INODOS; i++) {
        printf("%u ", ext_bytemaps->bmap_inodos[i]);
    }
    printf("\n");
}

// Función: Listar los archivos en el directorio
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos) {
    printf("Directorio:\n");
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != NULL_INODO) {
            EXT_SIMPLE_INODE *inode = &inodos->blq_inodos[directorio[i].dir_inodo];
            printf("Nombre: %s\n", directorio[i].dir_nfich);
            printf("Tamaño: %u bytes\n", inode->size_fichero);
            printf("Inodo: %u\n", directorio[i].dir_inodo);
            printf("Bloques: ");
            for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
                if (inode->i_nbloque[j] != NULL_BLOQUE) {
                    printf("%u ", inode->i_nbloque[j]);
                }
            }
            printf("\n\n");
        }
    }
}

// Función: Renombrar un archivo
int Renombrar(EXT_ENTRADA_DIR *directorio, char *nombre_antiguo, char *nombre_nuevo) {
    int i;
    int encontrado = -1;

    for (i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != NULL_INODO && strcmp(directorio[i].dir_nfich, nombre_nuevo) == 0) {
            printf("Error: El archivo con el nombre '%s' ya existe.\n", nombre_nuevo);
            return -1;
        }
    }

    for (i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != NULL_INODO && strcmp(directorio[i].dir_nfich, nombre_antiguo) == 0) {
            encontrado = i;
            break;
        }
    }

    if (encontrado == -1) {
        printf("Error: No se encontró el archivo '%s'.\n", nombre_antiguo);
        return -1;
    }

    strncpy(directorio[encontrado].dir_nfich, nombre_nuevo, LEN_NFICH - 1);
    directorio[encontrado].dir_nfich[LEN_NFICH - 1] = '\0';
    printf("Archivo '%s' renombrado a '%s'.\n", nombre_antiguo, nombre_nuevo);

    return 0;
}

// Función: Mostrar el contenido de un archivo
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre) {
    int i, j;
    int encontrado = -1;

    for (i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != NULL_INODO && strcmp(directorio[i].dir_nfich, nombre) == 0) {
            encontrado = i;
            break;
        }
    }

    if (encontrado == -1) {
        printf("Error: No se encontró el archivo '%s'.\n", nombre);
        return -1;
    }

    EXT_SIMPLE_INODE *inode = &inodos->blq_inodos[directorio[encontrado].dir_inodo];
    if (inode->size_fichero == 0) {
        printf("El archivo '%s' está vacío.\n", nombre);
        return 0;
    }

    printf("Contenido del archivo '%s':\n", nombre);
    for (j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
        if (inode->i_nbloque[j] != NULL_BLOQUE) {
            printf("%s", memdatos[inode->i_nbloque[j]].dato);
        }
    }
    printf("\n");

    return 0;
}

// Función: Eliminar un archivo del sistema
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps,
           EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre) {
    int i, j;
    int encontrado = -1;

    for (i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != NULL_INODO && strcmp(directorio[i].dir_nfich, nombre) == 0) {
            encontrado = i;
            break;
        }
    }

    if (encontrado == -1) {
        printf("Error: No se encontró el archivo '%s'.\n", nombre);
        return -1;
    }

    unsigned short inodo_idx = directorio[encontrado].dir_inodo;
    EXT_SIMPLE_INODE *inode = &inodos->blq_inodos[inodo_idx];

    for (j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
        if (inode->i_nbloque[j] != NULL_BLOQUE) {
            ext_bytemaps->bmap_bloques[inode->i_nbloque[j]] = 0;
            inode->i_nbloque[j] = NULL_BLOQUE;
            ext_superblock->s_free_blocks_count++;
        }
    }

    ext_bytemaps->bmap_inodos[inodo_idx] = 0;
    inode->size_fichero = 0;
    ext_superblock->s_free_inodes_count++;

    memset(directorio[encontrado].dir_nfich, 0, LEN_NFICH);
    directorio[encontrado].dir_inodo = NULL_INODO;

    printf("Archivo '%s' eliminado correctamente.\n", nombre);
    return 0;
}

// Función: Copiar un archivo dentro del sistema
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps,
           EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos,
           char *nombre_origen, char *nombre_destino) {
    int i, j;
    int encontrado = -1;

    for (i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != NULL_INODO && strcmp(directorio[i].dir_nfich, nombre_origen) == 0) {
            encontrado = i;
            break;
        }
    }

    if (encontrado == -1) {
        printf("Error: No se encontró el archivo origen '%s'.\n", nombre_origen);
        return -1;
    }

    for (i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo != NULL_INODO && strcmp(directorio[i].dir_nfich, nombre_destino) == 0) {
            printf("Error: El archivo destino '%s' ya existe.\n", nombre_destino);
            return -1;
        }
    }

    int inodo_nuevo = -1;
    for (i = 0; i < MAX_INODOS; i++) {
        if (ext_bytemaps->bmap_inodos[i] == 0) {
            inodo_nuevo = i;
            break;
        }
    }

    if (inodo_nuevo == -1) {
        printf("Error: No hay inodos libres.\n");
        return -1;
    }

    EXT_SIMPLE_INODE *inode_origen = &inodos->blq_inodos[directorio[encontrado].dir_inodo];
    EXT_SIMPLE_INODE *inode_destino = &inodos->blq_inodos[inodo_nuevo];
    inode_destino->size_fichero = inode_origen->size_fichero;

    for (j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) {
        if (inode_origen->i_nbloque[j] != NULL_BLOQUE) {
            int bloque_nuevo = -1;
            for (int k = PRIM_BLOQUE_DATOS; k < MAX_BLOQUES_PARTICION; k++) {
                if (ext_bytemaps->bmap_bloques[k] == 0) {
                    bloque_nuevo = k;
                    ext_bytemaps->bmap_bloques[k] = 1;
                    ext_superblock->s_free_blocks_count--;
                    break;
                }
            }

            if (bloque_nuevo == -1) {
                printf("Error: No hay bloques libres.\n");
                return -1;
            }

            inode_destino->i_nbloque[j] = bloque_nuevo;
            memcpy(memdatos[bloque_nuevo].dato, memdatos[inode_origen->i_nbloque[j]].dato, SIZE_BLOQUE);
        } else {
            inode_destino->i_nbloque[j] = NULL_BLOQUE;
        }
    }

    ext_bytemaps->bmap_inodos[inodo_nuevo] = 1;
    ext_superblock->s_free_inodes_count--;

    for (i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_inodo == NULL_INODO) {
            strncpy(directorio[i].dir_nfich, nombre_destino, LEN_NFICH - 1);
            directorio[i].dir_nfich[LEN_NFICH - 1] = '\0';
            directorio[i].dir_inodo = inodo_nuevo;
            printf("Archivo '%s' copiado como '%s'.\n", nombre_origen, nombre_destino);
            return 0;
        }
    }

    printf("Error: No hay espacio en el directorio.\n");
    return -1;
}
