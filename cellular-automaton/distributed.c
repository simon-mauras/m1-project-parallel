#include "stdio.h"
#include "stdlib.h"
#include "types.h"
#include "assert.h"

#include "mpi.h"

static MPI_Comm COMM_COL;
static MPI_Comm COMM_ROW;
static int rank_col, size_col;
static int rank_row, size_row;

static int pos(int nb, int rank, int size)
{
  int result = (nb / size) * rank;
  if (rank > nb % size)
    result += nb % size;
  else
    result += rank;
  return result;
}

void exit_distributed(int err)
{
  MPI_Finalize();
  exit(err);
}

void init_distributed(int argc, char** argv, int arg_grid_x, int arg_grid_y)
{
  MPI_Init(&argc,&argv);
  
  int size, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  if (arg_grid_x * arg_grid_y != size)
  {
    if (rank == 0)
      puts("Wrong number of processors");
    exit_distributed(1);
  }
  
	MPI_Comm_split(MPI_COMM_WORLD, rank/arg_grid_y, rank, &COMM_ROW);
	MPI_Comm_split(MPI_COMM_WORLD, rank%arg_grid_y, rank, &COMM_COL);
	
  MPI_Comm_rank(COMM_ROW, &rank_row);
  MPI_Comm_rank(COMM_COL, &rank_col);
  
  MPI_Comm_size(COMM_ROW, &size_row);
  MPI_Comm_size(COMM_COL, &size_col);
}

static void read_distributed(char* input, grid_t* result)
{
  MPI_File f;
  MPI_File_open(MPI_COMM_WORLD, input, MPI_MODE_RDONLY, MPI_INFO_NULL, &f);
  
  char type;
  MPI_File_read(f, &type, 1, MPI_CHAR, MPI_STATUS_IGNORE);
  MPI_File_read(f, &(result->nb_rows), 1, MPI_UNSIGNED_LONG, MPI_STATUS_IGNORE);
  MPI_File_read(f, &(result->nb_columns), 1, MPI_UNSIGNED_LONG, MPI_STATUS_IGNORE);
  result->nb_records = 0;
  
  size_t rows = pos(result->nb_rows, rank_col+1, size_col) - pos(result->nb_rows, rank_col, size_col);
  size_t cols = pos(result->nb_columns, rank_row+1, size_row) - pos(result->nb_columns, rank_row, size_row);
  
  assert(result->grid = calloc(rows, sizeof(block_t*)));
  for (size_t r=0; r<rows; r++)
    assert(result->grid[r] = calloc(cols, sizeof(block_t)));
  
  if (type == 0x01)
  {
    double velocity;
    MPI_File_read(f, &velocity, 1, MPI_DOUBLE, MPI_STATUS_IGNORE);
    
    size_t inf = pos(result->nb_columns, rank_row, size_row) * (sizeof(char) + sizeof(double));
    size_t sup = pos(result->nb_columns, rank_row+1, size_row) * (sizeof(char) + sizeof(double));
    size_t end = pos(result->nb_columns, size_row, size_row) * (sizeof(char) + sizeof(double));
    
    size_t offset_row = pos(result->nb_rows, rank_col, size_col) * result->nb_columns * (sizeof(char) + sizeof(double));
    MPI_File_seek(f, offset_row, MPI_SEEK_CUR);
    for (size_t r=0; r<rows; r++)
    {
      MPI_File_seek(f, inf, MPI_SEEK_CUR);
      for (size_t c=0; c<cols; c++)
      {
        MPI_File_read(f, &(result->grid[r][c].type), 1, MPI_CHAR, MPI_STATUS_IGNORE);
        MPI_File_read(f, &(result->grid[r][c].value), 1, MPI_DOUBLE, MPI_STATUS_IGNORE);
        result->grid[r][c].dvalue = 0;
        result->grid[r][c].velocity = velocity;
        if (result->grid[r][c].type == SENSOR)
          result->nb_records++;
      }
      MPI_File_seek(f, end - sup, MPI_SEEK_CUR);
    }
  }
  else if (type == 0x02)
  {
    size_t inf = pos(result->nb_columns, rank_row, size_row) * (sizeof(char) + 2*sizeof(double));
    size_t sup = pos(result->nb_columns, rank_row+1, size_row) * (sizeof(char) + 2*sizeof(double));
    size_t end = pos(result->nb_columns, size_row, size_row) * (sizeof(char) + 2*sizeof(double));
    
    size_t offset_row = pos(result->nb_rows, rank_col, size_col) * result->nb_columns * (sizeof(char) + 2*sizeof(double));
    MPI_File_seek(f, offset_row, MPI_SEEK_CUR);
    for (size_t r=0; r<rows; r++)
    {
      MPI_File_seek(f, inf, MPI_SEEK_CUR);
      for (size_t c=0; c<cols; c++)
      {
        MPI_File_read(f, &(result->grid[r][c].type), 1, MPI_CHAR, MPI_STATUS_IGNORE);
        MPI_File_read(f, &(result->grid[r][c].value), 1, MPI_DOUBLE, MPI_STATUS_IGNORE);
        MPI_File_read(f, &(result->grid[r][c].velocity), 1, MPI_DOUBLE, MPI_STATUS_IGNORE);
        result->grid[r][c].dvalue = 0;
        if (result->grid[r][c].type == SENSOR)
          result->nb_records++;
      }
      MPI_File_seek(f, end - sup, MPI_SEEK_CUR);
    }
  }
  
  assert(result->records = calloc(result->nb_records, sizeof(double)));
  size_t id_record = 0;
  for (size_t r=0; r<rows; r++)
    for (size_t c=0; c<cols; c++)
      if (result->grid[r][c].type == SENSOR)
        result->records[id_record++] = result->grid[r][c].value * result->grid[r][c].value;
  
  MPI_File_close(&f);
}

