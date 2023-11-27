cmake_minimum_required(VERSION 3.10)
project(HeliotropeRabbits C)

# Use C17 standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS_DEBUG_INIT "-Wall -g -fsanitize=unknown -fsanitize=address -Og -lm -fopenmp -march=native")
set(CMAKE_C_FLAGS_RELEASE_INIT "-Wall -Ofast -lm -fopenmp -march=native")

################
# Dependencies #
################

find_package(MPI REQUIRED)
find_package(OpenMP REQUIRED)

############
# Add Main #
############

# Main Program
add_executable(HeliotropeRabbits main.c)

# Math
target_link_libraries(HeliotropeRabbits PUBLIC m)

# OpenMP Handling
target_link_libraries(HeliotropeRabbits PUBLIC OpenMP::OpenMP_C)

# MPI Handling
message(STATUS "Run: ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} ${MPIEXEC_MAX_NUMPROCS} ${MPIEXEC_PREFLAGS} EXECUTABLE ${MPIEXEC_POSTFLAGS} ARGS")
target_link_libraries(HeliotropeRabbits PUBLIC MPI::MPI_C)

##################
# Add Sequential #
##################

# Sequential Program
add_executable(HeliotropeRabbits-Sequential sequential.c)

# Math
target_link_libraries(HeliotropeRabbits-Sequential PUBLIC m)

################
# Add MPI Only #
################

# Main Program
add_executable(HeliotropeRabbits-MPI mpi.c)

# Math
target_link_libraries(HeliotropeRabbits-MPI PUBLIC m)

# OpenMP Handling
target_link_libraries(HeliotropeRabbits-MPI PUBLIC OpenMP::OpenMP_C)

# MPI Handling
message(STATUS "Run: ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} ${MPIEXEC_MAX_NUMPROCS} ${MPIEXEC_PREFLAGS} EXECUTABLE ${MPIEXEC_POSTFLAGS} ARGS")
target_link_libraries(HeliotropeRabbits-MPI PUBLIC MPI::MPI_C)

######################################
# Add MPI + OMP on Thread Serialized #
######################################

# Main Program
add_executable(HeliotropeRabbits-MPI-Serialized mpi-omp-serialized.c)

# Math
target_link_libraries(HeliotropeRabbits-MPI-Serialized PUBLIC m)

# OpenMP Handling
target_link_libraries(HeliotropeRabbits-MPI-Serialized PUBLIC OpenMP::OpenMP_C)

# MPI Handling
message(STATUS "Run: ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} ${MPIEXEC_MAX_NUMPROCS} ${MPIEXEC_PREFLAGS} EXECUTABLE ${MPIEXEC_POSTFLAGS} ARGS")
target_link_libraries(HeliotropeRabbits-MPI-Serialized PUBLIC MPI::MPI_C)
