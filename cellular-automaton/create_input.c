#include "stdio.h"
#include "stdlib.h"
#include "types.h"

int main(int argc, char ** argv)
{
  puts("----- Creation of an input file -----");
  
  FILE* f;
  if (argc < 2 || (f = fopen(argv[1], "w")) == NULL)
  {
    puts("Error: Can't create file");
    exit(1);
  }
  
  char type;
  puts("Type of the input file ?");
  scanf("%c", &type);
  if (type != '1' && type != '2')
  {
    puts("Error: Wrong type");
    exit(1);
  }
  else
  {
    type = type - '0';
  }
  
  if (type == 1)
  {
    fwrite(&type, sizeof(char), 0x01, f);
    
    grid_t g;
    
    puts("Number of rows ?");
    scanf("%lu", &g.nb_rows);
    fwrite(&g.nb_rows, sizeof(size_t), 1, f);
    
    puts("Number of columns ?");
    scanf("%lu", &g.nb_columns);
    fwrite(&g.nb_columns, sizeof(size_t), 1, f);
    
    double velocity;
    puts("Velocity ?");
    scanf("%lf", &velocity);
    fwrite(&velocity, sizeof(double), 1, f);
    
    g.grid = calloc(g.nb_rows, sizeof(block_t*));
    for (size_t r=0; r<g.nb_rows; r++)
    {
      g.grid[r] = calloc(g.nb_columns, sizeof(block_t));
      for (size_t c=0; c<g.nb_columns; c++) {
        g.grid[r][c].velocity = velocity;
      }
    }
    
    puts("Blocks type ? [V]ibrating / [W]all");
    for (size_t r=0; r<g.nb_rows; r++)
    {
      char s[g.nb_columns+1];
      scanf("%s", s);
      for (size_t c=0; c<g.nb_rows; c++)
      {
        if (s[c] == 'V')
          g.grid[r][c].type = VIBRATING;
        else if (s[c] == 'W')
          g.grid[r][c].type = WALL;
        else
        {
          puts("Error: Wrong type");
          exit(1);
        }
      }
    }
    
    puts("Blocks value ?");
    for (size_t r=0; r<g.nb_rows; r++)
      for (size_t c=0; c<g.nb_rows; c++)
        scanf("%lf", &g.grid[r][c].value);
    
    for (size_t r=0; r<g.nb_rows; r++)
    {
      for (size_t c=0; c<g.nb_rows; c++)
      {
        fwrite(&g.grid[r][c].type, sizeof(char), 1, f);
        fwrite(&g.grid[r][c].value, sizeof(double), 1, f);
      }
    }
  }
  
  fclose(f);
}
