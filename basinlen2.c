/*
** basinlength: estimates the Euclidean length of subbasins in a DEM
** basinlen2: replaces recursive method with downstream flow tracking
** method. Also adds option for variable flow direction encoding.
**
** written by Greg Tucker, Oct. 1996
*/

#include <stdio.h>
#include <math.h>

#define NColumns 718 
#define NRows 1395 


struct CellCoord        /* Structure stores the coords of a cell */
{
        int x;
        int y;
};

struct CellCoord nbr[NColumns][NRows];
int NoDataValue;
float baslen[NColumns][NRows];
int gE,gSE,gS,gSW,gW,gNW,gN,gNE;


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


void ReadFlowDirFile( filename, format )
char *filename, format;
{
  FILE *fp;
  int i,j,jj,nrows,ncols,tmp;
  short temparr[NColumns][NRows];
  char tempstr[80];
  float dx, dy;

  if( (fp=fopen( filename, "r" ))==NULL ) {
    printf("Unable to find '%s'\n", filename );
    exit( 0 );
  }

  /* Read the header: ARC/INFO ascii format, or D. Tarboton ascii format */
  if( format=='a' )
  {
    ncols = ReadHeaderLine( fp );
    nrows = ReadHeaderLine( fp );
    fgets( tempstr, 80, fp );
    fgets( tempstr, 80, fp );
    fgets( tempstr, 80, fp );
    NoDataValue = ReadHeaderLine( fp );
  }
  else if( format=='t' )
  {
    fscanf( fp, "%d %d %f %f",&ncols,&nrows,&dx,&dy);
    NoDataValue = -1;
  }
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
      if( tmp==gE ) {
        nbr[i][jj].x = i+1;
        nbr[i][jj].y = jj;
      } 
      else if( tmp==gSE ) {
        nbr[i][jj].x = i+1;
        nbr[i][jj].y = jj-1;
      }
      else if( tmp==gS ) {
        nbr[i][jj].x = i;
        nbr[i][jj].y = jj-1;
      }
      else if( tmp==gSW ) {
        nbr[i][jj].x = i-1;
        nbr[i][jj].y = jj-1;
      }
      else if( tmp==gW ) {
        nbr[i][jj].x = i-1;
        nbr[i][jj].y = jj;
      }
      else if( tmp==gNW ) {
        nbr[i][jj].x = i-1;
        nbr[i][jj].y = jj+1;
      }
      else if( tmp==gN ) {
        nbr[i][jj].x = i;
        nbr[i][jj].y = jj+1;
      }
      else if( tmp==gNE ) {
        nbr[i][jj].x = i+1;
        nbr[i][jj].y = jj+1;
      }
      else if( tmp==NoDataValue ) nbr[i][jj].x = NoDataValue;
      else printf("Undefined flow direction %d at %d,%d\n", tmp, i, jj );
    }

  fclose( fp );

  printf("done.\n");
}


void main( argc, argv )
int argc;
char **argv;
{
  int i,j;
  FILE *fp;
  float localdist;
  int p,q,newp,test;

  /* Check that input files have been specified */
  if( argc < 3 ) {
    printf( "USAGE: %s <flow dir file> <encoding scheme>\n",argv[0] );
    exit( 0 );
  }

  /* Set coding scheme */
  if( argv[2][0]!='a' && argv[2][0]!='t' )
  {
     printf("Encoding scheme must be either a (Arc/Info) or t (Tarboton)\n");
     exit(1);
  }
  switch( argv[2][0] ) {
    case 'a':
      gE = 1;
      gSE = 2;
      gS = 4;
      gSW = 8;
      gW = 16;
      gNW = 32;
      gN = 64;
      gNE = 128;
      break;
    case 't':
      gE = 1;
      gSE = 8;
      gS = 7;
      gSW = 6;
      gW = 5;
      gNW = 4;
      gN = 3;
      gNE = 2;
      break;
    default:
      printf("I don't know what format '%s' is.\nPossible formats are: a - Arc/Info ascii, t - Tarboton ascii\n",
              argv[2][0] );
      exit(2);
      break;
  }

  /* Read flow directions file and convert to "neighbor cell" format */
  ReadFlowDirFile( argv[1], argv[2][0] );

  /* For each cell, find the maximum basin length */
  printf( "Measuring sub-basin lengths...\n");
  for( i=1; i<NColumns-1; i++ )
  {
    printf("Col %d\n",i);
    for( j=1; j<NRows-1; j++ )
      if( nbr[i][j].x != NoDataValue ) 
      {
        p = i;
        q = j;
        test = 0;
        while( nbr[p][q].x!=NoDataValue && test<10000)
        {
          test++;
          localdist = sqrt( (double)(abs(i-p)*abs(i-p) +
                    abs(j-q)*abs(j-q)) ) + 1;
          if( localdist > baslen[p][q] ) baslen[p][q] = localdist;
          newp = nbr[p][q].x;
          q = nbr[p][q].y;
          p = newp;
        }
        if( test==10000 ) {
          printf("There appears to be a loop in the flow direction data.\n");
          exit(0);
        }

      }
  }
  printf( "done.\n");

  /* Write the data in binary format */
  printf("Writing 'baslen.dat'...");
  fp = fopen("baslen.dat","w");
  fwrite( baslen, sizeof( baslen ), 1, fp );
  fclose( fp );
  printf("all done.\n");

}

