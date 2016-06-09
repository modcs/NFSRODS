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

#ifndef UNFS3_FH_CACHE_H
#define UNFS3_FH_CACHE_H

/* statistics */
extern int fh_cache_max;
extern int fh_cache_use;
extern int fh_cache_hit;

void fh_cache_init(void);

char *fh_decomp(nfs_fh3 fh);
unfs3_fh_t fh_comp(const char *path, struct svc_req *rqstp, int need_dir);
unfs3_fh_t *fh_comp_ptr(const char *path, struct svc_req *rqstp, int need_dir);

char *fh_cache_add(uint32 dev, uint64 ino, const char *path);

#endif
