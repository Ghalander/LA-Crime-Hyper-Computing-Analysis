/*
Project 2 : LA Crime Hyper Computing Analysis
Date: 5/11/2019
Hyper Computing Spring 2019.
Professor Doina Bein

Team Members:
Katelyn Jaing
Melissa Riddle
Hector Medina

Description:
Using K-NN to predict possible crimes with the closest latitude and longitude
Algorithm to compute distance of lat/long was a version of Harvesine that worked for our use case.
CSV file comes from https://data.lacity.org/A-Safe-City/Crime-Data-from-2010-to-Present/y8tr-7khq

*/

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <math.h>

// Data count is the number of entries our program should read.
//#define DATA_COUNT 1900000
#define DATA_COUNT 20000
#define BUFFER 255
#define PI 3.14159265359

float distanceMeasure(float deg_lat1, float deg_long1, float deg_lat2, float deg_long2);

// 0  - areadId, 1 - city, 2 - crimeId, 3 - lat, 4 - long
void storeData(int index, int arrayState, char* word, int* cityId, char** cities, int* crimeId, float* lats, float* longs) {
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
            crimeId[index] = atoi(word);
            break;
        }
        case 3: { // lat-long will appear like this (34.0256, -118.3248)
            char first[BUFFER] = "";
            char second[BUFFER] = "";
            int cut = 0;
            for (int i = 1; i < strlen(word)-1; ++i) {
                if (word[i] == ','){
                    cut = i;
                    break;
                }
                strcat(first, &word[i]);
            }
            for (int i = cut+2; i < strlen(word)-1; ++i){
                strcat(second, &word[i]);
            }
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
	int readCount, remainder, startEnd[2], start=0, end=-1;
    // Used to tokenize the csv file
    int commaCount = 0;
    char c;
    char word[BUFFER] = "";
    int avoidFirstLine = -1;
    int isQuote = -1; // some of the data include quotes with extra commas, we want to ignore those commas
    int linesRead = 0;
    int state = 0; // 0  - areadId, 1 - city, 2 - crimeId, 3 - lat, 4 - long
    int index = 0;
    
    // These are the variables to read in the data.
    // Works with large data values (but malloc is not working properly with MPI)
    // Found a function called MPI_ALLOC_MEM, did not have enough time to implement
    // could have possible solved our issue
    int* cityId = malloc(DATA_COUNT * sizeof(int));
    char** cities = malloc(DATA_COUNT * sizeof(char*));
    int* crimeId = malloc(DATA_COUNT * sizeof(int));
    for (int i =0; i < DATA_COUNT; i++){
        cities[i] = malloc((BUFFER+1) * sizeof(char));
    }
    float* latitudes = malloc(DATA_COUNT * sizeof(float));
    float* longitudes = malloc(DATA_COUNT * sizeof(float));
   
    MPI_Init(&argc, &argv);
    MPI_Comm_rank( MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &size);
	
	int eighty = DATA_COUNT * 0.8;
	readCount = eighty / (size-2);
	remainder = eighty % (size-2);
	int cityId2[ eighty ];
	char cities2[ eighty ][ BUFFER ];
	int crimeId2[ eighty ];
	float latitude2[ eighty ];
	float longitude2[ eighty ];

    // used to store values for our testing phase
    int twenty = DATA_COUNT-eighty;
    int cityIdTest[ twenty ];
    char citiesTest[ twenty ][ BUFFER ];
    int crimeIdTest[ twenty ];
    float latitudeTest[ twenty ];
    float longitudeTest[ twenty ];

	/***/
	float minDistCity[3];
	float closestCrime[ twenty ][size-2];
	float closestLat[ twenty ][size-2];
	float closestLong[ twenty ][size-2];
	float smallestDistance[ twenty ][ size-2 ];
	
    // let rank 0 process our csv file
    if(rank == 0){
        FILE *file;
        file = fopen("Crime_Data_from_2010_to_Present.csv", "r");
        // we want  city name, city name id, long/lat, and the crimeId occuring
        // going to ignore first line of the file
        if (file) {
            while ((c = getc(file)) != EOF){
                if (avoidFirstLine != -1) {
                    if (c == '\n') {
                        isQuote = -1;
                        commaCount = 0;
                        if (strlen(word)!=0) {
                            // printf("word -> %s\n", word);
                            storeData(index, state, word, cityId, cities, crimeId, latitudes, longitudes);
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
                            storeData(index, state, word, cityId, cities, crimeId, latitudes, longitudes);
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
                    else if (commaCount == 7){ // crimeId name
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

        // This loop is in charge of pulling in 80% of our data count
		for( int count = 0; count < eighty; count++ ){
			cityId2[ count ] = cityId[ count ];
			strcpy( citiesTest[ count ], cities[ count ] );
			crimeId2[ count ] = crimeId[ count ];
			latitude2[ count ] = latitudes[ count ];
			longitude2[ count ] = longitudes[ count ];
		}

        //This loop is in charge of pulling in 20% of our data count for testing.
        int startIndex = 0;
        for( int count = eighty; count < DATA_COUNT; count++ ){
			cityIdTest[ startIndex ] = cityId[ count ];
			strcpy( citiesTest[ startIndex ], cities[ count ] );
			crimeIdTest[ startIndex ] = crimeId[ count ];
			latitudeTest[ startIndex ] = latitudes[ count ];
			longitudeTest[ startIndex ] = longitudes[ count ];
            startIndex++;
		}
        startIndex = 0;

        // freeing our variables after transferring it.
        free(cityId);
        free(crimeId);
        free(cities);
        free(latitudes);
        free(longitudes);

		for( int r = 0; r < size; r++ ){
			int temp = readCount;

			if( r != 0 && r != (size-1 ) ){
				if( r <= remainder ){
					temp = readCount + 1;
				}
				else{
					temp = readCount;
				}

				if( r == 1 ){
					start = 0;
				}
				else {
					start = end + 1;
				}
				end += temp;
				startEnd[0] = start;
				startEnd[1] = end;
				//printf( "Rank: %d, temp: %d, start: %d, end: %d\n", r, temp, start, end );

				/*
					tag 0 = start/end values
					tag 1 = cityId
					tag 2 = city name
					tag 3 = crimeId
					tag 4 = latitude
					tag 5 = longitude
					//// ignore tag 6 = send all data to last rank ///
					
					tag 7 = testCrime
					tag 8 = testLat
					tag 9 = testLong

					tag 10 = send closestCrime to last rank
					tag 11 = send closestLat to last rank
					tag 12 = send closestLong to last rank
					tag 13 = send the samllest distances to last rank

					tag 14 = send original lat to last rank
					tag 15 = send original long to last rank
                    tag 16 = send original crime id to last rank
				*/
				MPI_Send( &startEnd, 2, MPI_INT, r, 0, MPI_COMM_WORLD );	
				MPI_Send( &cityId2, eighty, MPI_INT, r, 1, MPI_COMM_WORLD );	
				MPI_Send( &crimeId2, eighty, MPI_INT, r, 3, MPI_COMM_WORLD );	
				MPI_Send( &latitude2, eighty, MPI_FLOAT, r, 4, MPI_COMM_WORLD );
				MPI_Send( &longitude2, eighty, MPI_FLOAT, r, 5, MPI_COMM_WORLD );

				MPI_Send( &crimeIdTest, twenty, MPI_INT, r, 7, MPI_COMM_WORLD );
				MPI_Send( &latitudeTest, twenty, MPI_FLOAT, r, 8, MPI_COMM_WORLD );
				MPI_Send( &longitudeTest, twenty, MPI_FLOAT, r, 9, MPI_COMM_WORLD );

				MPI_Send( &latitudeTest, twenty, MPI_FLOAT, size-1, 14, MPI_COMM_WORLD );
				MPI_Send( &longitudeTest, twenty, MPI_FLOAT, size-1, 15, MPI_COMM_WORLD );
                MPI_Send( &crimeIdTest, twenty, MPI_INT, size-1, 16, MPI_COMM_WORLD );
			}
		}

    }
	else if( rank != size-1 ){
		MPI_Recv( &startEnd, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
		//printf( "Rank: %d, start: %d, end: %d\n", rank, startEnd[0], startEnd[1] );
		MPI_Recv( &cityId2, eighty, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
		MPI_Recv( &crimeId2, eighty, MPI_INT, 0, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
		MPI_Recv( &latitude2, eighty, MPI_FLOAT, 0, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
		MPI_Recv( &longitude2, eighty, MPI_FLOAT, 0, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE );

		MPI_Recv( &crimeIdTest, twenty, MPI_INT, 0, 7, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
		MPI_Recv( &latitudeTest, twenty, MPI_FLOAT, 0, 8, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
		MPI_Recv( &longitudeTest, twenty, MPI_FLOAT, 0, 9, MPI_COMM_WORLD, MPI_STATUS_IGNORE );

		float mindist;
		//initialize the first minimum distance of the test data with the training data
		for( int count = 0; count < twenty; count++ ){
			mindist = distanceMeasure( latitude2[ startEnd[0] ], longitude2[ startEnd[0] ], latitudeTest[ count ], longitudeTest[ count ] );
				for( int counter = startEnd[0]+1; counter <= startEnd[1]; counter++ ){
				 	float temp = distanceMeasure( latitude2[ counter ], longitude2[ counter ], latitudeTest[ count ], longitudeTest[ count ]  );
					if( temp < mindist ){
						mindist = temp;
						closestCrime[count][rank-1] = crimeId2[counter];
						closestLat[count][rank-1] = latitude2[ counter ];
						closestLong[count][rank-1] = longitude2[ counter ];
						smallestDistance[count][rank-1] = mindist;
					}	
			}
		}

        // Sending our possible closest values from our test data
		MPI_Send( &closestCrime, twenty*(size-2), MPI_FLOAT, size-1, 10, MPI_COMM_WORLD );
		MPI_Send( &closestLat, twenty*(size-2), MPI_FLOAT, size-1, 11, MPI_COMM_WORLD );
		MPI_Send( &closestLong, twenty*(size-2), MPI_FLOAT, size-1, 12, MPI_COMM_WORLD );
		MPI_Send( &smallestDistance, twenty*(size-2), MPI_FLOAT, size-1, 13, MPI_COMM_WORLD );
		

	}
	else if (rank == size-1){

		//get original test long/lat
		MPI_Recv( &latitudeTest, twenty, MPI_FLOAT, 0, 14, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv( &longitudeTest, twenty, MPI_FLOAT, 0, 15, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
        MPI_Recv( &crimeIdTest, twenty, MPI_INT, 0, 16, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		// printf( "first lat: %f, first long: %f\n", latitudeTest[ 0 ], longitudeTest[ 0 ],  );

		float tempCrime, tempLat, tempLong, tempDist;
		float possibleCrime[ twenty ];
		float possibleLat[ twenty ];
		float possibleLong[ twenty ];
		
			for( int counter = 1; counter < size-1; counter++ ){
                printf("OUTER counter rank = %d\n", counter);
				MPI_Recv( &closestCrime, twenty*(size-2), MPI_FLOAT, counter, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
				MPI_Recv( &closestLat, twenty*(size-2), MPI_FLOAT, counter, 11, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
				MPI_Recv( &closestLong, twenty*(size-2), MPI_FLOAT, counter, 12, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
				MPI_Recv( &smallestDistance, twenty*(size-2), MPI_FLOAT, counter, 13, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
                // making sure the lat and lngs are not zeroed out
                if ( closestLat[ 0 ][ counter-1 ] != 0.0 && closestLong[ 0 ][ counter-1 ]!= 0.0){
                    tempDist = smallestDistance[ 0 ][ counter-1 ]; 
                    tempCrime = closestCrime[ 0 ][ counter-1 ];
                    tempLat = closestLat[ 0 ][ counter-1 ];
                    tempLong = closestLong[ 0 ][ counter-1 ];
                }
                else{ // setting tempDist to the largest possible value I think is correct
                    tempDist = 9999.0;
                    tempCrime = 9999.0;
                    tempLat = 9999.0;
                    tempLong = 9999.0;
                }
                // checks the smallest distance of the results of the test data with the possible predicted crime
                for( int count = 1; count < twenty; count++ ){
                        // making sure the lat and lngs are not zeroed out
                        if( smallestDistance[ count ][ counter-1 ] < tempDist && closestLat[ count ][ counter-1 ] != 0.0 && closestLong[ count ][ counter-1 ]!= 0.0){
                            tempDist = smallestDistance[ count ][ counter-1 ]; 
                            tempCrime = closestCrime[ count ][ counter-1 ];
                            tempLat = closestLat[ count ][ counter-1 ];
                            tempLong = closestLong[ count ][ counter-1 ];
                        }
			    }
                // printing out our predictions
                printf( "Orig Lat: %f, Orig Long: %f, Orig Crim: %d, ClosestLat: %f, ClosestLong: %f, PossibleCrime: %f\n", latitudeTest[counter], longitudeTest[counter], crimeIdTest[counter], tempLat, tempLong, tempCrime );
		}
	}

    MPI_Finalize();
    return 0;
}

// Using Haversine algorithm to compute the the distance of latitude and longitude
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

