/*
**  drarea.c: Computes contributing drainage areas.
*/

#include <stdio.h>

#define XSIZE 609
#define YSIZE 1765

int area[XSIZE][YSIZE];
short nbrx[XSIZE][YSIZE];
short nbry[XSIZE][YSIZE];


void GetFileName( basename )
char *basename;
{
  printf("Enter base name of flow direction files (no ext): ");
  gets( basename );
}


void ReadFlowDirFiles( basename )
char *basename;
{
  int i;
  FILE *fp;
  char outfile[80];

  /* Write x-direction file */
  strcpy( outfile, basename );
  strcat( outfile, ".nbrx" );
  if( (fp = fopen( outfile, "r" ))==NULL )
  {
    printf( "Sorry, I can't find %s\n",outfile );
    exit(1);
  }
  printf( "Reading %s...", outfile );
  fread( nbrx, sizeof( nbrx ), 1, fp );
  fclose( fp );
  printf( "done.\n" );
printf("Area at (64,944) = %d\n",nbrx[64][944]);

  /* Write y-direction file */
  strcpy( outfile, basename );
  strcat( outfile, ".nbry" );
  if( (fp = fopen( outfile, "r" ))==NULL )
  {
    printf( "Sorry, I can't find %s\n",outfile );
    exit(1);
  }
  printf( "Reading %s...", outfile );
  fread( nbry, sizeof( nbry ), 1, fp );
  fclose( fp );
  printf( "done.\n" );

}


void StreamTrace()
{
 int i, j,       /* counters for x and y coords    */
     p, q,   /* x and y coords of cells traced along stream line */
     newp,   /* used so that old value of p won't get lost   */
     test;   /* counter tests for endless loops when cells   */
             /*      point to one another.                   */

  printf("Computing contibuting areas...");
  for( i=1; i<XSIZE-1; i++ )
    for( j=1; j<YSIZE-1; j++ )
      area[i][j] = 0;

  for( i=1; i<XSIZE-2; i++ ) for( j=1; j<YSIZE-2; j++ )
  {
if( i>=64 ) {printf("\n[%d,%d] ",i,j);
printf("<nbrx: %d> ",nbrx[i][j]);
}    if( nbrx[i][j]> -1 )
    {
      p = i;
      q = j;
printf("bad ");
      test = 0;
printf("iw ");
      while( p>0 && q>0 && p<XSIZE-1 && q<YSIZE-1 )
      {
        printf("(%d,%d) ",p,q);
        area[p][q] ++;
        test++;
printf("yuk ");
        if( test > 9980 )
          printf( "-> (%d,%d) \n",p,q );
        if( test > 10000 )
        {
          printf( "There seems to be an endless loop in StreamTrace.\n" );
          exit( 0 );
        }
        newp = nbry[p][q];
        q = nbrx[p][q];
        p = newp;
      }
    }
  }

  printf("done.\n");
}



void WriteAreaFile( basename )
char *basename;
{
  int i;
  FILE *fp;
  char outfile[80];

  /* Write x-direction file */
  strcpy( outfile, basename );
  strcat( outfile, ".area" );
  fp = fopen( outfile, "w" );
  printf( "Writing %s...", outfile );
  fwrite( area, sizeof( area ), 1, fp );
  fclose( fp );
  printf( "done.\n" );
}




main()
{
  char fname[80];

  GetFileName( fname );
  ReadFlowDirFiles( fname );
  StreamTrace();
  WriteAreaFile( fname );
  printf( "All done!\n");

}
