
/*
 * UNFS3 NFS protocol procedures
 * (C) 2004, Pascal Schmidt
 * see file LICENSE for license details
 */

#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>		       /* needed for statfs() on NetBSD */
#if HAVE_SYS_MOUNT_H == 1
#include <sys/mount.h>		       /* dito */
#endif
#if HAVE_SYS_VMOUNT_H == 1
#include <sys/vmount.h>		       /* AIX */
#endif
#include <rpc/rpc.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>
#ifndef WIN32
#include <sys/socket.h>
#include <sys/un.h>
#endif				       /* WIN32 */

#if HAVE_STATVFS == 1
# include <sys/statvfs.h>
#else
# define statvfs statfs
#endif

#include "utils/utils.h"

#include "nfs.h"
#include "mount.h"
#include "fh.h"
#include "fh_cache.h"
#include "attr.h"
#include "readdir.h"
#include "user.h"
#include "error.h"
#include "fd_cache.h"
#include "daemon.h"
#include "backend.h"
#include "Config/exports.h"
#include "Extras/cluster.h"
#include "irodsapi/rodscapi.h"
#include "utils/utils.h"

bool NFS_RODS_LDAP_ACTIVATED = TRUE;


/*
 * NFS RODS.
 *
 * Utility functions. The following functions
 * are auxiliary functions from the original
 * (unfs) code, and newly added for our purposes.
 */


/*
 * decompose filehandle and switch user if permitted access
 * otherwise zero result structure and return with error status
 */
#define PREP(p,f) do {						\
unfs3_fh_t *fh = (void *)f.data.data_val; \
switch_to_root();				\
p = fh_decomp(f);				\
if (exports_options(p, rqstp, NULL, NULL) == -1) { \
memset(&result, 0, sizeof(result));	\
if (p)				\
result.status = NFS3ERR_ACCES;	\
else					\
result.status = NFS3ERR_STALE;	\
return &result;			\
}						\
if (fh->pwhash != export_password_hash) { \
memset(&result, 0, sizeof(result));	\
result.status = NFS3ERR_STALE;        \
return &result;                       \
}                                         \
switch_user(rqstp);			\
} while (0)

/**
 * @brief getUID
 * retorna o ID do usuario logado no cliente NFS
 * @param arg
 * @return
 */
int getUID(caddr_t arg){
    struct authunix_parms *auth = (void *) arg;

    int uidProxy =(int) auth->aup_uid;

    return uidProxy;
}

/**
 * @brief getGID
 * retorna o ID do grupo do usuario logado no cliente NFS
 * @param arg
 * @return
 */
int getGID(caddr_t arg){
    struct authunix_parms *auth = (void *) arg;

    int gidProxy =(int) auth->aup_gid;

    return gidProxy;
}

/**
 * @brief getNFSDirectory
 * Retorna o diretorio que diferencia um usuario de outro no servidor nfs.
 * @param arg
 * @return
 */
char* getNFSDirectory(caddr_t arg){

    char * NFS_RODS_DIRECTORY = malloc(30 * (sizeof(char)));

    if(NFS_RODS_LDAP_ACTIVATED){
        int uid = getUID(arg);
        sprintf(NFS_RODS_DIRECTORY, "%d", uid);


    }else{
        NFS_RODS_DIRECTORY="public";
    }

    return NFS_RODS_DIRECTORY;

}

/**
 * @brief getRealPathNFS
 * retorna o path do servidor NFS utilizado em uma operacao levando em conta a mudança do path para cada usuario
 * @param userPath
 * @param arg
 * @return
 */
char* getRealPathNFS(char *userPath,caddr_t arg){

    char* NFS_RODS_DIRECTORY = getNFSDirectory(arg);

    char* path = concat( export_path,"/");

    path = concat(path,NFS_RODS_DIRECTORY);


    //IF we have concated path dont do it aganin
    if(strstr(userPath,path)!=NULL)
        return userPath;

    char * endPath = substring(userPath,(strlen(export_path)+1),strlen(userPath));

    path=concat(path,endPath);

    debug("getRealPathNFS path %s",path);
    debug("getRealPathNFS export leght %d",strlen(export_path));
    debug("getRealPathNFS user leght %d",strlen(userPath));

    //if(strlen(export_path)!=strlen(userPath))
    //path = concat( path,substring(userPath,strlen(export_path),strlen(userPath)));

    return path;

}

/**
 * @brief getIRODSpathGetAttr
 * Retorna o path que será utilizado para pesquisas no irods para o GetAttr
 * @param path
 * @param uid
 * @return
 */
char* getIRODSpathGetAttr(char * path,int uid){


    char* id = malloc(300*sizeof(char));

    sprintf(id,"%d",uid);

    char * endPath = substring(path,(strlen(export_path)+strlen(id)+2),strlen(path));

    //char * result = concat(export_path,endPath);
    char * result =endPath;

    return result;

}

/**
 * @brief cat_name
 *
 * @param path
 * @param name
 * @param result
 * @return
 */
nfsstat3 cat_name(const char *path, const char *name, char *result)
{
    char *last;

    if (!path)
        return NFS3ERR_STALE;

    if (!name)
        return NFS3ERR_ACCES;

    if (name[0] == 0 || strchr(name, '/') != NULL)
        return NFS3ERR_ACCES;

    if (strlen(path) + strlen(name) + 2 > NFS_MAXPATHLEN)
        return NFS3ERR_NAMETOOLONG;

    if (strcmp(name, ".") == 0) {
        strcpy(result, path);
        return NFS3_OK;
    }

    /*
     * Irix clients do lookups for .. and then use the
     * resulting filehandle for more lookups, causing them
     * to get filehandles that fh_decomp_raw will refuse to
     * resolve. Export list handling will also get very
     * confused if we allow such filehandles.
     */
    if (strcmp(name, "..") == 0) {
        last = strrchr(path, '/');
        if (!last || last == path)
            strcpy(result, "/");
        else {
            *last = 0;
            strcpy(result, path);
            *last = '/';
        }
        return NFS3_OK;
    }

    sprintf(result, "%s/%s", path, name);
    return NFS3_OK;
}

/**
 * @brief nfsproc3_null_3_svc NULL - Do nothing
 *
 * Procedure NULL does not do any work. It is made available to
 * allow server response testing and timing.
 * @return
 */
void *nfsproc3_null_3_svc(U(void *argp), U(struct svc_req *rqstp))
{
    static void *result = NULL;

    return &result;
}

/**
 * @brief nfsproc3_getattr_3_svc GETATTR - Get file attributes
 *
 * Procedure GETATTR retrieves the attributes for a specified
 * file system object. The object is identified by the file
 * handle that the server returned as part of the response
 * from a LOOKUP, CREATE, MKDIR, SYMLINK, MKNOD, or
 * READDIRPLUS procedure (or from the MOUNT service,
 * described elsewhere).
 *
 * @param argp The file handle of an object whose attributes are to be retrieved.
 * @param rqstp
 * @return
 */
GETATTR3res *nfsproc3_getattr_3_svc(GETATTR3args * argp,
                                    struct svc_req * rqstp)
{
   debug("GETATTR START","0");
    static GETATTR3res result;
    char *path;
    post_op_attr post;

    PREP(path, argp->object);

    //char * NFS_RODS_DIRECTORY = getNFSDirectory(rqstp->rq_clntcred);


    char *pathRods = getIRODSpathGetAttr(path,getUID(rqstp->rq_clntcred));

    char * name = getRodsPath(getUID(rqstp->rq_clntcred),pathRods);


    //char *rodspath = concat(ZONE, path);
    //char *rodspath = getRodsPath(getUID(rqstp->rq_clntcred),getIRODSpathGetAttr(path,getUID(rqstp->rq_clntcred)));
    //char *newpath = concat( path,"/");
    //newpath = concat( newpath,NFS_RODS_DIRECTORY);

