#ifndef HUFFMAN_TABLES_H
#define HUFFMAN_TABLES_H

#include <stdint.h>

// Estructura para definir las propiedades de cada una de las 32 tablas
typedef struct {
    const uint16_t *table;     // Puntero al array de códigos de la tabla
    uint8_t max_value;         // El valor máximo (usualmente 15)
    uint8_t linbits;           // Cuántos bits extras lee si el valor es max_value
} HuffmanTableInfo;




#endif