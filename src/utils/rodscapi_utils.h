#ifndef RODSCAPI_H
#define RODSCAPI_H

#include "rodsClient.h"
#include "rodsPath.h"
#include <rpc/types.h>
#include "nfs.h"

int executeQuery(char full_path[],  rcComm_t* comm, int attributes_number, int selected_attributes[], OutputQuery outputQuery);
int getatt_aux(nfs_fh3 fh, fattr3 fat, rcComm_t* comm, char *path, int *size);


#endif // RODSCAPI_H