    //path = newpath;
    path = getRealPathNFS(path,rqstp->rq_clntcred);

    debug("GETATTR path %s",path);
    debug("GETATTR irodspath %s",name);

    post = get_post_cached(rqstp);

    result.status = NFS3_OK;
    result.GETATTR3res_u.resok.obj_attributes =
            post.post_op_attr_u.attributes;

    if(result.GETATTR3res_u.resok.obj_attributes.type == NF3REG){

        rcComm_t* comm;

        if(NFS_RODS_LDAP_ACTIVATED){
            int uid = getUID(rqstp->rq_clntcred);
            comm = rodsConnectLDAP(uid);
        }else{
            comm = rodsConnect();
        }

        int i = rodsLogin(comm);


        rodsLong_t size = fileSize(name, comm);

        result.GETATTR3res_u.resok.obj_attributes.size = (uint64_t) size;
        result.GETATTR3res_u.resok.obj_attributes.used = (uint64_t) size;

        result.GETATTR3res_u.resok.obj_attributes.uid=getUID(rqstp->rq_clntcred);
        result.GETATTR3res_u.resok.obj_attributes.gid=getGID(rqstp->rq_clntcred);

        rcDisconnect(comm);
    }

    free(pathRods);
    free(name);
    //free(newpath);



    debug("GETATTR END","0");
    return &result;
}


/*
 * check ctime guard for SETATTR procedure
 */
static nfsstat3 in_sync(sattrguard3 guard, pre_op_attr pre)
{

    /*
    if (!pre.attributes_follow)
        return NFS3ERR_STALE;

    if (!guard.check)
        return NFS3_OK;

    if (guard.sattrguard3_u.obj_ctime.seconds !=
            pre.pre_op_attr_u.attributes.ctime.seconds)
        return NFS3ERR_NOT_SYNC;
        */
    //Always sync

    return NFS3_OK;
}

SETATTR3res *nfsproc3_setattr_3_svc(SETATTR3args * argp,
                                    struct svc_req * rqstp)
{
    debug("SETTATTR START","");
    static SETATTR3res result;
    pre_op_attr pre,pre_nfs;
    char *path, *owner;




    PREP(path, argp->object);



    path = getRealPathNFS(path,rqstp->rq_clntcred);

    debug("PATH %s",path);


    pre = get_pre_cached();

    /*

    struct authunix_parms *auth = (void *) rqstp->rq_clntcred;

    int uidProxy =(int) auth->aup_uid;


    if (pre.attributes_follow) {

       // pre_nfs=get_pre_cached_irods(path,&owner);

    }

    */


    result.status = join(in_sync(argp->guard, pre), exports_rw());

    if (result.status == NFS3_OK){
        //change file
        result.status = set_attr(path, argp->object, argp->new_attributes);
        //change irods

        rcComm_t* comm;

        if(NFS_RODS_LDAP_ACTIVATED){
            int uid = getUID(rqstp->rq_clntcred);
            comm = rodsConnectLDAP(uid);
            //debuguinho("Conseguiu autenticar ldap (readdir)\n", 0);
        }else{
            comm = rodsConnect();
        }

        int i = rodsLogin(comm);

        char *pathRods = getIRODSpathGetAttr(path,getUID(rqstp->rq_clntcred));

        char * name = getRodsPath(getUID(rqstp->rq_clntcred),pathRods);

        debug("IRODS ACCESS PATH %s",name);


        char* userName=malloc (300 *(sizeof (char)));

        int resultLdap = getLdapName(getUID(rqstp->rq_clntcred),userName);

        if(resultLdap<0){
           debug("CREATE - Cant Connect to LDAP", "");
           exit(0);
        }

        set_attr_irods(name,getUID(rqstp->rq_clntcred), argp->object, argp->new_attributes,userName,comm);

        free(userName);

        free(name);

        free(pathRods);

        rcDisconnect(comm);

    }

    /* overlaps with resfail */
    result.SETATTR3res_u.resok.obj_wcc.before = pre;
    result.SETATTR3res_u.resok.obj_wcc.after = get_post_stat(path, rqstp);

    debug("SETTATTR-stop","");
    return &result;
}


LOOKUP3res *nfsproc3_lookup_3_svc(LOOKUP3args * argp, struct svc_req * rqstp)
{


    //TODO não termina de ler esse método ?
    debug("START LOOKUP object %s",argp->what.name);
    //debuguinho("Caiu em lookup.\n",0);
    static LOOKUP3res result;
    unfs3_fh_t *fh;
    char *path;
    char obj[NFS_MAXPATHLEN];
    backend_statstruct buf;
    int res;
    uint32 gen;

    PREP(path, argp->what.dir);


    //char * NFS_RODS_DIRECTORY = getNFSDirectory(rqstp->rq_clntcred);

    //char *rodspath = concat(ZONE, path);
    char *rodspath = getRodsPath(getUID(rqstp->rq_clntcred),path);
    char *newpath = concat( path,"/");
   // newpath = concat( newpath,NFS_RODS_DIRECTORY);

    //path = newpath;
    path = getRealPathNFS(path,rqstp->rq_clntcred);

    debug("LOOKUP path %s",path);
    debug("LOOKUP irodspath %s",rodspath);


    result.status = cat_name(path, argp->what.name, obj);

    cluster_lookup(obj, rqstp, &result.status);

    if (result.status == NFS3_OK) {
        res = backend_lstat(obj, &buf);
        if (res == -1)
            result.status = lookup_err();
        else {
            if (strcmp(argp->what.name, ".") == 0 ||
                    strcmp(argp->what.name, "..") == 0) {
                fh = fh_comp_ptr(obj, rqstp, 0);
            } else {
                gen = backend_get_gen(buf, FD_NONE, obj);
                fh = fh_extend(argp->what.dir, buf.st_dev, buf.st_ino, gen);
                fh_cache_add(buf.st_dev, buf.st_ino, obj);
            }

            if (fh) {
                result.LOOKUP3res_u.resok.object.data.data_len =
                        fh_length(fh);
                result.LOOKUP3res_u.resok.object.data.data_val = (char *) fh;

                result.LOOKUP3res_u.resok.obj_attributes =
                        get_post_buf(buf, rqstp);
            } else {
                /* path was too long */
                result.status = NFS3ERR_NAMETOOLONG;
            }
        }
    }


    /* overlaps with resfail */
    result.LOOKUP3res_u.resok.dir_attributes = get_post_stat(path, rqstp);






    if(result.LOOKUP3res_u.resok.obj_attributes.post_op_attr_u.attributes.type  == NF3REG){
        rcComm_t* comm;

        if(NFS_RODS_LDAP_ACTIVATED){
            int uid = getUID(rqstp->rq_clntcred);
            comm = rodsConnectLDAP(uid);
            //debuguinho("Conseguiu autenticar no LDAP.",0);
        }else{
            comm = rodsConnect();
        }

        int i = rodsLogin(comm);

        rodsLong_t size = fileSize(rodspath, comm);


        result.LOOKUP3res_u.resok.obj_attributes.post_op_attr_u.attributes.size =
                (uint64) size;
        result.LOOKUP3res_u.resok.obj_attributes.post_op_attr_u.attributes.used =
                (uint64) size;

        result.LOOKUP3res_u.resok.obj_attributes.post_op_attr_u.attributes.uid=getUID(rqstp->rq_clntcred);
        result.LOOKUP3res_u.resok.obj_attributes.post_op_attr_u.attributes.gid=getGID(rqstp->rq_clntcred);

        rcDisconnect(comm);
    }



    free(rodspath);
    free(newpath);



    printf("END LOOKUP object","");
    return &result;
}

