#ifndef TYPES_H
#define TYPES_H

extern const char VIBRATING;
extern const char WALL;
extern const char SENSOR;

typedef struct {
  char type;
  double value;
  double dvalue;
  double velocity;
} block_t;

typedef struct {
  size_t nb_columns;
  size_t nb_rows;
  size_t nb_records;
  block_t** grid;
  double* records;
} grid_t;

#endif
