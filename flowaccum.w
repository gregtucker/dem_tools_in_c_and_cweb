% flowaccum.w: Calculates flow accumulation from a flow direction file
% 		generated by ArcInfo.

\nocon  % omit table of contents
\datethis       % print the date on the listing

@* Introduction. This program is designed to compute flow accumulation
(drainage area) for a DEM, using a precomputed array of flow directions
in ArcInfo format. The rationale behind the program is simply that
ArcInfo's own flow accumulation function produces strange results.

@c
@<Header files to include@>@/
@<Type declarations@>@/
@<Global variables@>@/
@<Function declarations@>

void main( argc, argv )
int argc;
char **argv;
{
        @<Variables local to |main|@>@#

        @<Open input file, read flow directions and convert@>;
	@<Compute flow accumulation@>;
        @<Write output file@>;
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


@ We'll use 2 arrays: |flowdir| for the Arc-coded flow directions,
and |area| to store the drainage area (in cells) for each cell.
|NoDataValue| is the code used by ArcInfo to indicate a cell for
which data is missing or can't be computed.

@d XSIZE 1765
@d YSIZE 609

@<Global variables@>=

struct CellCoord nbr[XSIZE][YSIZE];
int area[XSIZE][YSIZE];
int NoDataValue;


@ @<Function declarations@>=


@ @<Variables local to |main|@>=

int i,j;


@ Reading the input file and converting is placed in a separate function 
so as to minimize memory usage.

@<Open input file...@>=

	ReadAndConvert( argv[1] );


@ Here's the |ReadAndConvert| function:

@c

	ReadAndConvert( filename )
	char *filename;
	{
		FILE *fp;
                int i,j,jj,nrows,ncols,tmp;
                short temparr[XSIZE][YSIZE];
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


@ The program is compiled for a grid of dimensions |XSIZE| by |YSIZE|.
If these aren't the dimensions of the data file, we've got problems.

@<Check that data...@>=

        if( ncols!=XSIZE || nrows!=YSIZE )
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

	printf( "Reading <%s>\n", filename );
        for( j=0; j<YSIZE; j++ )
                for( i=0; i<XSIZE; i++ )
                {
			jj = (YSIZE-1)-j;
			fscanf( fp, "%d", &tmp );
			switch( tmp ) {
				case 1:	nbr[i][jj].x = i+1; 
					nbr[i][jj].y = jj; 
					break;
				case 2:	nbr[i][jj].x = i+1; 
					nbr[i][jj].y = jj-1; 
					break;
				case 4:	nbr[i][jj].x = i; 
					nbr[i][jj].y = jj-1; 
					break;
				case 8:	nbr[i][jj].x = i-1; 
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
					printf("Undefined flow direction %d at %d,%d\n", tmp, i, jj );
			}
                }
        for( j=4; j>=0; j-- )
	{
                for( i=0; i<=4; i++ )
			printf("(%d,%d)    ", nbr[i][j].x, nbr[i][j].y );
		printf( "\n" );
	} 


@ This function calculates flow accumulation.

@<Compute flow...@>=

        printf( "Computing flow accumulation...\n" );
        StreamTrace();


@ |StreamTrace| computes flow accumulation by incrementing each cell's
drainage area by one (itself) and then moving from cell to cell down
the network to increment all the cells to which the current cell drains.
Think of it as a raindrop landing on each cell (incrementing its drainage
area) and then flowing downhill to increment all the others in its path.
The procedure is computationally more simple-minded (and less efficient)
than, for example, working upstream along drainage networks first.

@c

    StreamTrace()
    {
           int i, j,       /* counters for x and y coords    */
                p, q,   /* x and y coords of cells traced along stream line */
                newp,   /* used so that old value of p won't get lost   */
                test;   /* counter tests for endless loops when cells   */
                        /*      point to one another.                   */

	for( i=1; i<XSIZE-1; i++ )
	    for( j=1; j<YSIZE-1; j++ )
                area[i][j] = 0;

	for( i=1; i<XSIZE-1; i++ )
	    for( j=1; j<YSIZE-1; j++ )
            {
                p = i;
                q = j;
                area[i][j] ++;
                test = 0;
                @<Trace down the net until reaching a boundary or NoData cell@>;
            }
    }


@ Here we flow downhill from cell to cell, incrementing the drainage area
of each as we go, until reaching the edge of the grid or a depression.
(Of course there shouldn't be any @.BASIN@>s, but this way you can
calculate drainage accumulation without first calculating an outlet
for each depression. This might be useful, for example, in a karst
terrain.)

@<Trace down the net...@>=

    while( p!=0 && q !=0 && p!=XSIZE-1 && q!=YSIZE-1 )
    {
        test++;
        if( test > 9980 )
                printf( "-> (%d,%d) \n",p,q );
        if( test > 10000 )
	{
            printf( "There seems to be an endless loop in StreamTrace.\n" );
		exit( 0 );
	}
        newp = nbr[p][q].x;
        q = nbr[p][q].y;
        p = newp;
        area[p][q] ++;
    }



@ And here we write the output file.

@<Write output...@>=

        @<Parse the file name to remove anything following a period@>;
	@<Write flow accumulation to <filename>.flowacc@>;

@ @<Variables local to |main|@>+=

        char basename[80], outfile[80];
	FILE *fp;


@ @<Parse the file name...@>=

        i=0;
        while( argv[1][i]!='.' && argv[1][i]!='\0' ) {
                basename[i] = argv[1][i];
                i++;
        }

@ @<Write flow accumulation...@>=

        strcpy( outfile, basename );
        strcat( outfile, ".flowacc" );
        fp = fopen( outfile, "w" );
        printf( "...and '%s'...\n", outfile );
        fwrite( area, sizeof( area ), 1, fp );
        fclose( fp );


