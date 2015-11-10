#include "stdio.h"
#include "stdlib.h"



typedef struct
{
	char   type;
	size_t rows;
	size_t cols;
	double gvelocity; // Only for type 1

	char   *matrix_types;
	double *matrix_values;
	double *matrix_velocities; // Only for type 2

} data_t;



data_t* load(char *path)
{
	FILE *file = fopen(path, "rb");

	data_t *data = calloc(1, sizeof(data_t));

	fread(&data->type,      sizeof(char),   1, file);
	fread(&data->rows,      sizeof(size_t), 1, file);
	fread(&data->cols,      sizeof(size_t), 1, file);
	if (data->type == 1)
	fread(&data->gvelocity, sizeof(double), 1, file);

	data->matrix_types      = calloc(data->rows * data->cols, sizeof(char)  );
	data->matrix_values     = calloc(data->rows * data->cols, sizeof(double));
	if (data->type == 2)
	data->matrix_velocities = calloc(data->rows * data->cols, sizeof(double));

	for (size_t i=0; i<data->rows*data->cols;++i)
	{
		fread(&data->matrix_types[i],      sizeof(char),   1, file);
		fread(&data->matrix_values[i],     sizeof(double), 1, file);
		if (data->type == 2)
		fread(&data->matrix_velocities[i], sizeof(double), 1, file);
	}

	fclose (file);

	return data;
}

void save(char *path, data_t *data)
{
	FILE *file = fopen(path, "wb");
	fwrite(data->matrix_values, sizeof(double), data->rows*data->cols, file);
	fclose(file);
}

int main(int argc, char *argv[])
{
	data_t* data = NULL;

	if (argc > 1)
	{
		data = load(argv[1]);
		printf("%d %d %d %f\n", data->type, data->rows, data->cols, data->gvelocity);
		printf("%p %p %p\n", data->matrix_types, data->matrix_values, data->matrix_velocities);
	}
	if (argc > 2)
	{
		save(argv[2], data);
	}

	return 0;
}