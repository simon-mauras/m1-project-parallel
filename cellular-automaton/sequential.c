#include "stdio.h"
#include "stdlib.h"
#include "types.h"
#include "assert.h"

static grid_t read_sequential(int step, char* input)
{
  FILE* f = fopen(input, "r");
  
  if (f == NULL)
  {
    puts("Error: Input file doesn't exists");
    exit(1);
  }
  
  grid_t result;
  
  if (step <= 3)
  {
    double velocity;
    
    fseek(f, 1, SEEK_SET);
    assert(fread(&result.nb_columns, sizeof(size_t), 1, f));
    assert(fread(&result.nb_rows, sizeof(size_t), 1, f));
    assert(fread(&velocity, sizeof(double), 1, f));
    
    assert(result.grid = calloc(result.nb_rows, sizeof(block_t*)));
    for (size_t r=0; r<result.nb_rows; r++)
    {
      assert(result.grid[r] = calloc(result.nb_columns, sizeof(block_t)));
      for (size_t c=0; c<result.nb_columns; c++)
      {
        assert(fread(&result.grid[r][c].type, sizeof(char), 1, f));
        assert(fread(&result.grid[r][c].value, sizeof(double), 1, f));
        result.grid[r][c].velocity = velocity;
        result.grid[r][c].dvalue = 0;
      }
    }
  }
  
  fclose(f);
  
  return result;
}

void export_sequential(grid_t* grid, char* filename)
{
  FILE *f = fopen(filename, "w");
  for (size_t r=0; r<grid->nb_rows; r++)
    for (size_t c=0; c<grid->nb_columns; c++)
      assert(fwrite(&(grid->grid[r][c].value), sizeof(double), 1, f));
  fclose(f);
}

void step_sequential(grid_t* grid, double dt)
{
  double v[grid->nb_rows][grid->nb_columns];
  double dv[grid->nb_rows][grid->nb_columns];
  for (size_t r=0; r<grid->nb_rows; r++)
  {
    for (size_t c=0; c<grid->nb_columns; c++)
    {
      if (grid->grid[r][c].type == VIBRATING)
      {
        double v2 = grid->grid[r][c].velocity * grid->grid[r][c].velocity;
        v[r][c] = grid->grid[r][c].value + dt * grid->grid[r][c].dvalue;
        dv[r][c] = grid->grid[r][c].dvalue;
        dv[r][c] -= dt * v2 * 4 * grid->grid[r][c].value;
        dv[r][c] += dt * v2 * grid->grid[(r+grid->nb_rows-1)%grid->nb_rows][c].value;
        dv[r][c] += dt * v2 * grid->grid[(r+grid->nb_rows+1)%grid->nb_rows][c].value;
        dv[r][c] += dt * v2 * grid->grid[r][(c+grid->nb_columns-1)%grid->nb_columns].value;
        dv[r][c] += dt * v2 * grid->grid[r][(c+grid->nb_columns+1)%grid->nb_columns].value;
      }
    }
  }
  
  for (size_t r=0; r<grid->nb_rows; r++)
  {
    for (size_t c=0; c<grid->nb_columns; c++)
    {
      if (grid->grid[r][c].type == VIBRATING)
      {
        grid->grid[r][c].value = v[r][c];
        grid->grid[r][c].dvalue = dv[r][c];
      }
    }
  }
}

void sequential(int arg_step,
                char* arg_i,
                int arg_iteration,
                double arg_dt,
                char* arg_lastdump,
                char* arg_alldump,
                char* arg_sensor)
{
  grid_t grid = read_sequential(arg_step, arg_i);
  
  for (int i=0; i<arg_iteration; i++)
  {
    step_sequential(&grid, arg_dt);
    if (arg_alldump != NULL)
    {
      char s[256];
      sprintf(s, arg_alldump, i);
      export_sequential(&grid, s);
    }
  }
  
  if (arg_lastdump != NULL)
    export_sequential(&grid, arg_lastdump);
}
