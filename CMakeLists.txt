cmake_minimum_required(VERSION 3.10)
project(HeliotropeRabbits C)

# Use C17 standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS_DEBUG_INIT "-Wall -g -fsanitize=unknown -fsanitize=address -Og -lm -fopenmp -march=native")
set(CMAKE_C_FLAGS_RELEASE_INIT "-Wall -Ofast -lm -fopenmp -march=native")

# Main Program
add_executable(HeliotropeRabbits main.c)

# OpenMP Handling
find_package(OpenMP REQUIRED)
target_link_libraries(HeliotropeRabbits PUBLIC OpenMP::OpenMP_C)

# MPI Handling
find_package(MPI REQUIRED)
message(STATUS "Run: ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} ${MPIEXEC_MAX_NUMPROCS} ${MPIEXEC_PREFLAGS} EXECUTABLE ${MPIEXEC_POSTFLAGS} ARGS")
target_link_libraries(HeliotropeRabbits PUBLIC MPI::MPI_C)