/**
 * @brief nfsproc3_access_3_svc ACCESS - Check Access Permission
 *
 * Procedure ACCESS determines the access rights that a user, as identified
 * by the credentials in the request, has with respect to a file system object.
 * The client encodes the set of permissions that are to be checked in a bit mask.
 *  The server checks the permissions encoded in the bit mask.
 * A status of NFS3_OK is returned along with a bit mask encoded with
 *  the permissions that the client is allowed.
 *
 * The results of this procedure are necessarily advisory in nature.
 *  That is, a return status of NFS3_OK and the appropriate bit set in the bit mask
 * does not imply that such access will be allowed to the file system object
 * in the future, as access rights can be revoked by the server at any time.
 *
 * @param argp
 * @param rqstp
 * @return
 */
ACCESS3res *nfsproc3_access_3_svc(ACCESS3args * argp, struct svc_req * rqstp)
{
    debug("ACCESS START ","");
    static ACCESS3res result;
    char *path;
    post_op_attr post;
    mode_t mode;
    int access = 0;

    PREP(path, argp->object);



    path = getRealPathNFS(path,rqstp->rq_clntcred);

    debug("ACCESS PATH %s",path);



    rcComm_t* comm;

    if(NFS_RODS_LDAP_ACTIVATED){
        int uid = getUID(rqstp->rq_clntcred);
        comm = rodsConnectLDAP(uid);
        //debuguinho("Conseguiu autenticar ldap (readdir)\n", 0);
    }else{
        comm = rodsConnect();
    }

    int i = rodsLogin(comm);

    char *pathRods = getIRODSpathGetAttr(path,getUID(rqstp->rq_clntcred));

    char * name = getRodsPath(getUID(rqstp->rq_clntcred),pathRods);

    debug("IRODS ACCESS PATH %s",name);




    getFile(path, name, FALSE, TRUE, comm);


    free(name);
    free(pathRods);

    rcDisconnect(comm);



    post = get_post_cached(rqstp);
    mode = post.post_op_attr_u.attributes.mode;

    /* owner permissions */
    if (is_owner(st_cache.st_uid, rqstp)) {
        if (mode & S_IRUSR)
            access |= ACCESS3_READ;
        if (mode & S_IWUSR)
            access |= ACCESS3_MODIFY | ACCESS3_EXTEND;
        if (mode & S_IXUSR) {
            access |= ACCESS3_EXECUTE;
            if (opt_readable_executables)
                access |= ACCESS3_READ;
        }
    } else if (has_group(st_cache.st_gid, rqstp)) {
        /* group permissions */
        if (mode & S_IRGRP)
            access |= ACCESS3_READ;
        if (mode & S_IWGRP)
            access |= ACCESS3_MODIFY | ACCESS3_EXTEND;
        if (mode & S_IXGRP) {
            access |= ACCESS3_EXECUTE;
            if (opt_readable_executables)
                access |= ACCESS3_READ;
        }
    } else {
        /* other permissions */
        if (mode & S_IROTH)
            access |= ACCESS3_READ;
        if (mode & S_IWOTH)
            access |= ACCESS3_MODIFY | ACCESS3_EXTEND;
        if (mode & S_IXOTH) {
            access |= ACCESS3_EXECUTE;
            if (opt_readable_executables)
                access |= ACCESS3_READ;
        }
    }

    /* root is allowed everything */
    if (get_uid(rqstp) == 0)
        access |= ACCESS3_READ | ACCESS3_MODIFY | ACCESS3_EXTEND;

    /* adjust if directory */
    if (post.post_op_attr_u.attributes.type == NF3DIR) {
        if (access & (ACCESS3_READ | ACCESS3_EXECUTE))
            access |= ACCESS3_LOOKUP;
        if (access & ACCESS3_MODIFY)
            access |= ACCESS3_DELETE;
        access &= ~ACCESS3_EXECUTE;
    }

    //FIXME
    access |= ACCESS3_READ | ACCESS3_MODIFY | ACCESS3_EXTEND;

    result.status = NFS3_OK;
    result.ACCESS3res_u.resok.access = access & argp->access;
    result.ACCESS3res_u.resok.obj_attributes = post;

    debug("ACCESS END ","");
    return &result;
}

/**
 * @brief nfsproc3_readlink_3_svc READLINK - Read from symbolic link
 *
 * This callback is not implemented in NFSRODS.
 *
 * @param argp
 * @param rqstp
 * @return
 */
READLINK3res *nfsproc3_readlink_3_svc(READLINK3args * argp,
                                      struct svc_req * rqstp)
{

    static READLINK3res result;

    /*   char *path;
    static char buf[NFS_MAXPATHLEN];
    int res;

    PREP(path, argp->symlink);

    res = backend_readlink(path, buf, NFS_MAXPATHLEN - 1);
    if (res == -1)
    result.status = readlink_err();
    else {
     readlink does not NULL-terminate
    buf[res] = 0;

    result.status = NFS3_OK;
    result.READLINK3res_u.resok.data = buf;
    }

     overlaps with resfail
    result.READLINK3res_u.resok.symlink_attributes =
    get_post_stat(path, rqstp); */
    result.status = NFS3ERR_NOTSUPP;

    return &result;
}

/**
 * @brief nfsproc3_read_3_svc  READ - Read From file
 *
 * Procedure READ reads data from a file.
 *
 * @param argp
 * @param rqstp
 * @return
 */
READ3res *nfsproc3_read_3_svc(READ3args * argp, struct svc_req * rqstp)
{

    debug("READ START","");
    static READ3res result;
    char *path;
    int fd, res;
    static char buf[NFS_MAXDATA_TCP + 1];
    unsigned int maxdata;
    int bytesRead;

    if (get_socket_type(rqstp) == SOCK_STREAM)
        maxdata = NFS_MAXDATA_TCP;
    else
        maxdata = NFS_MAXDATA_UDP;

    PREP(path, argp->file);
    result.status = is_reg();

    /* handle reading of executables */
    read_executable(rqstp, st_cache);

    /* handle read of owned files */
    read_by_owner(rqstp, st_cache);

    /* if bigger than rtmax, truncate length */
    if (argp->count > maxdata)
        argp->count = maxdata;

    if (result.status == NFS3_OK) {
        /*fd = fd_open(path, argp->file, UNFS3_FD_READ, TRUE);

        debug("Leu arquivo %s\n", path);

        if (fd != -1) {
            // read one more to check for eof
            res = backend_pread(fd, buf, argp->count + 1, argp->offset);

            // eof if we could not read one more
            result.READ3res_u.resok.eof = (res <= (int64) argp->count);

            // close for real when hitting eof
            if (result.READ3res_u.resok.eof)
            fd_close(fd, UNFS3_FD_READ, FD_CLOSE_REAL);
            else {
            fd_close(fd, UNFS3_FD_READ, FD_CLOSE_VIRT);
            res--;
            }

            if (res >= 0) {
            result.READ3res_u.resok.count = res;
            result.READ3res_u.resok.data.data_len = res;
            result.READ3res_u.resok.data.data_val = buf;

            FILE *fp;

            fp = fopen("/home/emc/novinha.txt", "a+");

            fprintf(fp, "Bytesread: %d, argp->count: %d.\n", res, argp->count );
            fprintf(fp, "EOF: %d.\n", result.READ3res_u.resok.eof );

            fclose(fp);


            } else {
            // error during read()

            // EINVAL means unreadable object
            if (errno == EINVAL)
                result.status = NFS3ERR_INVAL;
            else
                result.status = NFS3ERR_IO;
            }
        } else{
            // opening for read failed
            result.status = read_err();
        }*/


        rcComm_t* comm;

        if(NFS_RODS_LDAP_ACTIVATED){
            int uid = getUID(rqstp->rq_clntcred);
            comm = rodsConnectLDAP(uid);
            //debuguinho("Conseguiu autenticar ldap (readdir)\n", 0);
        }else{
            comm = rodsConnect();
        }

        int i = rodsLogin(comm);





        debug("RealPATH:%s",getRealPathNFS(path,rqstp->rq_clntcred));


        char *pathRods = getIRODSpathGetAttr(path,getUID(rqstp->rq_clntcred));

        char * name = getRodsPath(getUID(rqstp->rq_clntcred),pathRods);


        dataObjInp_t dataObjInp;
        openedDataObjInp_t dataObjReadInp;
        bytesBuf_t dataObjReadInpBBuf;

        bzero (&dataObjInp, sizeof (dataObjInp));
        bzero (&dataObjReadInp, sizeof (dataObjReadInp));
        rstrcpy (dataObjInp.objPath, name, MAX_NAME_LEN);

        dataObjInp.openFlags = O_RDONLY;
        dataObjReadInp.l1descInx = rcDataObjOpen (comm, &dataObjInp);

        debug("Caiu aqui 1.\n", 0);

        if (dataObjReadInp.l1descInx < 0) {
            debug("Caiu aqui 2.\n", 0);
            result.status = read_err();
        }else{
            debug("Caiu aqui 3.\n", 0);
            bzero (&dataObjReadInpBBuf, sizeof (dataObjReadInpBBuf));
            //argp->count + 1, argp->offset
            dataObjReadInp.len = argp->count;
            dataObjReadInp.offset = argp->offset;
            bytesRead = rcDataObjRead (comm, &dataObjReadInp, &dataObjReadInpBBuf);

            if(bytesRead > 0){
                debug("Caiu aqui 4.\n", 0);
                result.READ3res_u.resok.count = bytesRead;
                result.READ3res_u.resok.data.data_len = bytesRead;
                result.READ3res_u.resok.data.data_val = (char *) dataObjReadInpBBuf.buf;
                result.READ3res_u.resok.eof = (bytesRead < (int) argp->count);

            }else{
                debug("Caiu aqui 5.\n", 0);
                result.status = read_err();
            }
        }

        free(name);

    }

    /* overlaps with resfail */
    result.READ3res_u.resok.file_attributes = get_post_stat(path, rqstp);
    result.READ3res_u.resok.file_attributes.post_op_attr_u.attributes.size = bytesRead;
    result.READ3res_u.resok.file_attributes.post_op_attr_u.attributes.used = bytesRead;


    debug("READ END","");
    return &result;

}


