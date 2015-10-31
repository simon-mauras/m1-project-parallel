#ifndef TYPES_H
#define TYPES_H

typedef struct {
  char type;
  double value;
  double dvalue;
  double velocity;
} block_t;

typedef struct {
  size_t nb_columns;
  size_t nb_rows;
  block_t** grid;
} grid_t;

#endif