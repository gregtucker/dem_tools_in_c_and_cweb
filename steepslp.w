% steepslp.w: Calculates slopes for a DEM by steepest descent method.

\nocon
\datethis

@* Introduction. This program generates a slope map from a DEM using
the steepest-descent algorithm, meaning that the slope assigned to a
pixel is equal to the elevation difference between the cell and the
adjacent cell that lies in the direction of steepest descent, divided
by the distance between the two cells. The program works by reading
two files, one of elevation and one of flow directions. Flow 
directions are assumed to be in ArcInfo format, and so are converted
to neighbor coords. 

The present version of this program is set up for a 620 by 632 array
for the Zagros DEM, and reads elevation in 4-byte floating point
format.

@c
@<Header files to include@>@/
@<Type declarations@>@/
@<Global variables@>@/

void main( argc, argv )
int argc;
char **argv;
{
        @<Variables local to |main|@>@#

	@<Make sure input files have been specified@>;
	@<Open and read elevations file@>;
	@<Open input file, read flow directions and convert@>;
	@<Calculate slope for each cell@>;
	@<Write the output@>;
        printf( "Done.\n" );
}


@ We'll need |stdio| to do file I/O.

@<Header files...@>=

#include <stdio.h>


@ Variable type |CellCoord| is used for |nbr| array.

@<Type declarations@>=

struct CellCoord        /* Structure stores the coords of a cell */
{
        int x;
        int y;
};


@ @<Global variables@>=

struct CellCoord nbr[NColumns][NRows];
int NoDataValue;

@ Variables used in |main|. 
The variables |xstart| and |ystart| are the x and y coordinates of the
starting point, given in cells with the lower left pixel as 0,0.

@d NColumns	632
@d NRows	620

@<Variables local to |main|@>=

int i, j;
FILE *fp;
float elev[NColumns][NRows];
float slope[NColumns][NRows];
int k, m; /* For debug */



@ Check to see whether the right number of files have been listed on
the command line.

@<Make sure...@>=

if( argc < 3 ) {
	printf( "USAGE: steepslp <elevation file> <flow dir file>\n" );
	exit( 0 );
}


@ We read the elevation data as a binary file.

@<Open and read elevations...@>=

        if( (fp=fopen( argv[1], "r" ))==NULL ) {
                printf("Unable to find '%s'\n", argv[1] );
                exit( 0 );
        }
	printf( "Reading <%s>...\n", argv[1] );
	fread( elev, sizeof( elev ), 1, fp );
	fclose( fp );

 
@ Reading the input file and converting is placed in a separate function
so as to minimize memory usage.

@<Open input file...@>=

        ReadAndConvert( argv[2] );


@ Here's the |ReadAndConvert| function:

@c
        ReadAndConvert( filename )
        char *filename;
        {
                FILE *fp;
                int i,j,jj,nrows,ncols,tmp;
                short temparr[NColumns][NRows];
                char tempstr[80];

                @<Open the flow directions file@>;
                @<Read the header@>;
                @<Check that data file has the correct dimensions@>;
                @<Read the data and convert to cell coordinates@>;
                fclose( fp );
        }


@ If the file can't be opened (i.e. found) exit with an error message.

@<Open the flow directions file@>=

        if( (fp=fopen( filename, "r" ))==NULL ) {
                printf("Unable to find '%s'\n", filename );
                exit( 0 );
        }


@ When ArcInfo creates an ASCII file from a grid, it writes a header
containing information about the file. Here we read the header, keeping
some bits and discarding others.

@<Read the header@>=

	ncols = ReadHeaderLine( fp );
	nrows = ReadHeaderLine( fp );
	fgets( tempstr, 80, fp );
	fgets( tempstr, 80, fp );
	fgets( tempstr, 80, fp );
	NoDataValue = ReadHeaderLine( fp );
	printf("NoDataValue is %d\n",NoDataValue);