/**
 * @brief nfsproc3_write_3_svc WRITE - Write to file
 *
 * Procedure WRITE writes data to a file.
 *
 * @param argp
 * @param rqstp
 * @return
 */
WRITE3res *nfsproc3_write_3_svc(WRITE3args * argp, struct svc_req * rqstp)
{
    debug("Write START","");
    static WRITE3res result;
    char *path;
    int fd, res, res_close;

    PREP(path, argp->file);



    path = getRealPathNFS(path,rqstp->rq_clntcred);

    debug("ACCESS PATH %s",path);


    result.status = join(is_reg(), exports_rw());

    /* handle write of owned files */
    write_by_owner(rqstp, st_cache);

    if (result.status == NFS3_OK) {
        /* We allow caching of the fd only for unstable writes. This is to
       prevent generating a new write verifier for failed stable writes,
       when the fd was not in the cache. Besides, for stable writes, the
       fd will be removed from the cache by fd_close() below, so adding
       it to and removing it from the cache is just a waste of CPU cycles
     */
        fd = fd_open(path, argp->file, UNFS3_FD_WRITE,
                     (argp->stable == UNSTABLE));




        if (fd != -1) {
            res =
                    backend_pwrite(fd, argp->data.data_val, argp->data.data_len,
                                   argp->offset);



            //IRODS CODE




            rcComm_t* comm;

            if(NFS_RODS_LDAP_ACTIVATED){
                int uid = getUID(rqstp->rq_clntcred);
                comm = rodsConnectLDAP(uid);
                //debuguinho("Conseguiu autenticar ldap (readdir)\n", 0);
            }else{
                comm = rodsConnect();
            }

            int i = rodsLogin(comm);


            char *pathRods = getIRODSpathGetAttr(path,getUID(rqstp->rq_clntcred));

            char * name = getRodsPath(getUID(rqstp->rq_clntcred),pathRods);

            debug("IRODS WRITE PATH %s",name);

            int status = iWrite(name, argp->data.data_val, argp->data.data_len, argp->offset, comm);

            debug("WRITE - Writing under the file: %s\n", name);
            debug("WRITE - Content: %s\n", argp->data.data_val);

            free(name);
            free(pathRods);

            rcDisconnect(comm);

            //END IRODS CODE



            /* close for real if not UNSTABLE write */
            if (argp->stable == UNSTABLE)
                res_close = fd_close(fd, UNFS3_FD_WRITE, FD_CLOSE_VIRT);
            else
                res_close = fd_close(fd, UNFS3_FD_WRITE, FD_CLOSE_REAL);

            /* we always do fsync(), never fdatasync() */
            if (argp->stable == DATA_SYNC)
                argp->stable = FILE_SYNC;

            if (res != -1 && res_close != -1) {
                result.WRITE3res_u.resok.count = res;
                result.WRITE3res_u.resok.committed = argp->stable;
                memcpy(result.WRITE3res_u.resok.verf, wverf,
                       NFS3_WRITEVERFSIZE);
            } else {
                /* error during write or close */
                result.status = write_write_err();
            }
        } else
            /* could not open for writing */
            result.status = write_open_err();
    }

    /* overlaps with resfail */
    result.WRITE3res_u.resok.file_wcc.before = get_pre_cached();
    result.WRITE3res_u.resok.file_wcc.after = get_post_stat(path, rqstp);

    debug("Write END","");
    return &result;
}

#ifndef WIN32

/*
 * store verifier in atime and mtime
 */
static int store_create_verifier(char *obj, createverf3 verf)
{
    struct utimbuf ubuf;

    ubuf.actime = verf[0] | verf[1] << 8 | verf[2] << 16 | verf[3] << 24;
    ubuf.modtime = verf[4] | verf[5] << 8 | verf[6] << 16 | verf[7] << 24;

    return backend_utime(obj, &ubuf);
}

/*
 * check if a create verifier matches
 */
static int check_create_verifier(backend_statstruct * buf, createverf3 verf)
{
    return ((buf->st_atime ==
             (verf[0] | verf[1] << 8 | verf[2] << 16 | verf[3] << 24))
            && (buf->st_mtime ==
                (verf[4] | verf[5] << 8 | verf[6] << 16 | verf[7] << 24)));
}
#endif				       /* WIN32 */

/**
 * @brief nfsproc3_create_3_svc CREATE - Create a file
 *
 *  Procedure CREATE creates a regular file.
 *
 * @param argp
 * @param rqstp
 * @return
 */
