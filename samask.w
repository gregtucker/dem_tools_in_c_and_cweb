% slopearea2.w: Generates the data for a slope/drainage area plot
%		by averaging every n data pairs.
% slopearea3.w: Modified to treat y-axis as slope, slope-area product,
%		or slope-area product raised to exponents.
% samask.w: Modified from slopearea3 to read a mask file, and only consider
%		data points whose corresponding points in the mask file are set.

\nocon
\datethis

@* Introduction. This program was written to generate the data for a
slope/drainage area analysis from a DEM. Since DEMs generally contain
a large number of pixels (data points) with a large degree of scatter,
this program is used to reduce the scatter by averaging every N consecutive
data points after the points have been arranged in order by drainage area.
The program reads three files: a file containing a grid of slope values,
a file containing a grid of drainage area values, and a file containing
a mask grid. The mask grid specifies which points are to be included in
the analysis; only those slope and area values whose corresponding entry
in the mask grid is |TRUE| are considered.

This file has been temporarily modified to read all values (no averaging,
that is n pts to avg equals 1) with up to 171500 data points.


@c
@<Header files to include@>@/
@<Type declarations@>@/
@<Global variables@>@/
@<Comparison function for qsort@>@/

void main( argc, argv )
int argc;
char **argv;
{
    @<Variables local to |main|@>@#

	@<Open file and read slope data@>;
	@<Open file and read area data@>;
	@<Open and read mask file@>;
	@<Query user for type of data to include for ordinate array@>;
	@<Create list of valid data pairs@>;
	@<Sort valid data pairs by drainage area@>;
	@<Average for every n pairs of valid data points@>;
	@<Write the output@>;
        printf( "Done.\n" );
}


@ We'll need |stdio| to do file I/O and |stdlib| for the |qsort| function.
The |math| file is needed for the |pow| function, used when an exponential
slope-area product plot is desired.

@<Header files...@>=

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


@ The structure |DataPair| holds a slope value and an area value.

@<Type declarations@>=

typedef struct {
	float sl;
	long ar;
} DataPair;


@ @<Global variables@>=


@ Variables used in |main|. The |s| and |a| arrays contain the raw slope and
drainage area data read from files.
The |mask| array is a boolean array whose values are either TRUE (1) or
FALSE (0). This array indicates whether or not the corresponding points
in the |s| and |a| arrays are to be considered in the analysis. 
|ordinateType| contains the user's option for type of data to store in
y-axis ("slope") array.
The variables |areaexp| and |slopeexp| are only used when the y-axis
is some power of area and slope. 

@d NColumns	1765
@d NRows	609

@<Variables local to |main|@>=

FILE *fp;
int i,j,ctr;
float s[NColumns][NRows];
long a[NColumns][NRows];
char mask[NColumns][NRows];
int ordinateType;
float areaexp, slopeexp;




Here we assume the slope file is a binary 4-byte float file.

@<Open file and read slope...@>=

	if( argc < 3 ) {
		printf("Usage: samask <slopefile> <areafile> {maskfile}\n");
		exit( 0 );
	}
	if( (fp=fopen( argv[1], "r" ))==NULL ) {
		printf("Unable to find '%s'\n", argv[1] );
		exit( 0 );
	}
	printf( "Reading <%s>...\n", argv[1] );
	fread( s, sizeof( s ), 1, fp ); 
	fclose( fp );


@ Read area data. 
The area file should be a binary 4-byte-int file.

@<Open file and read area...@>=

	if( (fp=fopen( argv[2], "r" ))==NULL ) {
		printf("Unable to find '%s'\n", argv[2] );
		exit( 0 );
	}
	printf( "Reading <%s>...\n", argv[2] );
	fread( a, sizeof( a ), 1, fp ); 
	fclose( fp );


@ Read the mask file. 
This file should be a binary 1-byte (char) file.
If no mask file is specified, all points in the |mask| array are
considered valid (i.e., they are set to |TRUE|).

@<Open and read mask file@>=

	if( argc < 4 ) {
		printf( "Since you didn't specify a mask file, I'm assuming all points are valid.\n" );
		for( i=0; i<NColumns; i++ )
			for( j=0; j<NRows; j++ )
				mask[i][j] = TRUE;
	}
	else {
        if( (fp=fopen( argv[3], "r" ))==NULL ) {
                printf("Unable to find mask file '%s'\n", argv[3] );
                exit( 0 );
        }
		printf( "Reading <%s>...\n", argv[3] );
		fread( mask, sizeof( mask ), 1, fp ); 
		fclose( fp );
	}


@ @<Query user...@>=

printf( "Y-axis will be: slope (0), slope-area (1), or A^m S^n (2)? " );
scanf( "%d", &ordinateType );
if( ordinateType==2 ) {
	printf( "Area exponent: " );
	scanf( "%f", &areaexp );
	printf( "Slope exponent: " );
	scanf( "%f", &slopeexp );
}


@ Valid data points are kept in the |data| list, which will be sorted.
Here we also introduce the arrays |slp| and |area| which will hold the
averaged data. Since we're averaging over |NPtsToAverage|, the dimension 
of |slp| and |area| is |NAvgPts|=|MaxValidDataPoints|/|NPtsToAverage|.

@d MaxValidDataPoints	171500	
@d NPtsToAverage	1
@d NAvgPts	171500

@<Variables local to |main|@>+=

DataPair data[MaxValidDataPoints];
float slp[NAvgPts], area[NAvgPts];
int nvalidpts;