@ The function |ReadHeaderLine| reads 14 text characters (discarding them)
followed by an integer, and finally the linefeed character after the
integer.

@c
int ReadHeaderLine( fp )
FILE *fp;
{
        int i;

        /* Read the text part and discard */
        for( i=1; i<=14; i++ )
                fgetc( fp );

        /* Read the integer */
        fscanf( fp, "%d", &i );

        /* Read the line feed character */
        fgetc( fp );

        return( i );
}


@ The program is compiled for a grid of dimensions |NColumns| by |NRows|.
If these aren't the dimensions of the data file, we've got problems.

@<Check that data...@>=

        if( ncols!=NColumns || nrows!=NRows )
        {
                printf("Program must be recompiled for %d by %d data\n",
                        ncols, nrows );
                exit( 0 );
        }


@ Here's the meat of the function: we read each flow direction one by
one, and |switch| on it to find the appropriate neighbor coords.
If the flow direction isn't a power of 2 (1, 2, ... 128), then the
flow direction is undefined (this shouldn't happen with a properly
filled DEM).

@<Read the data and convert...@>=

	printf( "Reading <%s>...\n", filename );
	for( j=0; j<NRows; j++ )
                for( i=0; i<NColumns; i++ )
                {
                        jj = (NRows-1)-j;
                        fscanf( fp, "%d", &tmp );
                        switch( tmp ) {
                                case 1: nbr[i][jj].x = i+1;
                                        nbr[i][jj].y = jj;
                                        break;
                                case 2: nbr[i][jj].x = i+1;
                                        nbr[i][jj].y = jj-1;
                                        break;
                                case 4: nbr[i][jj].x = i;
                                        nbr[i][jj].y = jj-1;
                                        break;
                                case 8: nbr[i][jj].x = i-1;
                                        nbr[i][jj].y = jj-1;
                                        break;
                                case 16: nbr[i][jj].x = i-1;
                                        nbr[i][jj].y = jj;
                                        break;
                                case 32: nbr[i][jj].x = i-1;
                                        nbr[i][jj].y = jj+1;
                                        break;
                                case 64: nbr[i][jj].x = i;
                                        nbr[i][jj].y = jj+1;
                                        break;
                                case 128: nbr[i][jj].x = i+1;
                                        nbr[i][jj].y = jj+1;
                                        break;
                                default:
					if( tmp==NoDataValue ) nbr[i][jj].x = NoDataValue;
					else printf("Undefined flow direction %d at %d,%d\n", tmp, i, jj );
                        }
                }


@ Slope is the elevation distance between a cell and its steepest neighbor
|nbr|, divided by the distance between them. In computing distance, 30 meter
cells are assumed, and channel segments within a cell are assumed to be
straight.

@<Calculate slope...@>=

printf( "Computing slopes...\n" );
for( i=0; i<NColumns; i++ )
	for( j=0; j<NRows; j++ )
	    if( elev[i][j] != NoDataValue )
		{
		slope[i][j] = elev[i][j] - elev[nbr[i][j].x][nbr[i][j].y];
		if( slope[i][j]<-100 ) {
			printf("Neg. slope at (%d,%d) flowing to (%d,%d)\n",
					i,j,nbr[i][j].x,nbr[i][j].y );
			for( k=j+1; k>=j-1; k-- ) 
			{	
				for( m=i-1; m<=i+1; m++ )
					printf( "(%d,%d) %d   ",k,m,elev[m][k] );
				printf("\n");
			}
		}
		if( i==nbr[i][j].x || j==nbr[i][j].y )
			slope[i][j] = slope[i][j] / 30.0;
		else slope[i][j] = slope[i][j] / 42.4264;
		}
	    else slope[i][j] = NoDataValue;


@ @<Write the output@>=

fp = fopen( "steepslope.dat", "w" );
fwrite( slope, sizeof( slope ), 1, fp );
fclose( fp );