static void export_distributed(char* filename, grid_t* grid)
{
  MPI_File f;
  MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &f);
  
  size_t rows = pos(grid->nb_rows, rank_col+1, size_col) - pos(grid->nb_rows, rank_col, size_col);
  size_t cols = pos(grid->nb_columns, rank_row+1, size_row) - pos(grid->nb_columns, rank_row, size_row);
  
  size_t inf = pos(grid->nb_columns, rank_row, size_row) * sizeof(double);
  size_t sup = pos(grid->nb_columns, rank_row+1, size_row) * sizeof(double);
  size_t end = pos(grid->nb_columns, size_row, size_row) * sizeof(double);
  
  size_t offset_row = pos(grid->nb_rows, rank_col, size_col) * grid->nb_columns * sizeof(double);
  MPI_File_seek(f, offset_row, MPI_SEEK_CUR);
  for (size_t r=0; r<rows; r++)
  {
    MPI_File_seek(f, inf, MPI_SEEK_CUR);
    for (size_t c=0; c<cols; c++)
      MPI_File_write(f, &(grid->grid[r][c].value), 1, MPI_DOUBLE, MPI_STATUS_IGNORE);
    MPI_File_seek(f, end - sup, MPI_SEEK_CUR);
  }
  
  MPI_File_close(&f);
}

static void sensor_distributed(char* filename, grid_t* grid)
{
  MPI_File f;
  MPI_File_open(MPI_COMM_WORLD, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &f);
  
  size_t row_start = pos(grid->nb_rows, rank_col, size_col);
  size_t col_start = pos(grid->nb_columns, rank_row, size_row);
  
  size_t nb_rows = pos(grid->nb_rows, rank_col+1, size_col) - row_start;
  size_t nb_cols = pos(grid->nb_columns, rank_row+1, size_row) - col_start;
  
  
  if (grid->nb_records > 0)
  {
    char *start_buffer, *end_buffer;
    assert(start_buffer = end_buffer = calloc(100*grid->nb_records, sizeof(char)));
    size_t id_record = 0;
    for (size_t r=0; r<nb_rows; r++)
      for (size_t c=0; c<nb_cols; c++)
        if (grid->grid[r][c].type == SENSOR)
          end_buffer += sprintf(end_buffer, "%lu %lu %lf\n", r + row_start, c + col_start, grid->records[id_record++]);
    
    MPI_File_write_shared(f, start_buffer, end_buffer - start_buffer, MPI_CHAR, MPI_STATUS_IGNORE);
    
    free(start_buffer);
  }
  
  MPI_File_close(&f);
}