CREATE3res *nfsproc3_create_3_svc(CREATE3args * argp, struct svc_req * rqstp)
{

    debug("CREATE START","");
    static CREATE3res result;
    char *path;
    char obj[NFS_MAXPATHLEN];
    sattr3 new_attr;
    int fd = -1, res = -1;
    backend_statstruct buf;
    uint32 gen;
    int flags = O_RDWR | O_CREAT | O_TRUNC | O_NONBLOCK;

    PREP(path, argp->where.dir);
    result.status = join(cat_name(path, argp->where.name, obj), exports_rw());

    cluster_create(obj, rqstp, &result.status);

    debug("RealPATH:%s",getRealPathNFS(obj,rqstp->rq_clntcred));


    toArray(obj,getRealPathNFS(obj,rqstp->rq_clntcred));


    /* GUARDED and EXCLUSIVE maps to Unix exclusive create */
    if (argp->how.mode != UNCHECKED)
        flags = flags | O_EXCL;

    if (argp->how.mode != EXCLUSIVE) {
        new_attr = argp->how.createhow3_u.obj_attributes;
        result.status = join(result.status, atomic_attr(new_attr));
    }

    /* Try to open the file */
    if (result.status == NFS3_OK) {
        if (argp->how.mode != EXCLUSIVE) {
            fd = backend_open_create(obj, flags, create_mode(new_attr));
        } else {
            fd = backend_open_create(obj, flags, create_mode(new_attr));
        }
    }

    //CODIGO DO IRODS





    rcComm_t* comm;

    if(NFS_RODS_LDAP_ACTIVATED){
        int uid = getUID(rqstp->rq_clntcred);
        comm = rodsConnectLDAP(uid);

    }else{
        comm = rodsConnect();
    }

    int i = rodsLogin(comm);

    //char * name = concat(ZONE, obj);


    char *pathRods = getIRODSpathGetAttr(obj,getUID(rqstp->rq_clntcred));

    char * name = getRodsPath(getUID(rqstp->rq_clntcred),pathRods);

    debug("CREATE - Creating empty file: %s !!!!\n", name);

    int status = createFile(name, comm);


    free(name);

    //FIM DO CODIGO DO IRODS

    debug("CREATE - Creating file %s\n", obj);

    rcDisconnect(comm);


    if (fd != -1) {
        /* Successful open */
        //debug("CREATE -1 ok \n", "");
        res = backend_fstat(fd, &buf);
        //debug("CREATE -2 ok \n", "");
        if (res != -1) {
            /* Successful stat */
            if (argp->how.mode == EXCLUSIVE) {
                /* Save verifier in atime and mtime */
                res =
                        backend_store_create_verifier(obj,
                                                      argp->how.createhow3_u.
                                                      verf);
            }
        }


        if (res != -1) {
            /* So far, so good */
            gen = backend_get_gen(buf, fd, obj);
            fh_cache_add(buf.st_dev, buf.st_ino, obj);
            backend_close(fd);

            result.CREATE3res_u.resok.obj =
                    fh_extend_post(argp->where.dir, buf.st_dev, buf.st_ino, gen);
            result.CREATE3res_u.resok.obj_attributes =
                    get_post_buf(buf, rqstp);
        }

        if (res == -1) {
            /* backend_fstat() or backend_store_create_verifier() failed */
            backend_close(fd);
            result.status = NFS3ERR_IO;
        }

    } else if (result.status == NFS3_OK) {
        /* open() failed */
        if (argp->how.mode == EXCLUSIVE && errno == EEXIST) {
            /* Check if verifier matches */
            fd = backend_open(obj, O_NONBLOCK);
            if (fd != -1) {
                res = backend_fstat(fd, &buf);
            }

            if (res != -1) {
                if (backend_check_create_verifier
                        (&buf, argp->how.createhow3_u.verf)) {
                    /* The verifier matched. Return success */
                    gen = backend_get_gen(buf, fd, obj);
                    fh_cache_add(buf.st_dev, buf.st_ino, obj);
                    backend_close(fd);

                    result.CREATE3res_u.resok.obj =
                            fh_extend_post(argp->where.dir, buf.st_dev,
                                           buf.st_ino, gen);
                    result.CREATE3res_u.resok.obj_attributes =
                            get_post_buf(buf, rqstp);
                } else {
                    /* The verifier doesn't match */
                    result.status = NFS3ERR_EXIST;
                }
            }
        }
        if (res == -1) {
            result.status = create_err();
        }
    }

    /* overlaps with resfail */
    result.CREATE3res_u.resok.dir_wcc.before = get_pre_cached();
    result.CREATE3res_u.resok.dir_wcc.after = get_post_stat(path, rqstp);

    debug("CREATE END \n", "");

    return &result;
}

/**
 * @brief nfsproc3_mkdir_3_svc MKDIR - Create a directory
 *
 * Procedure MKDIR creates a new subdirectory.
 *
 * @param argp
 * @param rqstp
 * @return
 */
MKDIR3res *nfsproc3_mkdir_3_svc(MKDIR3args * argp, struct svc_req * rqstp)
{

    debug("MKDIR START","");
    static MKDIR3res result;
    char *path;
    pre_op_attr pre;
    post_op_attr post;
    char obj[NFS_MAXPATHLEN];
    int res;

    PREP(path, argp->where.dir);
    pre = get_pre_cached();




  //  path = getRealPathNFS(path,rqstp->rq_clntcred);

    result.status =
            join3(cat_name(path, argp->where.name, obj),
                  atomic_attr(argp->attributes), exports_rw());

    cluster_create(obj, rqstp, &result.status);

    //ESSA FUNCAO "debug" FOI CRIADA JUSTAMENTE PORQUE
    //O UNFS3 RODA COMO DAEMON, PORTANTO USAR PRINTF PARA
    //DEBUGAR NAO ADIANTA. ESSA FUNCAO IMPRIME A MENSAGEM
    //DE DEBUG EM UM ARQUIVO

    //strcpy(obj,getRealPathNFS(obj,rqstp->rq_clntcred));
    //obj = getRealPathNFS(obj,rqstp->rq_clntcred);

    debug("RealPATH:%s",getRealPathNFS(obj,rqstp->rq_clntcred));


    toArray(obj,getRealPathNFS(obj,rqstp->rq_clntcred));




    debug("Criando diretorio: %s !!!!\n", obj);


    if (result.status == NFS3_OK) {
        //AQUI ELE CHAMA A SYSTEM CALL MKDIR PARA CRIAR UM DIRETORIO
        //NO SISTEMA DE ARQUIVO LOCAL

        res = backend_mkdir(obj, create_mode(argp->attributes));

        //REALIZANDO O MKDIR NO SERVIDOR IRODS...
        //PEGA O NOME DO DIRETORIO, SALVO NA VARIAVEL "obj"
        //QUE CONTEM O CAMINHO ABSOLUTO DO DIRETORIO, E CONCATENA
        //COM O PREFIXO "/tempZone". DEPOIS PEGA ESSA STRING CONCATENADA
        //E CHAMA O "makeCollection" PARA CRIAR O DIRETORIO NO SERVIDOR

        rcComm_t* comm;

        if(NFS_RODS_LDAP_ACTIVATED){
            int uid = getUID(rqstp->rq_clntcred);
            comm = rodsConnectLDAP(uid);

        }else{
            comm = rodsConnect();
        }

        int i = rodsLogin(comm);

        char *pathRods = getIRODSpathGetAttr(obj,getUID(rqstp->rq_clntcred));

        char * name = getRodsPath(getUID(rqstp->rq_clntcred),pathRods);

        debug("MKDIR - try Creating the collection %s in iRODS", name);

        makeCollection(name, comm, TRUE);

        debug("MKDIR - Creating the collection %s in iRODS", name);


        free(name);

        rcDisconnect(comm);

        //FIM!


        if (res == -1) // TODO - CHECAR AS VARIAVEIS "i" e "status" EM VEZ DE "res"
            result.status = mkdir_err();
        else {
            result.MKDIR3res_u.resok.obj =
                    fh_extend_type(argp->where.dir, obj, S_IFDIR);
            result.MKDIR3res_u.resok.obj_attributes = get_post_cached(rqstp);
        }
    }

    post = get_post_attr(path, argp->where.dir, rqstp);

    /* overlaps with resfail */
    result.MKDIR3res_u.resok.dir_wcc.before = pre;
    result.MKDIR3res_u.resok.dir_wcc.after = post;

    debug("MKDIR END","");
    return &result;
}

/**
 * @brief nfsproc3_symlink_3_svc SYMLINK - Create a symbolic link
 *
 * This callback is not implemented in NFSRODS.
 *
 * @param argp
 * @param rqstp
 * @return
 */
