#ifndef __FILE_CONFIG_H_
#define __FILE_CONFIG_H_

#include "mytypes.h"
#include "server_channel.h"

int readln_from_a_file(FILE *fp, char *line, int size);
int read_file_config(char *file, SC_CONFIG *config);

#endif