static void share_rows(double* v, size_t cols, size_t rows)
{
  int next = (rank_col + 1) % size_col;
  int prev = (rank_col + size_col - 1) % size_col;
  MPI_Datatype t;
  MPI_Type_vector(cols+2, 1, 1, MPI_DOUBLE, &t);
  MPI_Type_commit(&t);
  MPI_Sendrecv(v+(cols+2)*rows, 1, t, next, 0, v, 1, t, prev, 0, COMM_COL, MPI_STATUS_IGNORE);
  MPI_Sendrecv(v+(cols+2), 1, t, prev, 0, v+(cols+2)*(rows+1), 1, t, next, 0, COMM_COL, MPI_STATUS_IGNORE);
  MPI_Type_free(&t);
}

static void share_cols(double* v, size_t cols, size_t rows)
{
  int next = (rank_row + 1) % size_row;
  int prev = (rank_row + size_row - 1) % size_row;
  MPI_Datatype t;
  MPI_Type_vector(rows+2, 1, cols+2, MPI_DOUBLE, &t);
  MPI_Type_commit(&t);
  MPI_Sendrecv(v+cols, 1, t, next, 0, v, 1, t, prev, 0, COMM_ROW, MPI_STATUS_IGNORE);
  MPI_Sendrecv(v+1, 1, t, prev, 0, v+cols+1, 1, t, next, 0, COMM_ROW, MPI_STATUS_IGNORE);
  MPI_Type_free(&t);
}

static void step_distributed(grid_t* grid, double dt)
{
  size_t rows = pos(grid->nb_rows, rank_col+1, size_col) - pos(grid->nb_rows, rank_col, size_col);
  size_t cols = pos(grid->nb_columns, rank_row+1, size_row) - pos(grid->nb_columns, rank_row, size_row);
  
  double *v = calloc((rows+2)*(cols+2), sizeof(double*));
  double *dv = calloc((rows+2)*(cols+2), sizeof(double*));
  
  for (size_t r=0; r<rows; r++)
  {
    for (size_t c=0; c<cols; c++)
    {
      v[(r+1)*(cols+2)+(c+1)] = grid->grid[r][c].value;
      dv[(r+1)*(cols+2)+(c+1)] = grid->grid[r][c].dvalue;
    }
  }
  
  share_rows(v, cols, rows);
  share_cols(v, cols, rows);
  share_rows(dv, cols, rows);
  share_cols(dv, cols, rows);
  
  for (size_t r=0; r<rows; r++)
  {
    for (size_t c=0; c<cols; c++)
    {
      if (grid->grid[r][c].type == VIBRATING || grid->grid[r][c].type == SENSOR)
      {
        double v2 = grid->grid[r][c].velocity * grid->grid[r][c].velocity;
        grid->grid[r][c].value = v[(r+1)*(cols+2)+(c+1)] + dt * dv[(r+1)*(cols+2)+(c+1)];
        grid->grid[r][c].dvalue = dv[(r+1)*(cols+2)+(c+1)];
        grid->grid[r][c].dvalue -= dt * v2 * 4 * v[(r+1)*(cols+2)+(c+1)];
        grid->grid[r][c].dvalue += dt * v2 * v[(r)*(cols+2)+(c+1)];
        grid->grid[r][c].dvalue += dt * v2 * v[(r+1)*(cols+2)+(c)];
        grid->grid[r][c].dvalue += dt * v2 * v[(r+2)*(cols+2)+(c+1)];
        grid->grid[r][c].dvalue += dt * v2 * v[(r+1)*(cols+2)+(c+2)];
      }
    }
  }
  
  size_t id_record = 0;
  for (size_t r=0; r<rows; r++)
    for (size_t c=0; c<cols; c++)
      if (grid->grid[r][c].type == SENSOR)
        grid->records[id_record++] += grid->grid[r][c].value * grid->grid[r][c].value;
  
  free(v);
  free(dv);
}

void distributed(char* arg_i,
                 int arg_iteration,
                 double arg_dt,
                 char* arg_lastdump,
                 char* arg_alldump,
                 char* arg_sensor)
{
  grid_t grid;
  read_distributed(arg_i, &grid);
  for (int i=0; i<arg_iteration; i++)
  {
    step_distributed(&grid, arg_dt);
    if (arg_alldump != NULL)
    {
      char s[256];
      sprintf(s, arg_alldump, i);
      export_distributed(s, &grid);
    }
  }
  
  if (arg_lastdump != NULL)
    export_distributed(arg_lastdump, &grid);
    
  if (arg_sensor != NULL)
    sensor_distributed(arg_sensor, &grid);
}
