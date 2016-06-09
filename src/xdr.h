/*
 * NFSRODS XDR routine prototypes
 * Generated by rpcgen
 */

#ifndef UNFS3_XDR_H
#define UNFS3_XDR_H

/* MOUNT protocol */

extern bool_t xdr_fhandle3 (XDR *, fhandle3*);
extern bool_t xdr_mountstat3 (XDR *, mountstat3*);
extern bool_t xdr_mountres3_ok (XDR *, mountres3_ok*);
extern bool_t xdr_mountres3 (XDR *, mountres3*);
extern bool_t xdr_dirpath (XDR *, dirpath*);
extern bool_t xdr_name (XDR *, name*);
extern bool_t xdr_mountlist (XDR *, mountlist*);
extern bool_t xdr_mountbody (XDR *, mountbody*);
extern bool_t xdr_groups (XDR *, groups*);
extern bool_t xdr_groupnode (XDR *, groupnode*);
extern bool_t xdr_exports (XDR *, exports*);
extern bool_t xdr_exportnode (XDR *, exportnode*);

/* NFS protocol */

extern bool_t xdr_filename (XDR *, filename*);
extern bool_t xdr_nfspath (XDR *, nfspath*);
#if HAVE_XDR_UINT64 == 0
extern bool_t xdr_uint64 (XDR *, uint64*);
#endif
#if HAVE_XDR_UINT32 == 0
extern bool_t xdr_uint32 (XDR *, uint32*);
#endif
#if HAVE_XDR_INT32 == 0
extern bool_t xdr_int32 (XDR *, int32*);
#endif
extern bool_t xdr_filename3 (XDR *, filename3*);
extern bool_t xdr_nfspath3 (XDR *, nfspath3*);
extern bool_t xdr_fileid3 (XDR *, fileid3*);
extern bool_t xdr_cookie3 (XDR *, cookie3*);
extern bool_t xdr_cookieverf3 (XDR *, cookieverf3);
extern bool_t xdr_createverf3 (XDR *, createverf3);
extern bool_t xdr_writeverf3 (XDR *, writeverf3);
extern bool_t xdr_uid3 (XDR *, uid3*);
extern bool_t xdr_gid3 (XDR *, gid3*);
extern bool_t xdr_size3 (XDR *, size3*);
extern bool_t xdr_offset3 (XDR *, offset3*);
extern bool_t xdr_mode3 (XDR *, mode3*);
extern bool_t xdr_count3 (XDR *, count3*);
extern bool_t xdr_nfsstat3 (XDR *, nfsstat3*);
extern bool_t xdr_ftype3 (XDR *, ftype3*);
extern bool_t xdr_specdata3 (XDR *, specdata3*);
extern bool_t xdr_nfs_fh3 (XDR *, nfs_fh3*);
extern bool_t xdr_nfstime3 (XDR *, nfstime3*);
extern bool_t xdr_fattr3 (XDR *, fattr3*);
extern bool_t xdr_post_op_attr (XDR *, post_op_attr*);
extern bool_t xdr_wcc_attr (XDR *, wcc_attr*);
extern bool_t xdr_pre_op_attr (XDR *, pre_op_attr*);
extern bool_t xdr_wcc_data (XDR *, wcc_data*);
extern bool_t xdr_post_op_fh3 (XDR *, post_op_fh3*);
extern bool_t xdr_time_how (XDR *, time_how*);
extern bool_t xdr_set_mode3 (XDR *, set_mode3*);
extern bool_t xdr_set_uid3 (XDR *, set_uid3*);
extern bool_t xdr_set_gid3 (XDR *, set_gid3*);
extern bool_t xdr_set_size3 (XDR *, set_size3*);
extern bool_t xdr_set_atime (XDR *, set_atime*);
extern bool_t xdr_set_mtime (XDR *, set_mtime*);
extern bool_t xdr_sattr3 (XDR *, sattr3*);
extern bool_t xdr_diropargs3 (XDR *, diropargs3*);
extern bool_t xdr_GETATTR3args (XDR *, GETATTR3args*);
extern bool_t xdr_GETATTR3resok (XDR *, GETATTR3resok*);
extern bool_t xdr_GETATTR3res (XDR *, GETATTR3res*);
extern bool_t xdr_sattrguard3 (XDR *, sattrguard3*);
extern bool_t xdr_SETATTR3args (XDR *, SETATTR3args*);
extern bool_t xdr_SETATTR3resok (XDR *, SETATTR3resok*);
extern bool_t xdr_SETATTR3resfail (XDR *, SETATTR3resfail*);
extern bool_t xdr_SETATTR3res (XDR *, SETATTR3res*);
extern bool_t xdr_LOOKUP3args (XDR *, LOOKUP3args*);
extern bool_t xdr_LOOKUP3resok (XDR *, LOOKUP3resok*);
extern bool_t xdr_LOOKUP3resfail (XDR *, LOOKUP3resfail*);
extern bool_t xdr_LOOKUP3res (XDR *, LOOKUP3res*);
extern bool_t xdr_ACCESS3args (XDR *, ACCESS3args*);
extern bool_t xdr_ACCESS3resok (XDR *, ACCESS3resok*);
extern bool_t xdr_ACCESS3resfail (XDR *, ACCESS3resfail*);
extern bool_t xdr_ACCESS3res (XDR *, ACCESS3res*);
extern bool_t xdr_READLINK3args (XDR *, READLINK3args*);
extern bool_t xdr_READLINK3resok (XDR *, READLINK3resok*);
extern bool_t xdr_READLINK3resfail (XDR *, READLINK3resfail*);
extern bool_t xdr_READLINK3res (XDR *, READLINK3res*);
extern bool_t xdr_READ3args (XDR *, READ3args*);
extern bool_t xdr_READ3resok (XDR *, READ3resok*);
extern bool_t xdr_READ3resfail (XDR *, READ3resfail*);
extern bool_t xdr_READ3res (XDR *, READ3res*);
extern bool_t xdr_stable_how (XDR *, stable_how*);
extern bool_t xdr_WRITE3args (XDR *, WRITE3args*);
extern bool_t xdr_WRITE3resok (XDR *, WRITE3resok*);
extern bool_t xdr_WRITE3resfail (XDR *, WRITE3resfail*);
extern bool_t xdr_WRITE3res (XDR *, WRITE3res*);
extern bool_t xdr_createmode3 (XDR *, createmode3*);
extern bool_t xdr_createhow3 (XDR *, createhow3*);
extern bool_t xdr_CREATE3args (XDR *, CREATE3args*);
extern bool_t xdr_CREATE3resok (XDR *, CREATE3resok*);
extern bool_t xdr_CREATE3resfail (XDR *, CREATE3resfail*);
extern bool_t xdr_CREATE3res (XDR *, CREATE3res*);
extern bool_t xdr_MKDIR3args (XDR *, MKDIR3args*);
extern bool_t xdr_MKDIR3resok (XDR *, MKDIR3resok*);
extern bool_t xdr_MKDIR3resfail (XDR *, MKDIR3resfail*);
extern bool_t xdr_MKDIR3res (XDR *, MKDIR3res*);
extern bool_t xdr_symlinkdata3 (XDR *, symlinkdata3*);
extern bool_t xdr_SYMLINK3args (XDR *, SYMLINK3args*);
extern bool_t xdr_SYMLINK3resok (XDR *, SYMLINK3resok*);
extern bool_t xdr_SYMLINK3resfail (XDR *, SYMLINK3resfail*);
extern bool_t xdr_SYMLINK3res (XDR *, SYMLINK3res*);
extern bool_t xdr_devicedata3 (XDR *, devicedata3*);
extern bool_t xdr_mknoddata3 (XDR *, mknoddata3*);
extern bool_t xdr_MKNOD3args (XDR *, MKNOD3args*);
extern bool_t xdr_MKNOD3resok (XDR *, MKNOD3resok*);
extern bool_t xdr_MKNOD3resfail (XDR *, MKNOD3resfail*);
extern bool_t xdr_MKNOD3res (XDR *, MKNOD3res*);
extern bool_t xdr_REMOVE3args (XDR *, REMOVE3args*);
extern bool_t xdr_REMOVE3resok (XDR *, REMOVE3resok*);
extern bool_t xdr_REMOVE3resfail (XDR *, REMOVE3resfail*);
extern bool_t xdr_REMOVE3res (XDR *, REMOVE3res*);
extern bool_t xdr_RMDIR3args (XDR *, RMDIR3args*);
extern bool_t xdr_RMDIR3resok (XDR *, RMDIR3resok*);
extern bool_t xdr_RMDIR3resfail (XDR *, RMDIR3resfail*);
extern bool_t xdr_RMDIR3res (XDR *, RMDIR3res*);
extern bool_t xdr_RENAME3args (XDR *, RENAME3args*);
extern bool_t xdr_RENAME3resok (XDR *, RENAME3resok*);
extern bool_t xdr_RENAME3resfail (XDR *, RENAME3resfail*);
extern bool_t xdr_RENAME3res (XDR *, RENAME3res*);
extern bool_t xdr_LINK3args (XDR *, LINK3args*);
extern bool_t xdr_LINK3resok (XDR *, LINK3resok*);
extern bool_t xdr_LINK3resfail (XDR *, LINK3resfail*);
extern bool_t xdr_LINK3res (XDR *, LINK3res*);
extern bool_t xdr_READDIR3args (XDR *, READDIR3args*);
extern bool_t xdr_entry3 (XDR *, entry3*);
extern bool_t xdr_dirlist3 (XDR *, dirlist3*);
extern bool_t xdr_READDIR3resok (XDR *, READDIR3resok*);
extern bool_t xdr_READDIR3resfail (XDR *, READDIR3resfail*);
extern bool_t xdr_READDIR3res (XDR *, READDIR3res*);
extern bool_t xdr_READDIRPLUS3args (XDR *, READDIRPLUS3args*);
extern bool_t xdr_entryplus3 (XDR *, entryplus3*);
extern bool_t xdr_dirlistplus3 (XDR *, dirlistplus3*);
extern bool_t xdr_READDIRPLUS3resok (XDR *, READDIRPLUS3resok*);
extern bool_t xdr_READDIRPLUS3resfail (XDR *, READDIRPLUS3resfail*);
extern bool_t xdr_READDIRPLUS3res (XDR *, READDIRPLUS3res*);
extern bool_t xdr_FSSTAT3args (XDR *, FSSTAT3args*);
extern bool_t xdr_FSSTAT3resok (XDR *, FSSTAT3resok*);
extern bool_t xdr_FSSTAT3resfail (XDR *, FSSTAT3resfail*);
extern bool_t xdr_FSSTAT3res (XDR *, FSSTAT3res*);
extern bool_t xdr_FSINFO3args (XDR *, FSINFO3args*);
extern bool_t xdr_FSINFO3resok (XDR *, FSINFO3resok*);
extern bool_t xdr_FSINFO3resfail (XDR *, FSINFO3resfail*);
extern bool_t xdr_FSINFO3res (XDR *, FSINFO3res*);
extern bool_t xdr_PATHCONF3args (XDR *, PATHCONF3args*);
extern bool_t xdr_PATHCONF3resok (XDR *, PATHCONF3resok*);
extern bool_t xdr_PATHCONF3resfail (XDR *, PATHCONF3resfail*);
extern bool_t xdr_PATHCONF3res (XDR *, PATHCONF3res*);
extern bool_t xdr_COMMIT3args (XDR *, COMMIT3args*);
extern bool_t xdr_COMMIT3resok (XDR *, COMMIT3resok*);
extern bool_t xdr_COMMIT3resfail (XDR *, COMMIT3resfail*);
extern bool_t xdr_COMMIT3res (XDR *, COMMIT3res*);

#endif
