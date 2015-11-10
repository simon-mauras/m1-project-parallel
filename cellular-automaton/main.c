#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "sequential.h"

static int arg_step;
static char* arg_i;
static int arg_iteration;
static double arg_dt;
static int arg_grid_x;
static int arg_grid_y;
static char* arg_lastdump;
static char* arg_alldump;
static char* arg_sensor;

void args(int argc, char** argv)
{
  arg_step = -1;
  arg_i = NULL;
  arg_iteration = -1;
  arg_dt = -1.;
  arg_grid_x = -1;
  arg_grid_y = -1;
  arg_lastdump = NULL;
  arg_alldump = NULL;
  arg_sensor = NULL;
  
  int error = 0;
  for (int i=1; i<argc; )
  {
    if (strcmp("-step", argv[i]) == 0)
    {
      if (i+2 > argc)
      {
        puts("Error: -step <number>");
        error = 1;
      }
      else
      {
        sscanf(argv[i+1], "%d", &arg_step);
      }
      i += 2;
    }
    else if (strcmp("-i", argv[i]) == 0)
    {
      if (i+2 > argc)
      {
        puts("Error: -i <input file>");
        error = 1;
      }
      else
      {
        arg_i = argv[i+1];
      }
      i += 2;
    }
    else if (strcmp("-iteration", argv[i]) == 0)
    {
      if (i+2 > argc)
      {
        puts("Error: -iteration <number>");
        error = 1;
      }
      else
      {
        sscanf(argv[i+1], "%d", &arg_iteration);
      }
      i += 2;
    }
    else if (strcmp("-dt", argv[i]) == 0)
    {
      if (i+2 > argc)
      {
        puts("Error: -dt <number>");
        error = 1;
      }
      else
      {
        sscanf(argv[i+1], "%lf", &arg_dt);
      }
      i += 2;
    }
    else if (strcmp("-grid", argv[i]) == 0)
    {
      if (i+3 > argc)
      {
        puts("Error: -grid <x> <y>");
        error = 1;
      }
      else
      {
        sscanf(argv[i+1], "%d", &arg_grid_x);
        sscanf(argv[i+2], "%d", &arg_grid_y);
      }
      i += 3;
    }
    else if (strcmp("-lastdump", argv[i]) == 0)
    {
      if (i+2 > argc)
      {
        puts("Error: -lastdump <output path>");
        error = 1;
      }
      else
      {
        arg_lastdump = argv[i+1];
      }
      i += 2;
    }
    else if (strcmp("-alldump", argv[i]) == 0)
    {
      if (i+2 > argc)
      {
        puts("Error: -alldump <output path>");
        error = 1;
      }
      else
      {
        arg_alldump = argv[i+1];
      }
      i += 2;
    }
    else if (strcmp("-sensor", argv[i]) == 0)
    {
      if (i+2 > argc)
      {
        puts("Error: -sensor <output_path>");
        error = 1;
      }
      else
      {
        arg_sensor = argv[i+1];
      }
      i += 2;
    }
    else
    {
      puts("Error: Unrecognized argument");
      error = 1;
      i++;
    }
  }
  
  if (arg_step < 0)
  {
    puts("Error: argument -step is required");
    error = 1;
  }
  
  if (arg_i == NULL)
  {
    puts("Error: argument -i is required");
    error = 1;
  }
  
  if (arg_iteration < 0)
  {
    puts("Error: argument -iteration is required");
    error = 1;
  }
  
  if (arg_dt < 0)
  {
    puts("Error: argument -dt is required");
    error = 1;
  }
  
  if (arg_grid_x < 0 || arg_grid_y < 0)
  {
    puts("Error: argument -grid is required");
    error = 1;
  }
  
  if (error)
    exit(1);
}

int main(int argc, char** argv)
{
  args(argc, argv);
  
  if (arg_step == 0)
  {
    sequential(arg_i,
               arg_iteration,
               arg_dt,
               arg_lastdump,
               arg_alldump,
               arg_sensor);
  }
}
