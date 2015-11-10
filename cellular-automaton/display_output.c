#include "stdio.h"

int main(int argc, char** argv)
{
  if (argc < 4)
    return 0;
  
  int nb_rows = -1, nb_columns = -1;
  sscanf(argv[3], "%d", &nb_columns);
  sscanf(argv[4], "%d", &nb_rows);
  
  if (nb_rows == -1) return 0;
  if (nb_columns == -1) return 0;
    
  FILE* in = fopen(argv[1], "r");
  FILE* out = fopen(argv[2], "w");
  
  if (in == NULL) return 0;
  if (out == NULL) return 0;
  
  fprintf(out, "P2\n");
  fprintf(out, "%d %d\n", nb_rows, nb_columns);
  fprintf(out, "65535\n");
  
  for (int r=0; r<nb_rows; r++)
  {
    for (int c=0; c<nb_columns; c++)
    {
      double value;
      fread(&value, sizeof(double), 1, in);
      if (value > 1) value = 1;
      if (value < -1) value = -1;
      int v = (1+value)/2  * 65535;
      fprintf(out, "%d%c", v, v == nb_columns-1 ? '\n' : ' ');
    }
  }
  
  fclose(in);
  fclose(out);
}
