#include <stdio.h>
#include <mpi.h>
#include <string.h>
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
    int commaCount = 0;
    char c;
    string word = "";
    vector<string> cities;
    vector<int> cityId;
    vector<float> latitudes;
    vector<float> longitudes;
    vector<string> crime;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank( MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &size);
    MPI_File mpiFile;
    MPI_Offset mpiOffset;

    // let rank 0 process our file
    if(rank == 0){
        FILE *file;
        file = fopen("Crime_Data_from_2010_to_Present.csv", "r");
        // we want  city name, city name id, long/lat, and the crime occuring
        // going to ignore first line of the file
        if (file) {
            while ((c = getc(file)) != EOF){
                if (commaCount == 4){ // city id
                    word += c;
                }
                else if (commaCount == 5){ // city name
                    word += c;
                }
                else if (commaCount == 6){ // crime name
                    word += c;
                }
                else if (commaCount == 25){ // lat
                    word += c;
                }
                else if (commaCount == 26){ // long
                    word += c;
                }
                putchar(c);
                if (c == ','){ //new comma was found save word

                }
            }
            fclose(file);
        }
    }
    MPI_Finalize();
    return 0;
}
