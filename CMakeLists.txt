cmake_minimum_required(VERSION 3.10)
project(ReprA_API C)

# 1. BANDERAS DE OPTIMIZACIÓN EXTREMA
# -mavx2: Activa AVX2
# -mfma: Permite (a*b)+c en un solo ciclo (vital para audio)
# -O3: Optimización máxima del compilador
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx2 -mfma -O3 -Wall")

# 2. DIRECTORIOS DE CABECERAS
# Le decimos a CMake dónde buscar los archivos .h
include_directories(include src)


# 3. ARCHIVOS FUENTE
# Aquí listamos todos tus archivos .c (la "cocina")
find_package(ALSA REQUIRED)
set(SOURCES 
src/alloc.c 
src/engine.c
src/dsp_avx2.c
src/loader.c
src/backend_alsa.c
src/repra_threads.c
src/mp3_internals.c
# Cuando crees src/dsp_avx2.c, lo agregas aquí abajo
)

# 4. CREAR EL TEST (EJECUTABLE)
# Esto crea un programa llamado 'repra_test' usando tus fuentes y el main de pruebas
add_executable(repra_test ${SOURCES} tests/test_main.c)
target_link_libraries(repra_test PRIVATE ALSA::ALSA)