@ We sweep through the |s| (slope) array looking for valid data points.
If the data point is invalid, it will be coded |FALSE| in the |mask| array. 
Otherwise,
it's good and we place it on the list of |DataPairs|.
After we've gone through all the data, it's likely that there will
be some empty space at the end of the |data| array. 
To ensure that this doesn't get confused with data, we assign each
area a value of -1 for all the entries beyond |nvalidpts|. This
means that they'll be at the top of the list when we sort, so that
we can easily skip past them.

@d SlopeOrdinate 0
@d StreamPowerOrdinate 1

@<Create list...@>=

printf( "Computing averages...\n" );
nvalidpts = 0;
for( i=0; i<NColumns; i++ )
	for( j=0; j<NRows; j++ )
		if( mask[i][j] ) 
		{
			
			if( ordinateType==SlopeOrdinate )
				data[nvalidpts].sl = s[i][j];
			else if( ordinateType==StreamPowerOrdinate )
				data[nvalidpts].sl = s[i][j]*a[i][j];
			else
				data[nvalidpts].sl = pow(a[i][j],areaexp)*pow(0.01*s[i][j],slopeexp); 
			data[nvalidpts].ar = a[i][j];
			nvalidpts++;
			if( nvalidpts > MaxValidDataPoints )
			{
				printf( "Sorry, there are too many valid data points for the available array size.\n" );
				exit( 0 );
			}
		}
printf( "There are %d total valid slope/area pairs.\n", nvalidpts );
for( i=nvalidpts; i<MaxValidDataPoints; i++ )
	data[i].ar = -1;


@ Since I'm lazy, we'll just use the UNIX qsort routine to sort the data pairs
by drainage area. Next we'll divide up the list and average every consecutive
|NPtsToAverage| data pairs.

@<Sort valid data...@>=
qsort( data, MaxValidDataPoints, sizeof( data[0] ), CompareAreas );


@ The |CompareAreas| function is called by |qsort|. It returns a value
of 1, -1, or 0 depending on whether the area of record |a| is greater,
less than or equal to that of record |b|.

@<Comparison function...@>=

int CompareAreas( a, b )
DataPair *a, *b;
{
	if( a->ar > b->ar ) return( 1 );
	else if( a->ar < b->ar ) return( -1 );
	else return( 0 );
}


@ These are the variables used in the averaging algorithm, described below.
|avgindex| and |dataindex| are the array index values for the averaged
arrays and the sorted list of ``raw'' data, respectively. |datactr| keeps
track of how many values we've summed up within each interval.

@<Variables local to |main|@>+=

int avgindex, datactr, dataindex; 


@ Here's where we do the averaging, and the algorithm is a bit tricky.
First, we sum all the slope and area values in groups of |NPtsToAverage|. 
The variable |datactr| keeps track of how many values we've stuffed
into one interval. The loop is finished when we've processed all
the valid data points. 
Note that since all the unused entries of |data| are at the beginning
of the array, we skip past them by setting the |dataindex| value
to a value equal to the number of unused entries, which is also the
difference between the array size (|MaxValidDataPoints|) and the
actual number of valid points |nvalidpts|.
Next we need to calculate the average value by
dividing by |NPtsToAverage|...except that the very last group will in
general have fewer accumulated values than this. 
Fortunately, the number of values in this
last interval will be equal to |datactr| at the end of
the loop. We also want to know just how many entries in the |slp| and
|area| arrays we've actually filled. 
At the end of the loop, |avgindex| contains the index of the last entry
in the averaged arrays (|slp| and |area|) that actually contain data.
This is the one that should be divided by
|datactr| rather than by |NPtsToAverage|. Since it might be the case
sometimes that the number of values in this last entry IS exactly
equal to |NPtsToAverage| (just like the others), we have to check the
value of |datactr| to avoid a division by zero. 

While we're at it we'll also convert drainage area from pixels to
square kilometers, and convert from percent rise to slope by
dividing by 100. 
Cell size is assumed to be 30m (if this was commercial
software this value wouldn't be hardcoded---but if I was writing
commercial software I'd be a lot richer, too...)  

@d ExponentialOrdinate 2
@d debug printf

@<Average for every...@>=

avgindex = 0;
datactr = 0;
dataindex = MaxValidDataPoints - nvalidpts;
do {
	slp[avgindex] += data[dataindex].sl;
	area[avgindex] += data[dataindex].ar;
	datactr++;
	if( datactr==NPtsToAverage ) {
		datactr = 0;
		avgindex++;
	}
	dataindex++;
} while( dataindex < MaxValidDataPoints );
printf( "There are %d averaged data points.\n", avgindex+1 );
if( datactr==0 ) datactr = NPtsToAverage;
for( i=0; i<avgindex; i++ )
{
	slp[i] = slp[i]/(float)NPtsToAverage;
	if( ordinateType!=ExponentialOrdinate ) slp[i] *= 0.01;
	area[i] = 0.0009*area[i]/(float)NPtsToAverage;
}
if( ordinateType!=ExponentialOrdinate ) slp[avgindex] *= 0.01;
slp[avgindex] = slp[avgindex]/(float)datactr;
area[avgindex] = 0.0009*area[avgindex]/(float)datactr;



@ @<Write the output@>=

fp = fopen( "slope.dat", "w" );
fwrite( slp, sizeof( slp ), 1, fp );
fclose( fp );
fp = fopen( "area.dat", "w" );
fwrite( area, sizeof( area ), 1, fp );
fclose( fp );