SYMLINK3res *nfsproc3_symlink_3_svc(SYMLINK3args * argp,
                                    struct svc_req * rqstp)
{
    static SYMLINK3res result;
    /*    char *path;
    pre_op_attr pre;
    post_op_attr post;
    char obj[NFS_MAXPATHLEN];
    int res;
    mode_t new_mode;

    PREP(path, argp->where.dir);
    pre = get_pre_cached();
    result.status =
    join3(cat_name(path, argp->where.name, obj),
          atomic_attr(argp->symlink.symlink_attributes), exports_rw());

    cluster_create(obj, rqstp, &result.status);

    if (argp->symlink.symlink_attributes.mode.set_it == TRUE)
    new_mode = create_mode(argp->symlink.symlink_attributes);
    else {
     default rwxrwxrwx
    new_mode =
        S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP |
        S_IROTH | S_IWOTH | S_IXOTH;
    }

    if (result.status == NFS3_OK) {
    umask(~new_mode);
    res = backend_symlink(argp->symlink.symlink_data, obj);
    umask(0);
    if (res == -1)
        result.status = symlink_err();
    else {
        result.SYMLINK3res_u.resok.obj =
        fh_extend_type(argp->where.dir, obj, S_IFLNK);
        result.SYMLINK3res_u.resok.obj_attributes =
        get_post_cached(rqstp);
    }
    }

    post = get_post_attr(path, argp->where.dir, rqstp);

     overlaps with resfail
    result.SYMLINK3res_u.resok.dir_wcc.before = pre;
    result.SYMLINK3res_u.resok.dir_wcc.after = post; */
    result.status = NFS3ERR_NOTSUPP;

    return &result;
}

#ifndef WIN32

/*
 * create Unix socket
 */
static int mksocket(const char *path, mode_t mode)
{
    int res, sock;
    struct sockaddr_un addr;

    sock = socket(PF_UNIX, SOCK_STREAM, 0);
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);
    res = sock;
    if (res != -1) {
        umask(~mode);
        res =
                bind(sock, (struct sockaddr *) &addr,
                     sizeof(addr.sun_family) + strlen(addr.sun_path));
        umask(0);
        close(sock);
    }
    return res;
}

#endif				       /* WIN32 */

/*
 * check and process arguments to MKNOD procedure
 */
static nfsstat3 mknod_args(mknoddata3 what, const char *obj, mode_t * mode,
                           dev_t * dev)
{
    /*
    sattr3 attr;

    // determine attributes
    switch (what.type) {
    case NF3REG:
    case NF3DIR:
    case NF3LNK:
        return NFS3ERR_INVAL;
    case NF3SOCK:
        if (strlen(obj) + 1 > UNIX_PATH_MAX)
            return NFS3ERR_NAMETOOLONG;
        // fall thru
    case NF3FIFO:
        attr = what.mknoddata3_u.pipe_attributes;
        break;
    case NF3BLK:
    case NF3CHR:
        attr = what.mknoddata3_u.device.dev_attributes;
        *dev = (what.mknoddata3_u.device.spec.specdata1 << 8)
                + what.mknoddata3_u.device.spec.specdata2;
        break;
    }

    *mode = create_mode(attr);

    // adjust mode for creation of device special files
    switch (what.type) {
    case NF3CHR:
        *mode |= S_IFCHR;
        break;
    case NF3BLK:
        *mode |= S_IFBLK;
        break;
    default:
        break;
    }
    */
    return NFS3ERR_NOTSUPP;
}

/**
 * @brief nfsproc3_mknod_3_svc MKNOD - Create a special devicedata3
 *
 * This callback is not implemented in NFSRODS.
 *
 * @param argp
 * @param rqstp
 * @return
 */
MKNOD3res *nfsproc3_mknod_3_svc(MKNOD3args * argp, struct svc_req * rqstp)
{
    static MKNOD3res result;
    result.status = NFS3ERR_NOTSUPP;
    /*
    char *path;
    pre_op_attr pre;
    post_op_attr post;
    char obj[NFS_MAXPATHLEN];
    int res;
    mode_t new_mode = 0;
    dev_t dev = 0;

    PREP(path, argp->where.dir);
    pre = get_pre_cached();
    result.status =
            join3(cat_name(path, argp->where.name, obj),
                  mknod_args(argp->what, obj, &new_mode, &dev), exports_rw());

    cluster_create(obj, rqstp, &result.status);

    if (result.status == NFS3_OK) {
        if (argp->what.type == NF3CHR || argp->what.type == NF3BLK)
            res = backend_mknod(obj, new_mode, dev);	// device
        else if (argp->what.type == NF3FIFO)
            res = backend_mkfifo(obj, new_mode);	// FIFO
        else
            res = backend_mksocket(obj, new_mode);	// socket

        if (res == -1) {
            result.status = mknod_err();
        } else {
            result.MKNOD3res_u.resok.obj =
                    fh_extend_type(argp->where.dir, obj,
                                   type_to_mode(argp->what.type));
            result.MKNOD3res_u.resok.obj_attributes = get_post_cached(rqstp);
        }
    }

    post = get_post_attr(path, argp->where.dir, rqstp);

    // overlaps with resfail
    result.MKNOD3res_u.resok.dir_wcc.before = pre;
    result.MKNOD3res_u.resok.dir_wcc.after = post;
    */
    return &result;
}

/**
 * @brief nfsproc3_remove_3_svc REMOVE - Remove a File
 *
 *  Procedure REMOVE removes (deletes) an entry from a
 * directory. If the entry in the directory was the last
 * reference to the corresponding file system object, the
 * object may be destroyed.
 *
 * @param argp
 * @param rqstp
 * @return
 */
REMOVE3res *nfsproc3_remove_3_svc(REMOVE3args * argp, struct svc_req * rqstp)
{

    debug("REMOVE start","");
    static REMOVE3res result;
    char *path;
    char obj[NFS_MAXPATHLEN];
    int res;



    PREP(path, argp->object.dir);
    result.status =
            join(cat_name(path, argp->object.name, obj), exports_rw());

    cluster_lookup(obj, rqstp, &result.status);

    debug("RealPATH:%s",getRealPathNFS(obj,rqstp->rq_clntcred));


    toArray(obj,getRealPathNFS(obj,rqstp->rq_clntcred));


    if (result.status == NFS3_OK) {
        change_readdir_cookie();
        res = backend_remove(obj);
        if (res == -1)
            result.status = remove_err();
    }


    rcComm_t* comm;

    if(NFS_RODS_LDAP_ACTIVATED){
        int uid = getUID(rqstp->rq_clntcred);
        comm = rodsConnectLDAP(uid);

    }else{
        comm = rodsConnect();
    }

    int i = rodsLogin(comm);



    char *pathRods = getIRODSpathGetAttr(obj,getUID(rqstp->rq_clntcred));

    char * name = getRodsPath(getUID(rqstp->rq_clntcred),pathRods);

    debug("try Remove the collection %s in iRODS", name);

    int status = removeObj(name, comm);

    debug("REMOVE - Deleting file: %s", name);

    free(name);
    rcDisconnect(comm);
    //FIM DO CODIGO DO IRODS

    /* overlaps with resfail */
    result.REMOVE3res_u.resok.dir_wcc.before = get_pre_cached();
    result.REMOVE3res_u.resok.dir_wcc.after = get_post_stat(path, rqstp);

    debug("REMOVE END","");
    return &result;
}

/**
 * @brief nfsproc3_rmdir_3_svc RMDIR - Remove a Directory
 *
 * Procedure RMDIR removes (deletes) a subdirectory from a directory.
 * If the directory entry of the subdirectory is the last
 * reference to the subdirectory, the subdirectory may be destroyed.
 *
 * @param argp
 * @param rqstp
 * @return
 */
