/*
 * UNFS3 attribute handling
 * (C) 2004, Pascal Schmidt
 * see file LICENSE for license details
 */

#ifndef NFS_ATTR_H
#define NFS_ATTR_H

#include "rodsClient.h"

nfsstat3 is_reg(void);

mode_t type_to_mode(ftype3 ftype);

post_op_attr get_post_attr(const char *path, nfs_fh3 fh, struct svc_req *req);
post_op_attr get_post_stat(const char *path, struct svc_req *req);
post_op_attr get_post_cached(struct svc_req *req);
post_op_attr get_post_buf(backend_statstruct buf, struct svc_req *req);
pre_op_attr  get_pre_cached(void);

nfsstat3 set_attr(const char *path, nfs_fh3 fh, sattr3 sattr);
nfsstat3 set_attr_irods(char *path, int uidProxy, nfs_fh3 nfh, sattr3 news,char * owner,rcComm_t *conn);


mode_t create_mode(sattr3 sattr);

nfsstat3 atomic_attr(sattr3 sattr);

#endif
