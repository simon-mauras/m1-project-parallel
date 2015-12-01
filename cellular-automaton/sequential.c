#include "stdio.h"
#include "stdlib.h"
#include "types.h"
#include "assert.h"

static void read_sequential(char* input, grid_t* result)
{
  FILE* f = fopen(input, "r");
  
  if (f == NULL)
  {
    puts("Error: Input file doesn't exists");
    exit(1);
  }
  
  char type;
  assert(fread(&type, sizeof(char), 1, f));
  
  if (type == 0x01)
  {
    double velocity;
    
    assert(fread(&(result->nb_rows), sizeof(size_t), 1, f));
    assert(fread(&(result->nb_columns), sizeof(size_t), 1, f));
    assert(fread(&velocity, sizeof(double), 1, f));
    
    assert(result->grid = calloc(result->nb_rows, sizeof(block_t*)));
    for (size_t r=0; r<result->nb_rows; r++)
    {
      assert(result->grid[r] = calloc(result->nb_columns, sizeof(block_t)));
      for (size_t c=0; c<result->nb_columns; c++)
      {
        assert(fread(&(result->grid[r][c].type), sizeof(char), 1, f));
        assert(fread(&(result->grid[r][c].value), sizeof(double), 1, f));
        result->grid[r][c].velocity = velocity;
        result->grid[r][c].dvalue = 0;
      }
    }
  }
  else if (type == 0x02)
  {
    puts("Warning: File of type 2 detected.");
    assert(fread(&(result->nb_rows), sizeof(size_t), 1, f));
    assert(fread(&(result->nb_columns), sizeof(size_t), 1, f));
    
    assert(result->grid = calloc(result->nb_rows, sizeof(block_t*)));
    for (size_t r=0; r<result->nb_rows; r++)
    {
      assert(result->grid[r] = calloc(result->nb_columns, sizeof(block_t)));
      for (size_t c=0; c<result->nb_columns; c++)
      {
        assert(fread(&(result->grid[r][c].type), sizeof(char), 1, f));
        assert(fread(&(result->grid[r][c].value), sizeof(double), 1, f));
        assert(fread(&(result->grid[r][c].velocity), sizeof(double), 1, f));
        result->grid[r][c].dvalue = 0;
      }
    }
  }
  
  fclose(f);
}

static void export_sequential(char* filename, grid_t* grid)
{
  FILE *f = fopen(filename, "w");
  
  if (f == NULL)
  {
    puts("Error: Can't create output file");
    exit(1);
  }
  
  for (size_t r=0; r<grid->nb_rows; r++)
    for (size_t c=0; c<grid->nb_columns; c++)
      assert(fwrite(&(grid->grid[r][c].value), sizeof(double), 1, f));
  fclose(f);
}

static void step_sequential(grid_t* grid, double dt)
{
  double v[grid->nb_rows][grid->nb_columns];
  double dv[grid->nb_rows][grid->nb_columns];
  
  for (size_t r=0; r<grid->nb_rows; r++)
  {
    for (size_t c=0; c<grid->nb_columns; c++)
    {
      v[r][c] = grid->grid[r][c].value;
      dv[r][c] = grid->grid[r][c].dvalue;
    }
  }
  
  for (size_t r=0; r<grid->nb_rows; r++)
  {
    for (size_t c=0; c<grid->nb_columns; c++)
    {
      if (grid->grid[r][c].type == VIBRATING)
      {
        double v2 = grid->grid[r][c].velocity * grid->grid[r][c].velocity;
        grid->grid[r][c].value = v[r][c] + dt * dv[r][c];
        grid->grid[r][c].dvalue  = dv[r][c];
        grid->grid[r][c].dvalue -= dt * v2 * 4 * v[r][c];
        grid->grid[r][c].dvalue += dt * v2 * v[(r+grid->nb_rows-1)%grid->nb_rows][c];
        grid->grid[r][c].dvalue += dt * v2 * v[(r+grid->nb_rows+1)%grid->nb_rows][c];
        grid->grid[r][c].dvalue += dt * v2 * v[r][(c+grid->nb_columns-1)%grid->nb_columns];
        grid->grid[r][c].dvalue += dt * v2 * v[r][(c+grid->nb_columns+1)%grid->nb_columns];
      }
    }
  }
}

void sequential(char* arg_i,
                int arg_iteration,
                double arg_dt,
                char* arg_lastdump,
                char* arg_alldump,
                char* arg_sensor)
{
  grid_t grid;
  read_sequential(arg_i, &grid);
  
  for (int i=0; i<arg_iteration; i++)
  {
    step_sequential(&grid, arg_dt);
    if (arg_alldump != NULL)
    {
      char s[256];
      sprintf(s, arg_alldump, i);
      export_sequential(s, &grid);
    }
  }
  
  if (arg_lastdump != NULL)
    export_sequential(arg_lastdump, &grid);
}
