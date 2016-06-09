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

#ifndef UNFS3_BACKEND_H
#define UNFS3_BACKEND_H

#ifdef WIN32
#include "backend_win32.h"
#else
#include "backend_unix.h"
#endif /* WIN32 */

#endif