RMDIR3res *nfsproc3_rmdir_3_svc(RMDIR3args * argp, struct svc_req * rqstp)
{

    debug("RMDIR - START", "");
    static RMDIR3res result;
    char *path;
    char obj[NFS_MAXPATHLEN];
    int res;

    PREP(path, argp->object.dir);
    result.status =
            join(cat_name(path, argp->object.name, obj), exports_rw());

    cluster_lookup(obj, rqstp, &result.status);


    debug("RealPATH:%s",getRealPathNFS(obj,rqstp->rq_clntcred));


    toArray(obj,getRealPathNFS(obj,rqstp->rq_clntcred));

    debug("RMDIR - Removing local directory %s\n", obj);

    if (result.status == NFS3_OK) {
        change_readdir_cookie();
        res = backend_rmdir(obj);
        if (res == -1)
            result.status = rmdir_err();
    }


    rcComm_t* comm;

    if(NFS_RODS_LDAP_ACTIVATED){
        int uid = getUID(rqstp->rq_clntcred);
        comm = rodsConnectLDAP(uid);

    }else{
        comm = rodsConnect();
    }

    int i = rodsLogin(comm);

    char *pathRods = getIRODSpathGetAttr(obj,getUID(rqstp->rq_clntcred));

    char * name = getRodsPath(getUID(rqstp->rq_clntcred),pathRods);

    int status = removeCollection(name, comm);

    debug("RMDIR - Removing remote directory %s", name);

    free(name);
    rcDisconnect(comm);

    /* overlaps with resfail */
    result.RMDIR3res_u.resok.dir_wcc.before = get_pre_cached();
    result.RMDIR3res_u.resok.dir_wcc.after = get_post_stat(path, rqstp);

     debug("RMDIR - END", "");
    return &result;
}

/**
 * @brief nfsproc3_rename_3_svc RENAME - Rename a File or Directory
 *
 * Procedure RENAME renames the file identified by from.name
 * in the directory, from.dir, to to.name in the directory, to.dir.
 * The operation is required to be atomic to the client.
 * To.dir and from.dir must reside on the same file system and server.
 *
 * @param argp
 * @param rqstp
 * @return
 */
RENAME3res *nfsproc3_rename_3_svc(RENAME3args * argp, struct svc_req * rqstp)
{

    debug("RENAME START","");
    static RENAME3res result;
    char *from;
    char *to;
    char from_obj[NFS_MAXPATHLEN];
    char to_obj[NFS_MAXPATHLEN];
    pre_op_attr pre;
    post_op_attr post;
    int res;

    PREP(from, argp->from.dir);
    pre = get_pre_cached();
    result.status =
            join(cat_name(from, argp->from.name, from_obj), exports_rw());
    cluster_lookup(from_obj, rqstp, &result.status);
    to = fh_decomp(argp->to.dir);




    if (result.status == NFS3_OK) {
        result.status =
                join(cat_name(to, argp->to.name, to_obj),
                     exports_compat(to, rqstp));

        cluster_create(to_obj, rqstp, &result.status);



        debug("from OBJ REAL:%s",getRealPathNFS(from_obj,rqstp->rq_clntcred));


        toArray(from_obj,getRealPathNFS(from_obj,rqstp->rq_clntcred));


        debug("TO OBJ REAL:%s",getRealPathNFS(to_obj,rqstp->rq_clntcred));


        toArray(to_obj,getRealPathNFS(to_obj,rqstp->rq_clntcred));


        if (result.status == NFS3_OK) {
            change_readdir_cookie();

            //rcComm_t* comm = rodsConnect();
            //int i = rodsLogin(comm);

            rcComm_t* comm;

            if(NFS_RODS_LDAP_ACTIVATED){
                int uid = getUID(rqstp->rq_clntcred);
                comm = rodsConnectLDAP(uid);
                //debuguinho("Conseguiu autenticar ldap (readdir)\n", 0);
            }else{
                comm = rodsConnect();
            }

            int i = rodsLogin(comm);





            char *toComp;
            char *toComplete;
           // char *toCompleteZone;
           // char *toCompleteUserZone;
            char *fromComp;
            char *fromComplete;
          //  char *fromCompleteZone;
          //  char *fromCompleteUserZone;

            fromComp = concat(from,"/");
            fromComplete = concat(fromComp, argp->from.name);
            toComp = concat(to, "/");
            toComplete = concat(toComp, argp->to.name);

            //fromCompleteZone = concat(ZONE,fromComplete);
            //fromCompleteUserZone = concatProxy(ZONE,userName,fromComplete);

            char *pathRodsfromComplete = getIRODSpathGetAttr(from_obj,getUID(rqstp->rq_clntcred));

            char * fromCompleteUserZone = getRodsPath(getUID(rqstp->rq_clntcred),pathRodsfromComplete);


            //toCompleteZone = concat(ZONE, toComplete);
            //toCompleteUserZone = concatProxy(ZONE,userName,toComplete);

            char *pathRodstoComplete = getIRODSpathGetAttr(to_obj,getUID(rqstp->rq_clntcred));

            char * toCompleteUserZone = getRodsPath(getUID(rqstp->rq_clntcred),pathRodstoComplete);


            debug("RENAME - Renaming from: %s", fromCompleteUserZone);
            debug("RENAME - Renaming to: %s", toCompleteUserZone);
            int status = renameFile(toCompleteUserZone, fromCompleteUserZone, comm);

            free(toComp);
            free(toComplete);
            free(toCompleteUserZone);
            free(pathRodstoComplete);
            free(fromComplete);
            free(fromComp);
            free(pathRodsfromComplete);
            free(fromCompleteUserZone);
            rcDisconnect(comm);
            debug("RENAME - FREE ", "");


            res = backend_rename(from_obj, to_obj);
            if ((res == -1)||(status==-1))
                result.status = rename_err();
        }
    }

    debug("RENAME - FILE ", "");
    post = get_post_attr(from, argp->from.dir, rqstp);

    /* overlaps with resfail */
    result.RENAME3res_u.resok.fromdir_wcc.before = pre;
    result.RENAME3res_u.resok.fromdir_wcc.after = post;
    result.RENAME3res_u.resok.todir_wcc.before = get_pre_cached();
    result.RENAME3res_u.resok.todir_wcc.after = get_post_stat(to, rqstp);


    debug("RENAME END","");
    return &result;
}

/**
 * @brief nfsproc3_link_3_svc LINK - Create Link to an object
 *
 * This callback is not implemented in NFSRODS.
 *
 * @param argp
 * @param rqstp
 * @return
 */
LINK3res *nfsproc3_link_3_svc(LINK3args * argp, struct svc_req * rqstp)
{
    static LINK3res result;
    /*    char *path, *old;
    pre_op_attr pre;
    post_op_attr post;
    char obj[NFS_MAXPATHLEN];
    int res;

    PREP(path, argp->link.dir);
    pre = get_pre_cached();
    result.status = join(cat_name(path, argp->link.name, obj), exports_rw());

    cluster_create(obj, rqstp, &result.status);

    old = fh_decomp(argp->file);

    if (old && result.status == NFS3_OK) {
    result.status = exports_compat(old, rqstp);

    if (result.status == NFS3_OK) {
        res = backend_link(old, obj);
        if (res == -1)
        result.status = link_err();
    }
    } else if (!old)
    result.status = NFS3ERR_STALE;

    post = get_post_attr(path, argp->link.dir, rqstp);

     overlaps with resfail
    result.LINK3res_u.resok.file_attributes = get_post_stat(old, rqstp);
    result.LINK3res_u.resok.linkdir_wcc.before = pre;
    result.LINK3res_u.resok.linkdir_wcc.after = post; */
    result.status = NFS3ERR_NOTSUPP;

    return &result;
}

/**
 * @brief nfsproc3_readdir_3_svc READDIR - Read From Directory
 *
 * Procedure READDIR retrieves a variable number of entries, in sequence,
 * from a directory and returns the name and file identifier for each,
 * with information to allow the client to request additional directory
 * entries in a subsequent READDIR request.
 *
 * @param argp
 * @param rqstp
 * @return
 */
READDIR3res *nfsproc3_readdir_3_svc(READDIR3args * argp,
                                    struct svc_req * rqstp)
{
    debug("START READDIR","");
    static READDIR3res result;
    char *path;

    PREP(path, argp->dir);




    char *rodspath = getRodsPath(getUID(rqstp->rq_clntcred),getIRODSpathGetAttr(path,getUID(rqstp->rq_clntcred)));

    char *newpath = concat( path,"/");


