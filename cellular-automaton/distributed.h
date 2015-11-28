#ifndef DISTRIBUTED_H
#define DISTRIBUTED_H

#include "types.h"

void init_distributed(int argc, char** argv, int arg_grid_x, int arg_grid_y);
void exit_distributed(int err);

void distributed(char* arg_i,
                 int arg_iteration,
                 double arg_dt,
                 char* arg_lastdump,
                 char* arg_alldump,
                 char* arg_sensor);

#endif
