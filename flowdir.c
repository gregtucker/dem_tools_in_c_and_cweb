/*
**  flowdir.c: Computes flow directions from a DEM using D8 algorithm.
*/

#include <stdio.h>

/*#define XSIZE 609 
#define YSIZE 1765
*/#define YSIZE 609 
#define XSIZE 1765

short elev[XSIZE][YSIZE];
short nbrx[XSIZE][YSIZE];
short nbry[XSIZE][YSIZE];


void ReadElevationFile( fname )
char *fname;
{
  FILE *fp;
  int i,j;

  /* Open the file (if not successful, quit with error) */
  if( (fp=fopen(fname,"rb"))==NULL )
  {
    printf("Unable to open %s\n",fname);
    exit(1);
  }

  /* Read data in binary format, and close file */
  fread( elev, sizeof( elev ), 1, fp );
  fclose( fp );

}


void GetElevFileName( elevname )
char *elevname;
{
  printf("Enter name of elevation file: ");
  gets( elevname );
}


void FindFlowDirections()
{
  int i, j, ii, jj;
  float drop, maxdrop, root2recip = 0.70710678;  /* 1/sqrt(2) */
  int nsink=0,            /* Number of sinks in the data */
      nambig=0;           /* Number of locations w/ ambiguous flow dir'n */

  /* For each node in grid, not including edges, search neighbors and find
     steepest drop. Don't consider points with zero or lower elevation */
  printf( "Computing flow directions...");
  for( i=1; i<XSIZE-1; i++ )  
    for( j=1; j<YSIZE-1; j++ )  
      if( elev[i][j]>0 )
      {
        /* Find max drop to one of eight surrounding nodes, and store the
           neighbor coordinates in nbrx and nbry arrays */
        maxdrop = -1;
        for( ii=i-1; ii<=i+1; ii++ )
          for( jj=j-1; jj<=j+1; jj++ )
          {
            drop = elev[i][j]-elev[ii][jj];
            if( i!=ii && j!=jj ) drop *= root2recip;
            if( drop>maxdrop )
            {
              maxdrop = drop;
              nbry[i][j] = ii;
              nbrx[i][j] = jj;
            }
            else if( drop==maxdrop ) nambig++;
          }
          if( nbry[i][j]==i && nbrx[i][j]==j ) nsink++;
      } 
      else nbrx[i][j] = -1;

  printf("done.\n");
  printf("There are %d sinks in the data set.\n",nsink);
  printf("There are %d ambiguous flow directions.\n",nambig);

}


void WriteFlowDirFiles( fname )
char *fname;
{
  int i;
  FILE *fp;
  char basename[80], outfile[80];

  /* Parse the input (elevation) file name to remove anything following a . */
  i=0;
  while( fname[i]!='.' && fname[i]!='\0' ) {
    basename[i] = fname[i];
    i++;
  }
  basename[i] = '\0';

  /* Write x-direction file */
  strcpy( outfile, basename );
  strcat( outfile, ".nbrx" );
  fp = fopen( outfile, "w" );
  printf( "Writing %s...", outfile );
  fwrite( nbrx, sizeof( nbrx ), 1, fp );
  fclose( fp );
  printf( "done.\n" );

  /* Write y-direction file */
  strcpy( outfile, basename );
  strcat( outfile, ".nbry" );
  fp = fopen( outfile, "w" );
  printf( "Writing %s...", outfile );
  fwrite( nbry, sizeof( nbry ), 1, fp );
  fclose( fp );
  printf( "done.\n" );

}


main()
{
  int i,j;
  char elevname[80];

  GetElevFileName( elevname );
  ReadElevationFile( elevname );
  FindFlowDirections();
  WriteFlowDirFiles( elevname );

  printf("All done!\n");
   
}

