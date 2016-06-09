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

#ifndef UNFS3_ERROR_H
#define UNFS3_ERROR_H

nfsstat3 symlink_err(void);
nfsstat3 remove_err(void);
nfsstat3 rmdir_err(void);
nfsstat3 setattr_err(void);
nfsstat3 readdir_err(void);
nfsstat3 mkdir_err(void);
nfsstat3 mknod_err(void);
nfsstat3 link_err(void);
nfsstat3 lookup_err(void);
nfsstat3 readlink_err(void);
nfsstat3 read_err(void);
nfsstat3 write_open_err(void);
nfsstat3 write_write_err(void);
nfsstat3 create_err(void);
nfsstat3 rename_err(void);

nfsstat3 join(nfsstat3, nfsstat3);
nfsstat3 join3(nfsstat3, nfsstat3, nfsstat3);

#endif
