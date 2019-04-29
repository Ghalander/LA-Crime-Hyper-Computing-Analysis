#include <stdio.h>
#include <mpi.h>

// Don't know if we need parallelization to read files, obv incomplete
// void readFileMPI(){
    
//     // open single csv file
//     MPI_File_open(MPI_COMM_WORLD, "Crime_Data_from_2010_to_Present.csv", MPI_MODE_RDONLY, MPI_INFO_NULL, &mpiFile);
//     // get size of file -- mpiOffset gets number of bytes
//     MPI_File_get_size(mpiFile, &mpiOffset);
//     // range for each processor to read
//     range = mpiOffset / size;
//     // entry for each processor to start on
//     startEntry = range * rank;
//     // define where to stop reading
//     if (rank == size-1) {
//         lastEntry = mpiOffset;
//     }
//     else {
//         lastEntry = startEntry + range;
//     }
//     // just printing out line ranges just to check if it worked.
//     printf("Process: %d, in charge of range = %d to %d\n", rank, startEntry, lastEntry);
// }

int main( int argc, char *argv[] )
{
    int rank, size, range, startEntry, lastEntry;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank( MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &size);
    MPI_File mpiFile;
    MPI_Offset mpiOffset;

    // let rank 0 process our file
    if(rank == 0){
        int c;
        FILE *file;
        file = fopen("Crime_Data_from_2010_to_Present.csv", "r");
        if (file) {
            while ((c = getc(file)) != '\n')
                putchar(c);
            fclose(file);
        }
    }
    MPI_Finalize();
    return 0;
}
