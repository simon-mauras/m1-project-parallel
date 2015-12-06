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


static void send_row(double** v, size_t cols, size_t row_from, size_t row_to, int from, int to)
{
  if (rank_col == from)
    MPI_Send(v[row_from], cols, MPI_DOUBLE, to, 0, COMM_COL);
  if (rank_col == to)
    MPI_Recv(v[row_to], cols, MPI_DOUBLE, from, 0, COMM_COL, MPI_STATUS_IGNORE);
}

static void share_rows(double** v, double** dv, size_t cols, size_t rows)
{
  int CPU_A, CPU_B;
  
  CPU_A = size_col-1;
  CPU_B = 0;
  
  if (rank_col == CPU_A || rank_col == CPU_B)
  {
    send_row(v, cols+2, rows, 0, CPU_A, CPU_B);
    send_row(v, cols+2, 1, rows+1, CPU_B, CPU_A);
    send_row(dv, cols+2, rows, 0, CPU_A, CPU_B);
    send_row(dv, cols+2, 1, rows+1, CPU_B, CPU_A);
  }
  
  CPU_A = (rank_col % 2 == 0) ? rank_col : rank_col-1;
  CPU_B = (rank_col % 2 == 1) ? rank_col : rank_col+1;
  if (CPU_A >= 0 && CPU_B < size_col)
  {
    send_row(v, cols+2, rows, 0, CPU_A, CPU_B);
    send_row(v, cols+2, 1, rows+1, CPU_B, CPU_A);
    send_row(dv, cols+2, rows, 0, CPU_A, CPU_B);
    send_row(dv, cols+2, 1, rows+1, CPU_B, CPU_A);
  }
  
  CPU_A = (rank_col % 2 == 1) ? rank_col : rank_col-1;
  CPU_B = (rank_col % 2 == 0) ? rank_col : rank_col+1;
  if (CPU_A >= 0 && CPU_B < size_col)
  {
    send_row(v, cols+2, rows, 0, CPU_A, CPU_B);
    send_row(v, cols+2, 1, rows+1, CPU_B, CPU_A);
    send_row(dv, cols+2, rows, 0, CPU_A, CPU_B);
    send_row(dv, cols+2, 1, rows+1, CPU_B, CPU_A);
  }
}

static void send_col(double** v, size_t rows, size_t col_from, size_t col_to, int from, int to)
{
  double tmp[rows];
  if (rank_row == from)
  {
    for (size_t r=0; r<rows; r++)
      tmp[r] = v[r][col_from];
    MPI_Send(tmp, rows, MPI_DOUBLE, to, 0, COMM_ROW);
  }
  if (rank_row == to)
  {
    MPI_Recv(tmp, rows, MPI_DOUBLE, from, 0, COMM_ROW, MPI_STATUS_IGNORE);
    for (size_t r=0; r<rows; r++)
      v[r][col_to] = tmp[r];
  }
}

static void share_cols(double** v, double** dv, size_t cols, size_t rows)
{
  int CPU_A, CPU_B;
  
  CPU_A = size_row-1;
  CPU_B = 0;
  if (rank_row == CPU_A || rank_row == CPU_B)
  {
    send_col(v, rows+2, cols, 0, CPU_A, CPU_B);
    send_col(v, rows+2, 1, cols+1, CPU_B, CPU_A);
    send_col(dv, rows+2, cols, 0, CPU_A, CPU_B);
    send_col(dv, rows+2, 1, cols+1, CPU_B, CPU_A);
  }
  
  CPU_A = (rank_row % 2 == 0) ? rank_row : rank_row-1;
  CPU_B = (rank_row % 2 == 1) ? rank_row : rank_row+1;
  if (CPU_A >= 0 && CPU_B < size_row)
  {
    send_col(v, rows+2, cols, 0, CPU_A, CPU_B);
    send_col(v, rows+2, 1, cols+1, CPU_B, CPU_A);
    send_col(dv, rows+2, cols, 0, CPU_A, CPU_B);
    send_col(dv, rows+2, 1, cols+1, CPU_B, CPU_A);
  }
  
  CPU_A = (rank_row % 2 == 1) ? rank_row : rank_row-1;
  CPU_B = (rank_row % 2 == 0) ? rank_row : rank_row+1;
  if (CPU_A >= 0 && CPU_B < size_row)
  {
    send_col(v, rows+2, cols, 0, CPU_A, CPU_B);
    send_col(v, rows+2, 1, cols+1, CPU_B, CPU_A);
    send_col(dv, rows+2, cols, 0, CPU_A, CPU_B);
    send_col(dv, rows+2, 1, cols+1, CPU_B, CPU_A);
  }
}

static void step_distributed(grid_t* grid, double dt)
{
  size_t rows = pos(grid->nb_rows, rank_col+1, size_col) - pos(grid->nb_rows, rank_col, size_col);
  size_t cols = pos(grid->nb_columns, rank_row+1, size_row) - pos(grid->nb_columns, rank_row, size_row);
  
  double **v = calloc(rows+2, sizeof(double*));
  double **dv = calloc(rows+2, sizeof(double*));
  for (size_t r=0; r<rows+2; r++)
  {
    v[r] = calloc(cols+2, sizeof(double));
    dv[r] = calloc(cols+2, sizeof(double));
  }
  
  for (size_t r=0; r<rows; r++)
  {
    for (size_t c=0; c<cols; c++)
    {
      v[r+1][c+1] = grid->grid[r][c].value;
      dv[r+1][c+1] = grid->grid[r][c].dvalue;
    }
  }
  
  share_rows(v, dv, cols, rows);
  share_cols(v, dv, cols, rows);
  
  for (size_t r=0; r<rows; r++)
  {
    for (size_t c=0; c<cols; c++)
    {
      if (grid->grid[r][c].type == VIBRATING || grid->grid[r][c].type == SENSOR)
      {
        double v2 = grid->grid[r][c].velocity * grid->grid[r][c].velocity;
        grid->grid[r][c].value = v[r+1][c+1] + dt * dv[r+1][c+1];
        grid->grid[r][c].dvalue = dv[r+1][c+1];
        grid->grid[r][c].dvalue -= dt * v2 * 4 * v[r+1][c+1];
        grid->grid[r][c].dvalue += dt * v2 * v[r][c+1];
        grid->grid[r][c].dvalue += dt * v2 * v[r+1][c];
        grid->grid[r][c].dvalue += dt * v2 * v[r+2][c+1];
        grid->grid[r][c].dvalue += dt * v2 * v[r+1][c+2];
      }
    }
  }
  
  size_t id_record = 0;
  for (size_t r=0; r<rows; r++)
    for (size_t c=0; c<cols; c++)
      if (grid->grid[r][c].type == SENSOR)
        grid->records[id_record++] += grid->grid[r][c].value * grid->grid[r][c].value;
  
  for (size_t r=0; r<rows+2; r++)
  {
    free(v[r]);
    free(dv[r]);
  }
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
