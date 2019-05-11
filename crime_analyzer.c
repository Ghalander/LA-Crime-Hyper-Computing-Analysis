#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <math.h>

// Data count is the number of entries our program should read.
#define DATA_COUNT 5
#define BUFFER 255
#define PI 3.14159265359

float distanceMeasure(float deg_lat1, float deg_long1, float deg_lat2, float deg_long2);

// 0  - areadId, 1 - city, 2 - crime, 3 - lat, 4 - long
void storeData(int index, int arrayState, char* word, int* cityId, char** cities, char** crime, float* lats, float* longs) {
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
        case 3: { // lat-long will appear like this (34.0256, -118.3248)
        
            // printf("Lat of crime is %s\n", word);
            char first[BUFFER] = "";
            char second[BUFFER] = "";
            int cut = 0;
            for (int i = 1; i < strlen(word)-1; ++i) {
                if (word[i] == ','){
                    cut = i;
                    break;
                }
                // printf("word[i] is %c:\n", word[i]);
                strcat(first, &word[i]);
            }
            // strcat(first, '\0');
            for (int i = cut+2; i < strlen(word)-1; ++i){
                // printf("word[i] is %c:\n", word[i]);
                strcat(second, &word[i]);
            }
            // strcat(second/, '\0');
            // strcpy(word, word);
            lats[index] = atof(first);
            longs[index] = atof(second);
            break;
        }
        default: printf("No proper state was found\n");
            break;
        return;
    }
}

int main( int argc, char *argv[] )
{
    int rank, size, range, startEntry, lastEntry;
    int commaCount = 0;
    char c;
    char word[BUFFER] = "";
    int* cityId = malloc(DATA_COUNT * sizeof(int));
    char** cities = malloc(DATA_COUNT * sizeof(char*));
    char** crime = malloc(DATA_COUNT * sizeof(char*));
    for (int i =0; i < DATA_COUNT; i++){
        cities[i] = malloc((BUFFER+1) * sizeof(char));
        crime[i] = malloc((BUFFER+1) * sizeof(char));
    }
    float* latitudes = malloc(DATA_COUNT * sizeof(float));
    float* longitudes = malloc(DATA_COUNT * sizeof(float));
    int avoidFirstLine = -1;
    int isQuote = -1; // some of the data include quotes with extra commas, we want to ignore those commas
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
                        isQuote = -1;
                        commaCount = 0;
                        if (strlen(word)!=0) {
                            // printf("word -> %s\n", word);
                            storeData(index, state, word, cityId, cities, crime, latitudes, longitudes);
                            strcpy(word, "");
                        }
                        index++;
                        linesRead++;
                        if (linesRead == DATA_COUNT) {
                            break;
                        }
                    }
                    else if (c == ',' && isQuote == -1){ //new comma was found save word
                        commaCount++;
                        if (strlen(word)!=0) {
                            // printf("word -> %s\n", word);
                            storeData(index, state, word, cityId, cities, crime, latitudes, longitudes);
                            strcpy(word, "");
                        }
                    }
                    else if (c == '\"'){
                        isQuote *= -1;
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
                        // state = 4;
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

// int training_entries = DATA_COUNT * .8
// size = total # of threads
// rank = this thread #

//ranks 1 through size-2 are actually reading file
//each one needs to get an equal portion of training_entries
// training_entries / (size - 2)
//start: (rank - 1) * training_entries / (size - 2)
//end: ((rank) * training_entries / (size - 2)) - 1



// This is just to check if the data matches, comment/remove this later
    for ( int i = 0 ; i < DATA_COUNT; i++ ) {
        printf("Entry number : %d\n", i);
        printf("City ids are : %d\n", cityId[i]);
        printf("City names are : %s\n", cities[i]);
        printf("The crimes are : %s\n", crime[i]);
        printf("The Lats are : %f\n", latitudes[i]);
        printf("The Longs are : %f\n\n", longitudes[i]);
    }
    MPI_Finalize();
    return 0;
}

float distanceMeasure(float deg_lat1, float deg_long1, float deg_lat2, float deg_long2) {
	//convert to radians
	double lat1 = deg_lat1 * PI / 180;
	double long1 = deg_long1 * PI / 180;
	double lat2 = deg_lat2 * PI / 180;
	double long2 = deg_long2 * PI / 180;
	double lat_diff = lat2 - lat1;
	double long_diff = long2 - long1;
	double a = sin(lat_diff/2)*sin(lat_diff/2) + cos(lat1)*cos(lat2)*sin(long_diff/2)*sin(long_diff/2);
	if(sqrt(a) < 1)
		return asin(sqrt(a));
	else
		return asin(1);
}

