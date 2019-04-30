# LA-Crime-Hyper-Computing-Analysis
Statistical analysis of crime data from the LA county area. Data provided dates range from April 2010 to April 2019.

Data was provided by [Los Angeles Open Data](https://data.lacity.org/A-Safe-City/Crime-Data-from-2010-to-Present/y8tr-7khq)

Members:

Hector M

Katelyn J

Melissa R

### TODO:
- When attempting to read from the file it may take a while, should use multiple processes to read from file
- It should collect the data and somehow format it. Json in C? or Arrays and carefully label information by index?
- Will be using Nearest Neighbor Classification
- Using labeled crime data with lat-long when presenting a new lat-long, it should be able ot predict a crime of the same nature.


### resources
- [MPI I/O](http://wgropp.cs.illinois.edu/courses/cs598-s16/lectures/lecture32.pdf)
1. Read in .csv file
2. Use MPI to look up different city area ids/names/long-lat/crime 
3. Predict

### TASKS:
[Hector] read in .csv file

[ ] Use MPI to look up different city area ids/names/long-lat/crime

[Katelyn] Long-Lat math to determine crime that occurred in that area (function to be called)

[ ] Predict
