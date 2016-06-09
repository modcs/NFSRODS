/* NFS-RODS: A Tool for Accessing iRODS Repositories
 * via the NFS Protocol
 * (C) 2016, Danilo Mendonça, Vandi Alves, Iure Fe,
 * Aleciano Lobo Junior, Francisco Airton Silva,
 * Gustavo Callou and Paulo Maciel <prmm@cin.ufpe.br>
 *
 * Original Copyright notice
 * UNFS3 NFS protocol procedures
 * (C) 2004, Pascal Schmidt
 * see file LICENSE for license details
 */

#ifndef UNFS3_READDIR_H
#define UNFS3_READDIR_H
#include "rodsClient.h"
READDIR3res
read_dir(const char *path, cookie3 cookie, cookieverf3 verf, count3 count);
uint32 directory_hash(const char *path);

READDIR3res read_diriRODS(const char *path, cookie3 cookie, cookieverf3 verf,
             count3 count, rcComm_t *rodsCommPtr,char *userName);

#endif
