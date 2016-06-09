/* NFS-RODS: A Tool for Accessing iRODS Repositories
 * via the NFS Protocol
 * (C) 2016, Danilo Mendonça, Vandi Alves, Iure Fe,
 * Aleciano Lobo Junior, Francisco Airton Silva,
 * Gustavo Callou and Paulo Maciel <prmm@cin.ufpe.br>
 */

#ifndef RODSCAPI_H
#define RODSCAPI_H

#include <rpc/types.h>

#include "rodsClient.h"
#include "rodsPath.h"
#include "utils/utils.h"
#include "utils/rodscapi_utils.h"
#include "nfs.h"
typedef int bool;

typedef struct
{
    int selectSize;
    int *selectsColumns;  // columns in Select SQL

    int condSize;
    int *condColumns; // Collumns in where SQL
    char **condValues; // Values in where SQL

}InputQuery;

typedef struct
{
    int size;
    int *resultCollumns; // Collumns in where SQL
    char **resultValues; // Values in where SQL
}OutputQuery;

int isPublic();
rcComm_t *  rodsConnect();
rcComm_t* rodsConnectProxy(char * clientUserName,char *zone);
int rodsLogin(rcComm_t * rodsCommPtr );
int listCollectionOld(char *collPath, rcComm_t *rodsCommPtr);
int makeCollection(char *collPath, rcComm_t * rodsCommPtr, bool makeRecursive);
int removeCollection(char *collPath, rcComm_t * rodsCommPtr);
int putFile(char *localPath, char *objPath, rcComm_t * rodsCommPtr);
int getFile(char *localPath, char *objPath, bool verifyChecksum,
            bool allowOverwrite, rcComm_t *rodsCommPtr);
int removeObj(char *objPath, rcComm_t *rodsCommPtr);
int createFile(char *objPath, rcComm_t * conn);
int renameFile(char *objPathDes, char *objPathSrc, rcComm_t *rodsCommPtr);
int iWrite(char *objPath, char* buff, int len, int offset, rcComm_t * rodsCommPtr);
int iGetatt(nfs_fh3 fh, fattr3 fat, char *path, int *size,int uidProxy);
void testRead(rcComm_t * conn);
void testWrite(rcComm_t * conn);
pre_op_attr get_pre_cached_irods(char *path,char** owner);
int changeAttr(char *path,char *userName,char * accessLevel,rcComm_t *conn);
int getFileInfo(char *path, rcComm_t * conn, rodsObjStat_t **rodsObjStatOut);
int listCollection(char *remoteFolder, char *localFolder, rcComm_t *rodsCommPtr);
rcComm_t* rodsConnecUser(int uidProxy);
rodsLong_t fileSize(char * objPath, rcComm_t * conn );
char* getRodsPath(int uidProxy,char* path);
int fileMode(char * path, rcComm_t * comm );


#endif // RODSCAPI_H


