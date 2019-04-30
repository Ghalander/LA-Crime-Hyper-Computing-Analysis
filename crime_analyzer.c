#include <stdio.h>
#include <stdlib.h>
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

// 0  - areadId, 1 - city, 2 - crime, 3 - lat, 4 - long
void storeData(int index, int arrayState, char* word, int* cityId, char (*cities)[255], char (*crime)[255], float* lats, float* longs) {
    switch (arrayState)
    {
        case 0: {
            cityId[index] = atoi(word);
            break;
        }
        case 1: {
            strcpy(cities[index], word);
            break;
        }
        case 2: {
            strcpy(crime[index], word);
            break;
        }
        case 3: { // lat looks like this "(34.0029, copy over the word after the parenthesis
            strcpy(word, (word+2));
            lats[index] = atof(word);
            printf("new lat: %f\n", lats[index]);
            break;
        }
        case 4: { // longs can look like this -118.2565)" as well. null the word before the parenthesis
            word[strlen(word)-2] = '\0';
            longs[index] = atof(word);
            printf("new long: %f\n", longs[index]);
            break;
        }
        default: printf("No proper state was found\n");
            break;
        return;
    }
}

// Data count is the number of entries our program should read.
#define DATA_COUNT 85
int main( int argc, char *argv[] )
{
    int rank, size, range, startEntry, lastEntry;
    int commaCount = 0;
    char c;
    char word[255] = "";
    int cityId[DATA_COUNT];
    char cities[DATA_COUNT][255]; // 10 strings with max string size of 255
    float latitudes[DATA_COUNT];
    float longitudes[DATA_COUNT];
    char crime[DATA_COUNT][255];
    int avoidFirstLine = -1;
    int linesRead = 0;
    int state = 0; // 0  - areadId, 1 - city, 2 - crime, 3 - lat, 4 - long
    int index = 0;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank( MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &size);
    // MPI_File mpiFile;
    // MPI_Offset mpiOffset;

    // let rank 0 process our file
    if(rank == 0){
        FILE *file;
        file = fopen("Crime_Data_from_2010_to_Present.csv", "r");
        // we want  city name, city name id, long/lat, and the crime occuring
        // going to ignore first line of the file
        if (file) {
            while ((c = getc(file)) != EOF){
                if (avoidFirstLine != -1) {
                    if (c == '\n') {
                        commaCount = 0;
                        if (strlen(word)!=0) {
                            printf("word -> %s\n", word);
                            storeData(index, state, word, cityId, cities, crime, latitudes, longitudes);
                            strcpy(word, "");
                        }
                        index++;
                        linesRead++;
                        printf("number of lines read: %d\n", linesRead);
                        if (linesRead == DATA_COUNT) {
                            break;
                        }
                    }
                    if (c == ','){ //new comma was found save word
                        commaCount++;
                        if (strlen(word)!=0) {
                            printf("word -> %s\n", word);
                            storeData(index, state, word, cityId, cities, crime, latitudes, longitudes);
                            strcpy(word, "");
                        }
                    }
                    else if (commaCount == 4){ // city id
                        state = 0;
                        strcat(word, &c);
                    }
                    else if (commaCount == 5){ // city name
                        state = 1;
                        strcat(word, &c);
                    }
                    else if (commaCount == 8){ // crime name
                        state = 2;
                        strcat(word, &c);
                    }
                    else if (commaCount == 25){ // lat
                        state = 3;
                        strcat(word, &c);
                    }
                    else if (commaCount == 26){ // long
                        state = 4;
                        strcat(word, &c);
                    }
                }
                else if (c == '\n'){ // this is how we skip the first line
                    avoidFirstLine = 0;
                }
            }
            fclose(file);
        }
    }

// This is just to check if the data matches, comment/remove this later
    for ( int i = 0 ; i < DATA_COUNT; i++ ) {
        printf("City ids are : %d\n", cityId[i]);
        printf("City names are : %s\n", cities[i]);
        printf("The crimes are : %s\n", crime[i]);
        printf("The Lats are : %f\n", latitudes[i]);
        printf("The Longs are : %f\n\n", longitudes[i]);
    }
    MPI_Finalize();
    return 0;
}