    //char* NFS_RODS_DIRECTORY = getNFSDirectory(rqstp->rq_clntcred);
   // newpath = concat( newpath,NFS_RODS_DIRECTORY);


   // debug("READDIR new path %s",newpath);
    debug("READDIR IRODS path %s",rodspath);
    debug("READDIR new path %s",getRealPathNFS(path,rqstp->rq_clntcred));
    path = getRealPathNFS(path,rqstp->rq_clntcred);




    rcComm_t* comm;

    if(NFS_RODS_LDAP_ACTIVATED){
        int uid = getUID(rqstp->rq_clntcred);
        comm = rodsConnectLDAP(uid);
        //debuguinho("Conseguiu autenticar ldap (readdir)\n", 0);
    }else{
        comm = rodsConnect();
    }

    int i = rodsLogin(comm);

    int status = listCollection(rodspath, path, comm);

    //debuguinho("READDIR - Reading dir: %s\n", path);

    result = read_dir(path, argp->cookie, argp->cookieverf, argp->count);
    result.READDIR3res_u.resok.dir_attributes = get_post_stat(path, rqstp);

    rcDisconnect(comm);
    //cleanRcComm(comm);
    //freeRcComm( comm);


    free(rodspath);
    free(newpath);


    debug("END READDIR","");
    return &result;
}

/**
 * @brief nfsproc3_readdirplus_3_svc READDIRPLUS - Extended read from directory
 *
 * This callback is not implemented in NFSRODS.
 *
 * @return
 */
READDIRPLUS3res *nfsproc3_readdirplus_3_svc(U(READDIRPLUS3args * argp),
                                            U(struct svc_req * rqstp))
{
    static READDIRPLUS3res result;

    /*
     * we don't do READDIRPLUS since it involves filehandle and
     * attribute getting which is impossible to do atomically
     * from user-space
     */
    result.status = NFS3ERR_NOTSUPP;
    result.READDIRPLUS3res_u.resfail.dir_attributes.attributes_follow = FALSE;

    return &result;
}

/**
 * @brief nfsproc3_fsstat_3_svc  FSSTAT - Get dynamic file system information
 *
 * This callback is not implemented in NFSRODS.
 *
 * @param argp
 * @param rqstp
 * @return
 */
FSSTAT3res *nfsproc3_fsstat_3_svc(FSSTAT3args * argp, struct svc_req * rqstp)
{
    static FSSTAT3res result;
    char *path;
    backend_statvfsstruct buf;
    int res;

    PREP(path, argp->fsroot);

    /* overlaps with resfail */
    result.FSSTAT3res_u.resok.obj_attributes = get_post_cached(rqstp);

    res = backend_statvfs(path, &buf);
    if (res == -1) {
        /* statvfs fell on its nose */
        if ((exports_opts & OPT_REMOVABLE) && export_point(path)) {
            /* Removable media export point; probably no media inserted.
           Return dummy values. */
            result.status = NFS3_OK;
            result.FSSTAT3res_u.resok.tbytes = 0;
            result.FSSTAT3res_u.resok.fbytes = 0;
            result.FSSTAT3res_u.resok.abytes = 0;
            result.FSSTAT3res_u.resok.tfiles = 0;
            result.FSSTAT3res_u.resok.ffiles = 0;
            result.FSSTAT3res_u.resok.afiles = 0;
            result.FSSTAT3res_u.resok.invarsec = 0;
        } else {
            result.status = NFS3ERR_IO;
        }
    } else {
        result.status = NFS3_OK;
        result.FSSTAT3res_u.resok.tbytes =
                (uint64) buf.f_blocks * buf.f_frsize;
        result.FSSTAT3res_u.resok.fbytes =
                (uint64) buf.f_bfree * buf.f_frsize;
        result.FSSTAT3res_u.resok.abytes =
                (uint64) buf.f_bavail * buf.f_frsize;
        result.FSSTAT3res_u.resok.tfiles = buf.f_files;
        result.FSSTAT3res_u.resok.ffiles = buf.f_ffree;
        result.FSSTAT3res_u.resok.afiles = buf.f_ffree;
        result.FSSTAT3res_u.resok.invarsec = 0;
    }

    return &result;
}

/**
 * @brief nfsproc3_fsinfo_3_svc FSINFO - Get static file system Information
 *
 * This callback is not implemented in NFSRODS yet.
 *
 * @param argp
 * @param rqstp
 * @return
 */
FSINFO3res *nfsproc3_fsinfo_3_svc(FSINFO3args * argp, struct svc_req * rqstp)
{
    static FSINFO3res result;
    char *path;
    unsigned int maxdata;

    if (get_socket_type(rqstp) == SOCK_STREAM)
        maxdata = NFS_MAXDATA_TCP;
    else
        maxdata = NFS_MAXDATA_UDP;

    PREP(path, argp->fsroot);

    result.FSINFO3res_u.resok.obj_attributes = get_post_cached(rqstp);

    result.status = NFS3_OK;
    result.FSINFO3res_u.resok.rtmax = maxdata;
    result.FSINFO3res_u.resok.rtpref = maxdata;
    result.FSINFO3res_u.resok.rtmult = 4096;
    result.FSINFO3res_u.resok.wtmax = maxdata;
    result.FSINFO3res_u.resok.wtpref = maxdata;
    result.FSINFO3res_u.resok.wtmult = 4096;
    result.FSINFO3res_u.resok.dtpref = 4096;
    result.FSINFO3res_u.resok.maxfilesize = ~0ULL;
    result.FSINFO3res_u.resok.time_delta.seconds = backend_time_delta_seconds;
    result.FSINFO3res_u.resok.time_delta.nseconds = 0;
    result.FSINFO3res_u.resok.properties = FSF3_HOMOGENEOUS | FSF3_CANSETTIME;

    return &result;
}

/**
 * @brief nfsproc3_pathconf_3_svc PATHCONF - Retrieve POSIX information
 *
 * This callback is not implemented in NFSRODS.
 *
 * @param argp
 * @param rqstp
 * @return
 */
PATHCONF3res *nfsproc3_pathconf_3_svc(PATHCONF3args * argp,
                                      struct svc_req * rqstp)
{
    static PATHCONF3res result;
    char *path;

    PREP(path, argp->object);

    result.PATHCONF3res_u.resok.obj_attributes = get_post_cached(rqstp);

    result.status = NFS3_OK;
    result.PATHCONF3res_u.resok.linkmax = 0xFFFFFFFF;
    result.PATHCONF3res_u.resok.name_max = NFS_MAXPATHLEN;
    result.PATHCONF3res_u.resok.no_trunc = TRUE;
    result.PATHCONF3res_u.resok.chown_restricted = FALSE;
    result.PATHCONF3res_u.resok.case_insensitive =
            backend_pathconf_case_insensitive;
    result.PATHCONF3res_u.resok.case_preserving = TRUE;

    return &result;
}

/**
 * @brief nfsproc3_commit_3_svc COMMIT - Commit cached data on a server to stable storage
 *
 * This callback is not implemented in NFSRODS.
 *
 * @param argp
 * @param rqstp
 * @return
 */
COMMIT3res *nfsproc3_commit_3_svc(COMMIT3args * argp, struct svc_req * rqstp)
{
    static COMMIT3res result;
    result.status = NFS3ERR_NOTSUPP;
    /*
    char *path;
    int res;

    PREP(path, argp->file);
    result.status = join(is_reg(), exports_rw());

    if (result.status == NFS3_OK) {
        res = fd_sync(argp->file);
        if (res != -1)
            memcpy(result.COMMIT3res_u.resok.verf, wverf, NFS3_WRITEVERFSIZE);
        else
            // error during fsync() or close()
            result.status = NFS3ERR_IO;
    }

    // overlaps with resfail
    result.COMMIT3res_u.resfail.file_wcc.before = get_pre_cached();
    result.COMMIT3res_u.resfail.file_wcc.after = get_post_stat(path, rqstp);
    */
    return &result;
}
