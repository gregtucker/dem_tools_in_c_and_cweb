/*
** strmlength: estimates the streamwise length of the "main stream" for
**             each subbasin in a DEM. "Main stream" is defined as the path
**             of maximum drainage area, working upstream. If two or more
**             upstream directions have the same drainage area, one is
**             selected at random. Length is measured as the sum of the
**             distance between pixel centers along the stream path. 
**
**    Written by Greg Tucker, October 1996.
*/

#include <stdio.h>
#include <math.h>

/* These values should be set to the dimensions of the data set */
#define NColumns 718 
#define NRows 1395 


/* Variable type definitions (just one here) */

struct CellCoord        /* Structure stores the coords of a cell */
{
        int x;
        int y;
};


/* Global variables */

struct CellCoord nbr[NColumns][NRows];
int NoDataValue;
float strmlen[NColumns][NRows];
long a[NColumns][NRows];
int gE,gSE,gS,gSW,gW,gNW,gN,gNE;



void GetFileName( basename )
char *basename;
{
  printf("Enter base name of flow direction files (no ext): ");
  gets( basename );
}


void ReadAreaFile( filenm, format )
char *filenm;
{
  FILE *fp;
  int i,j,jj,itmp1,itmp2;
  float ftmp1,ftmp2;

  if( (fp=fopen( filenm, "r" ))==NULL ) {
          printf("Unable to find '%s'\n", filenm );
          exit( 0 );
  }
  printf( "Reading <%s>...\n", filenm );

  if( format=='a' )
  {
    fread( a, sizeof( a ), 1, fp );
    fclose( fp );
    printf("done.\n");
  }
  else if( format=='t' )
  {
    fscanf( fp, "%d %d %f %f",&itmp1,&itmp2,&ftmp1,&ftmp2 );
    for( j=0; j<NRows; j++ )
      for( i=0; i<NColumns; i++ )
      {
        jj = (NRows-1)-j;
        fscanf( fp, "%d ", &a[i][jj] );
      }
  }

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


double followmainchan( x,y,totlen,reclvl )
int x,y;
float totlen;
int reclvl;
{
  int i,j;
  int npossdir=0;     /* No. of possible upstream routes */
  long amax= -1;      /* Max. area found so far */
  int mnx[8],mny[8];  /* Location(s) of max upstream area */
  int thedir;

  /* Check recursion level to avoid endless loops */
  reclvl=reclvl+1;
  if( reclvl>NColumns*NRows)
  {
    printf("Recursion level %d too high!\n",reclvl);
    exit(1);
  }

  /* For each adjacent cell, see if it drains here (x,y). If so,
     compare its area with the highest found so far and remember
     the max upstream area and location in amax, mnx(1), and mny(1).
     If we find another with the same value as amax, record multiple
     possible upstream directions by incrementing npossdir and
     recording the upstream locations in mnx() and mny(). */
  for( i=x-1; i<=x+1; i++ ) for( j=y-1; j<=y+1; j++ )
    if( nbr[i][j].x>0 )
    {
      /* Does (i,j) drain here? */
      if( nbr[i][j].x==x && nbr[i][j].y==y )
      {
        if( a[i][j]>amax )
        {
          npossdir=1;
          amax=a[i][j];
          mnx[0]=i;
          mny[0]=j;
        }
        else if( a[i][j]==amax )
        {
          npossdir++;
          mnx[npossdir-1]=i;
          mny[npossdir-1]=j;
        }
      }
    }

  /* If there is at least one upstream neighbor, increment total stream length
     and call ourself again with the upstream coords. */
  if( amax>0 )
  {
    if( npossdir>1 )  /* If multiple possible directions, choose 1 at random */
    {
      thedir = (int)(npossdir*(rand()/32767.0));
      if( thedir>=npossdir ) thedir=npossdir-1;  /* Will happen very rarely! */
      mnx[0]=mnx[thedir];
      mny[0]=mny[thedir];
    }

    /* Increment the total stream length according to whether orientation
       is straight or diagonal. */
    if( mnx[0]==x || mny[0]==y )
      totlen = totlen+1;
    else
      totlen = totlen+1.4142136;

    /* Now call ourself with the upstream coords */
    totlen = followmainchan(mnx[0],mny[0],totlen,reclvl);
  }

  return (double)totlen;

}


void main( argc, argv )
int argc;
char **argv;
{
  int i,j;
  FILE *fp;

  /* Check that input files have been specified */
  if( argc < 4 ) {
    printf( "USAGE: %s <flow dir file> <area file> <encoding scheme>\n", argv[0] );
    exit( 0 );
  }

  /* Set coding scheme */
  if( argv[3][0]!='a' && argv[3][0]!='t' )
  {
     printf("I don't know what '%c' is. The encoding scheme must be either a (Arc/Info) or t (Tarboton)\n",argv[3][0]);
     exit(1);
  }
  switch( argv[3][0] ) {
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
  ReadFlowDirFile( argv[1], argv[3][0] );

  /* Read drainage areas */
  ReadAreaFile( argv[2], argv[3][0] );

  /* For each cell, find the maximum basin length */
  for( i=1; i<NColumns-1; i++ )
  {
    printf("Col %d\n",i);
    for( j=1; j<NRows-1; j++ )
      if( nbr[i][j].x != NoDataValue ) 
      {
        strmlen[i][j] = (float)followmainchan(i,j,0.0,0);
      }
  }

  /* Write the data in binary format */
  printf("Writing 'strmlen.dat'...");
  fp = fopen("strmlen.dat","w");
  fwrite( strmlen, sizeof( strmlen ), 1, fp );
  fclose( fp );
  printf("all done.\n");

}

