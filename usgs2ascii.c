/* usgs2ascii: attempt to convert a USGS DEM file to plain ascii */

#include <stdio.h>

main()
{
  FILE *fp, *fpout;
  char header[1170];
  char achar;
  char achunk[10];
  int i;

  fp = fopen("mariposae.dem","r");
  fpout = fopen("mariposa.ascii","w");
  fprintf(fpout,"north:                   4342700.00\n");
  fprintf(fpout,"south:                   4334280.00\n");
  fprintf(fpout,"east:                     690700.00\n");
  fprintf(fpout,"west:                     685420.00\n");
  fprintf(fpout,"rows:                       1201\n");
  fprintf(fpout,"cols:                       1201\n");
  fread( header, sizeof( header ), 1, fp );
  for( i=1; i<=983947; i++ ) {
    fread( achunk, sizeof( achunk ), 1, fp );
    fwrite( achunk, sizeof( achunk ), 1, fpout );
  } 
  fclose(fp);

}
