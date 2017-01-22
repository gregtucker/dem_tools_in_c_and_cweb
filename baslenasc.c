/*
** basinlength: estimates the Euclidean length of subbasins in a DEM
*/

#include <stdio.h>
#include <math.h>

#define NColumns 1765
#define NRows 609


struct CellCoord        /* Structure stores the coords of a cell */
{
        int x;
        int y;
};

struct CellCoord nbr[NColumns][NRows];
int NoDataValue;
double baslen[NColumns][NRows];



void GetFileName( basename )
char *basename;
{
  printf("Enter base name of flow direction files (no ext): ");
  gets( basename );
}


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


void ReadFlowDirFile( filename )
char *filename;
{
  FILE *fp;
  int i,j,jj,nrows,ncols,tmp;
  short temparr[NColumns][NRows];
  char tempstr[80];

  if( (fp=fopen( filename, "r" ))==NULL ) {
    printf("Unable to find '%s'\n", filename );
    exit( 0 );
  }

  /* Read the header (ARC/INFO ascii format is assumed) */
  ncols = ReadHeaderLine( fp );
  nrows = ReadHeaderLine( fp );
  fgets( tempstr, 80, fp );
  fgets( tempstr, 80, fp );
  fgets( tempstr, 80, fp );
  NoDataValue = ReadHeaderLine( fp );
  printf("NoDataValue is %d\n",NoDataValue);

  /* Check that data have the right dimensions */
  if( ncols!=NColumns || nrows!=NRows )
  {
    printf("Program must be recompiled for %d by %d data\n",
            ncols, nrows );
    exit( 0 );
  }

  /* Convert from 1,2,4,... code to neighbor cell coords */
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

  fclose( fp );

  printf("done.\n");
}


double lengthtopoint( i,j,x,y,lmax,reclvl)
int i,j,x,y;
double lmax;
int reclvl;
{
  double localdist;
  int ii,jj;

  /* Check recursion level to avoid endless loops */
  reclvl=reclvl+1;
  if( reclvl>NColumns*NRows)
  {
    printf("Recursion level too high!\n");
    exit(1);
  }

  /* Compute local distance to the outlet (i,j), and record it as the max
   * if it's larger than current max */
  localdist = sqrt( (double)(abs(i-x)*abs(i-x) + 
                    abs(j-y)*abs(j-y)) ) + 1;
  if( localdist > lmax ) lmax = localdist;

  /* Check each adjacent node: if it flows here, then call ourself again
   * with the upstream neighbor coords */
  for( ii=x-1; ii<=x+1; ii++ )
    for( jj=y-1; jj<=y+1; jj++ )
      if( nbr[ii][jj].x==x & nbr[ii][jj].y==y )
        lmax=lengthtopoint(i,j,ii,jj,lmax,reclvl);

  return lmax;

}


void main( argc, argv )
int argc;
char **argv;
{
  int i,j;
  FILE *fp;

  /* Check that input files have been specified */
  if( argc < 2 ) {
    printf( "USAGE: basinlength <flow dir file>\n" );
    exit( 0 );
  }

  /* Read flow directions file and convert to "neighbor cell" format */
  ReadFlowDirFile( argv[1] );

  /* For each cell, find the maximum basin length */
  for( i=1; i<NColumns-1; i++ )
  {
    printf("Col %d\n",i);
    for( j=1; j<NRows-1; j++ )
      if( nbr[i][j].x != NoDataValue ) 
      {
        baslen[i][j] = lengthtopoint(i,j,i,j,0,0);
        printf("(%d,%d) %f\n",i,j,baslen[i][j]);
      }
  }

  /* Write the data in binary format */
  printf("Writing 'baslen.dat'...");
  fp = fopen("baslen.dat","w");
  fwrite( baslen, sizeof( baslen ), 1, fp );
  fclose( fp );
  printf("all done.\n");

}

