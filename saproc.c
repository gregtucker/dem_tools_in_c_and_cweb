/* saproc.c: Program to process slope-area data. Modified from samask.w,
             April 1997.
Introduction. This program was written to generate the data for a
slope/drainage area analysis from a DEM. Since DEMs generally contain
a large number of pixels (data points) with a large degree of scatter,
this program is used to reduce the scatter by averaging every N consecutive
data points after the points have been arranged in order by drainage area.
The program reads three files: a file containing a grid of slope values,
a file containing a grid of drainage area values, and a file containing
a mask grid. The mask grid specifies which points are to be included in
the analysis; only those slope and area values whose corresponding entry
in the mask grid is |TRUE| are considered.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NColumns 1765
#define NRows 609
#define MaxValidDataPoints	171500	
#define NPtsToAverage	1
#define NAvgPts	171500

typedef struct {
	float sl;
	long ar;
} DataPair;


void main( argc, argv )
int argc;
char **argv;
{
  FILE *fp;
  int i,j,ctr;
  float s[NColumns][NRows];
  long a[NColumns][NRows];
  char mask[NColumns][NRows];
  int ordinateType;
  float areaexp, slopeexp;
  /* Valid data points are kept in the |data| list, which will be sorted.
     Here we also introduce the arrays |slp| and |area| which will hold the
     averaged data. Since we're averaging over |NPtsToAverage|, the dimension 
     of |slp| and |area| is |NAvgPts|=|MaxValidDataPoints|/|NPtsToAverage|. */
  DataPair data[MaxValidDataPoints];
  float slp[NAvgPts], area[NAvgPts];
  int nvalidpts;

  /* Here we assume the slope file is a binary 4-byte float file. */
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

	/* Read area data. 
           The area file should be a binary 4-byte-int file. */
	if( (fp=fopen( argv[2], "r" ))==NULL ) {
		printf("Unable to find '%s'\n", argv[2] );
		exit( 0 );
	}
	printf( "Reading <%s>...\n", argv[2] );
	fread( a, sizeof( a ), 1, fp ); 
	fclose( fp );

	/* Read the mask file. 
           This file should be a binary 1-byte (char) file.
           If no mask file is specified, all points in the |mask| array are
           considered valid (i.e., they are set to |TRUE|). */
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

    printf( "Y-axis will be: slope (0), slope-area (1), or A^m S^n (2)? " );
    scanf( "%d", &ordinateType );
    if( ordinateType==2 ) {
	printf( "Area exponent: " );
	scanf( "%f", &areaexp );
	printf( "Slope exponent: " );
	scanf( "%f", &slopeexp );
    }

    /* We sweep through the |s| (slope) array looking for valid data points.
     If the data point is invalid, it will be coded FALSE in the |mask| array
     Otherwise,
     it's good and we place it on the list of |DataPairs|.
     After we've gone through all the data, it's likely that there will
     be some empty space at the end of the |data| array. 
     To ensure that this doesn't get confused with data, we assign each
     area a value of -1 for all the entries beyond |nvalidpts|. This
     means that they'll be at the top of the list when we sort, so that
     we can easily skip past them. */

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


     printf( "Done.\n" );
}
