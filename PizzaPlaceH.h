#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#define NCOOK 2
#define NOVEN 5
#define NDELIVERER 10
#define NORDERLOW 1
#define NORDERHIGH 5
#define TORDERLOW 1
#define TORDERHIGH 5
#define TPREP 1
#define TBAKE 10
#define TLOW 5
#define THIGH 15

void *order(void * customer);
