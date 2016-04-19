#include "rodscapi.h"
#include <stdio.h>
#include <sys/stat.h>
#include "dataObjLseek.h"
#include "utils/rodscapi_utils.h"
#include "backend.h"

/**
 * @brief rodsConnect Opens a iRODS connection without LDAP authentication.
 *
 * This function authenticates in the iRODS environment by using
 * the credentials stored in ~/.irods/irods_environment.json file.
 *
 * @return a rcComm_t pointer.
 *
 */
rcComm_t* rodsConnect(){
    int status = 0;
    rodsEnv rodsUserEnv;
    rcComm_t *rodsCommPtr = 0;
    rErrMsg_t lastErrMsg;

    // get user iRODS environment
    if ((status = getRodsEnv(&rodsUserEnv)) < 0){
        return (NULL);
    }

    // rods api connect
    if ((rodsCommPtr = rcConnect(rodsUserEnv.rodsHost, rodsUserEnv.rodsPort,
                                 rodsUserEnv.rodsUserName, rodsUserEnv.rodsZone,
                                 0, &lastErrMsg)) == NULL)
    {
        return(NULL);
    }

    return (rodsCommPtr);
}


/**
 * @brief rodsConnectProxy Alguem d[a um helpis nessa fun;'ao
 *
 * @param clientUserName
 * @param zone The iRODS storage zone of the user
 *
 * @return a rcComm_t pointer.
 *
 */
rcComm_t* rodsConnectProxy(char * clientUserName,char *zone){
    int status = 0;
    rodsEnv rodsUserEnv;
    rcComm_t *rodsCommPtr = 0;
    rErrMsg_t errMsg;

    // get user iRODS environment
    if ((status = getRodsEnv(&rodsUserEnv)) < 0){
        return (NULL);
    }

    // rods api proxy connect
    if ((rodsCommPtr = _rcConnect(rodsUserEnv.rodsHost, rodsUserEnv.rodsPort,
                                 rodsUserEnv.rodsUserName, rodsUserEnv.rodsZone,clientUserName,zone,&errMsg,0,0 )) == NULL)
    {
        return(NULL);
    }

    return (rodsCommPtr);
}


/**
 * @brief rodsLogin Performs a login using a opened connection.
 * @param rodsCommPtr An opened connection
 * @return A status code.
 */
int rodsLogin(rcComm_t *rodsCommPtr)
{
    int status = 0;

    // first of all, we must be connected
    if (!rodsCommPtr)
        return (status);

    // try to authenticate client to the iRODS server
    if ((status = clientLogin(rodsCommPtr, 0, 0)) != 0)
    {
        rcDisconnect(rodsCommPtr);
    }

    return (status);
}


/**
 * @deprecated
 * @brief listCollectionOld Lists a collection. (DEPRECATED)
 * @param collPath
 * @param rodsCommPtr
 * @return
 */
int listCollectionOld(char *collPath, rcComm_t *rodsCommPtr)
{
    collHandle_t rodsColl;
    collEnt_t rodsCollEntry;
    char collPathOut[MAX_NAME_LEN];
    int status = 0;

    rodsEnv rodsUserEnv;

    // get user iRODS environment
    if ((status = getRodsEnv(&rodsUserEnv)) < 0){
        return (-1);
    }

    // first the path string must be parsed by iRODS
    if ((status = parseRodsPathStr(collPath, &rodsUserEnv, collPathOut) < 0))
        return (status);

    // try to open collection from iRODS
    if ((status = rclOpenCollection(rodsCommPtr, collPathOut, 0, &rodsColl)) < 0)
        return (status);

    // read collection while there are objects to loop over
    do {
        status = rclReadCollection(rodsCommPtr, &rodsColl, &rodsCollEntry);

        if (status >= 0)
        {
            printf("Collection: %s\n", rodsCollEntry.collName);
        }

    } while (status >= 0);

    // close the collection handle
    status = rclCloseCollection(&rodsColl);

    return (status);
}

/**
 * @brief listCollection Retrieves the contents of a folder in iRODS storage system.
 *
 * This function creates a local folder with the same directories, and creates empty
 * files with the same name, in order to help the NFS MKDIR callback to return its response.
 * This is a non-recursive function, and it must be called to each sub-folder individually.
 *
 * @param remoteFolder
 * @param localFolder
 * @param rodsCommPtr
 * @return
 */
int listCollection(char *remoteFolder, char *localFolder, rcComm_t *rodsCommPtr)
{
    removedirectoryrecursively(localFolder);


    debug("LIST COllection  %s",localFolder);
    struct stat s;
    int err = stat(localFolder, &s);
    if(-1 == err) {
        if(ENOENT == errno) {
            debug("Criar recursivo %s",localFolder);
            rec_mkdir(localFolder);
        } else {
            perror("stat");
            exit(1);
        }
    } else {
      //  if(S_ISDIR(s.st_mode)) {
            /* it's a dir */
        //} else {
            /* exists but is no dir */
        //}
    }

    int status, handleInx;
    collInp_t collOpenInp;
    collEnt_t *collEnt = NULL;
    bzero (&collOpenInp, sizeof (collOpenInp));
    rstrcpy (collOpenInp.collName, remoteFolder, MAX_NAME_LEN);
    collOpenInp.flags = VERY_LONG_METADATA_FG;
    handleInx = rcOpenCollection (rodsCommPtr, &collOpenInp);

    debug("rclOpenCollection OK","");
    char *path;
    char *path2;
    char *dir;
    char *command;

    debug("ListCollection RemoteFolder %s",remoteFolder);
    // read collection while there are objects to loop over
      while ((status = rcReadCollection (rodsCommPtr, handleInx, &collEnt)) >= 0) {

        if(collEnt->objType == DATA_OBJ_T){

            if(!collEnt->dataName){
                break;
            }

            path2 = concat(localFolder, "/");
            path = concat(path2, collEnt->dataName);
            command = concat("echo \" \" > ", path);

            system(command);

            char * pathIrods = concat(remoteFolder,"/");
            pathIrods = concat(pathIrods,collEnt->dataName);

            int dataMode = fileMode(pathIrods,rodsCommPtr);
            debug("data mode %d",dataMode);


            if(dataMode==1200){
              //own
                command="chmod 600 ";
            }else if(dataMode==1120){
                //write
                command="chmod 200 ";
            }else if(dataMode==1050){
                //read
                command="chmod 400 ";
            }else{
                command="chmod 000 ";

            }

            command = concat(command, path);

            system(command);


            /*
            command = concat("chown ", rodsCommPtr->clientUser.userName);
            command = concat(command, ":");
            command = concat(command, rodsCommPtr->clientUser.userName);
            command = concat(command, "  ");
            command = concat(command, path);
            */

            system(command);



            debug("ReadDir Create %s",path);






            free(path);
            free(path2);
            free(command);
            free(collEnt);
        }else{

            if(strlen(collEnt->collName)==strlen(remoteFolder))
                continue;


            if(!collEnt->collName){
                break;
            }





            dir = basename(collEnt->collName);

            path2 = concat(localFolder, "/");
            path = concat(path2, dir);



            debug("ReadDir %s",path);

            mkdir(path, S_IRWXU);

            free(path);
            free(path2);
            freeCollEnt(collEnt);
        }

    }

    // close the collection handle

    status = rcCloseCollection (rodsCommPtr, handleInx);

    return (status);
}




/**
 * @brief makeCollection Creates an empty collection.
 * @param collPath The path of the collection to be created in iRODS storage
 * @param rodsCommPtr An iRODS opened connection
 * @param makeRecursive Create parents recursively, if they didn't exist
 * @return A status code.
 */
int makeCollection(char *collPath, rcComm_t *rodsCommPtr, bool makeRecursive)
{
    collInp_t theColl;
    int status = 0;

    // initialize rods api coll struct
    memset(&theColl, 0, sizeof (collInp_t));
    rstrcpy(theColl.collName, collPath, MAX_NAME_LEN);

    // if a recursive operation is asked for, specify it
    if (makeRecursive)
        addKeyVal(&theColl.condInput, RECURSIVE_OPR__KW, "");

    // call rods api to make collection
    status = rcCollCreate(rodsCommPtr, &theColl);

    // return status to caller
    return (status);
}

/**
 * @brief removeCollection Removes an empty collection.
 *
 * This function needs that the contents of the collection to be deleted
 * be deleted first.
 *
 * @param collPath The collection to be deleted
 * @param rodsCommPtr An iRODS opened connection
 * @return A status code.
 */
int removeCollection(char *collPath, rcComm_t *rodsCommPtr)
{
    collInp_t theColl;
    int status = 0;

    // initialize rods api struct
    memset(&theColl, 0, sizeof (collInp_t));
    rstrcpy(theColl.collName, collPath, MAX_NAME_LEN);

    // initialize parameters for remove operation
    addKeyVal(&theColl.condInput, RECURSIVE_OPR__KW, "");
    addKeyVal(&theColl.condInput, FORCE_FLAG_KW, "");

    // call for rods api to remove collection
    status = rcRmColl(rodsCommPtr, &theColl, FALSE);

    // return status to caller
    return (status);
}

/**
 * @brief fsize Returns the file size of a local file
 * @param filename The path to the file
 * @return The size of the file
 */
off_t fsize(char *filename) {
    struct stat st;

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1;
}

/**
 * @brief iGetatt Fetch the size value of an iRODS object.
 * The params nfs_fh3 and fattar3 are needed to call the setatt function,
 * because the size variable will change the local value with the iRODS size value.
 * If this setatt is not executed an stale file handle error occurs.
 * @param fh
 * @param fat
 * @param path
 * @param size
 * @return
 */
int iGetatt(nfs_fh3 fh, fattr3 fat, char *path, int *size,int uidProxy){

    debug("iGetatt - UID %d ", uidProxy);

    char* userName=malloc (300 *(sizeof (char)));

    int resultLdap = getLdapName(uidProxy,userName);

    if(resultLdap<0){
        debug("iGetatt - Cant Connect to LDAP", "");
      exit(0);
    }

    rcComm_t* comm = rodsConnectProxy(userName,"tempZone");


    path = concatProxy(ZONE,userName,path);
   // rcComm_t* comm = rodsConnect();
    //path= concat(ZONE,path);
    int i = rodsLogin(comm);
    int j =  getatt_aux(fh, fat, comm, path, size);
    return j;
}

/**
 * @brief putFile Sends a file stored in the local filesystem to the iRODS storage
 *
 * This is similar to the iput CLI command
 *
 * @param localPath The path of the local file
 * @param objPath The path that the file will be writen
 * @param rodsCommPtr  An iRODS opened connection
 * @return A status code.
 */
int putFile(char *localPath, char *objPath, rcComm_t * rodsCommPtr)
{
    dataObjInp_t putParam;
    char filePath[MAX_NAME_LEN];
    rodsLong_t size = 0;
    int status = 0;
    int numThreads = 1;

    // get file size
    size = (rodsLong_t) fsize(localPath);

    // zero rods api input params structure
    memset(&putParam, 0, sizeof(dataObjInp_t));

    // initialize rods api input params struct
    putParam.dataSize = size;
    putParam.oprType = PUT_OPR;
    putParam.numThreads = numThreads;

    // set remote object path
    rstrcpy(putParam.objPath, objPath, MAX_NAME_LEN);

    // for now, we use the generic data type
    addKeyVal(&putParam.condInput, DATA_TYPE_KW, "generic");

    // target storage resource, if defined
    //if (rodsResc.length())
    //    addKeyVal(&putParam.condInput, DEST_RESC_NAME_KW, rodsResc.c_str());

    // take copy of the local file path for the rods api
    strcpy(filePath, localPath);

    // call rods api
    status = rcDataObjPut(rodsCommPtr, &putParam, filePath);

    // return rods api status
    return (status);
}


/**
 * @brief getFileInfo Retrieves information about a file located at the iRODS storage
 * @param path The path of the file in the iRODS storage
 * @param conn An opened iRODS connection
 * @param rodsObjStatOut The output parameter of this function, containing the file info
 * @return A status code.
 */
int getFileInfo(char *path, rcComm_t * conn, rodsObjStat_t **rodsObjStatOut)
{


    dataObjInp_t dataObjInp;

    //rodsObjStat_t *rodsObjStatOut = NULL;

    bzero (&dataObjInp, sizeof (dataObjInp));

    rstrcpy (dataObjInp.objPath, path, MAX_NAME_LEN);

    int status = rcObjStat (conn, &dataObjInp, rodsObjStatOut);

    if (status < 0) {
     return -1;
    }

    //printf("rods type %d \n",rodsObjStatOut->objType);

    //freeRodsObjStat (rodsObjStatOut);

    return (status);
}

/**
 * @brief getFile Downloads a file located at the iRODS storage.
 *
 * This function is similar to the iget CLI command
 *
 * @param localPath The path where the remote file will be writen in the local filesystem
 * @param objPath The path to the iRODS object to be downloaded
 * @param verifyChecksum Check file for errors
 * @param allowOverwrite Allow to overwrite an existing local file @see localPath
 * @param rodsCommPtr An opened iRODS connection
 * @return  A status code.
 */
int getFile(char *localPath, char *objPath, bool verifyChecksum, bool allowOverwrite, rcComm_t * rodsCommPtr)
{
    int status = 0;
    dataObjInp_t getParam;
    int numThreads = 1;

    // zero rods api param struct
    memset(&getParam, 0, sizeof (dataObjInp_t));

    // set parameters for get operation
    getParam.oprType = GET_OPR;
    getParam.numThreads = numThreads;

    if (verifyChecksum)
        addKeyVal(&getParam.condInput, VERIFY_CHKSUM_KW, "");

    if (allowOverwrite)
        addKeyVal(&getParam.condInput, FORCE_FLAG_KW, "");

    // copy obj path string
    rstrcpy(getParam.objPath, objPath, MAX_NAME_LEN);

    // execute data object get
    status = rcDataObjGet(rodsCommPtr, &getParam, localPath);

    // return rods api status
    return (status);
}

/**
 * @brief removeObj Deletes an iRODS object.
 *
 * This is similar to the irm CLI command.
 *
 * @param objPath The path to the object to be removed
 * @param rodsCommPtr An opened iRODS connection
 * @return A status code.
 */
int removeObj(char *objPath, rcComm_t * rodsCommPtr)
{
    dataObjInp_t theObj;
    int status = 0;

    // initialize rods api struct
    memset(&theObj, 0, sizeof (dataObjInp_t));
    rstrcpy(theObj.objPath, objPath, MAX_NAME_LEN);

    // initialize remove params
    addKeyVal(&theObj.condInput, FORCE_FLAG_KW, "");

    // call for rods api to remove data object
    status = rcDataObjUnlink(rodsCommPtr, &theObj);

    // return status to caller
    return (status);
}

/**
 * @deprecated
 * @brief testWrite This function was created just for learning purposes
 * @param conn
 */
void testWrite(rcComm_t * conn){
    dataObjInp_t dataObjInp;
    openedDataObjInp_t dataObjWriteInp;
    bytesBuf_t dataObjWriteOutBBuf;
    int bytesWrite;
    bzero (&dataObjInp, sizeof (dataObjInp));
    bzero (&dataObjWriteInp, sizeof (dataObjWriteInp));
    rstrcpy (dataObjInp.objPath, "/tempZone/home/rods/myfile", MAX_NAME_LEN);
    dataObjInp.openFlags = O_RDWR;
    addKeyVal (&dataObjInp.condInput, DEST_RESC_NAME_KW, "demoResc");

    openStat_t *openStat = NULL;

    dataObjWriteInp.l1descInx = rcDataObjOpenAndStat(conn, &dataObjInp, &openStat);

    bzero (&dataObjWriteOutBBuf, sizeof (dataObjWriteOutBBuf));

    char * ronaldo = "Ronaldo. Brilha muito no corinthians.\n";
    dataObjWriteInp.len = strlen(ronaldo);
    dataObjWriteOutBBuf.buf = (void *) ronaldo;
    dataObjWriteOutBBuf.len = strlen(ronaldo);

    fileLseekOut_t *dataObjLseekOut = NULL;

    dataObjWriteInp.offset = openStat->dataSize;
    dataObjWriteInp.whence = SEEK_CUR;

    int seekstatus = rcDataObjLseek (conn, &dataObjWriteInp, &dataObjLseekOut);

    if(seekstatus < 0){

    }

    bytesWrite = rcDataObjWrite (conn, &dataObjWriteInp, &dataObjWriteOutBBuf);

    if (bytesWrite < 0) {

    }

    rcDataObjClose (conn, &dataObjWriteInp);
}

/**
 * @brief createFile Creates an empty file in the iRODS storage
 * @param objPath The path of the iRODS file to be created
 * @param rodsCommPtr An opened iRODS connection
 * @return A status code.
 */
int createFile(char *objPath, rcComm_t * rodsCommPtr){
    //char *emptyFile = "emptyFile";
    /*debug("Caiu aqui 1\n", 0);

    if( access( emptyFile, F_OK ) == -1 ) {
        debug("Caiu aqui 2\n", 0);
        char createCommand[256] = "touch /home/emc/emptyFile";
        system(createCommand);
    }

    int status;
    status = putFile("/home/emc/emptyFile", objPath, rodsCommPtr);
    */

    dataObjInp_t dataObjInp;

    bzero (&dataObjInp, sizeof (dataObjInp));

    rstrcpy (dataObjInp.objPath, objPath, MAX_NAME_LEN);
    dataObjInp.createMode = 0750;
    dataObjInp.dataSize = 12345;
    dataObjInp.openFlags = O_WRONLY;
    addKeyVal (&dataObjInp.condInput, DEST_RESC_NAME_KW, "demoResc");
    int status = rcDataObjCreate (rodsCommPtr, &dataObjInp);

    return status;
}

/**
 * @deprecated
 * @brief iread
 * @param objPath
 * @param rodsCommPtr
 * @return
 */
int iread(char *objPath, rcComm_t * rodsCommPtr){
    return 0;
}

/**
 * @brief renameFile Renames an iRODS object.
 *
 * This is similar to the imv CLI command.
 *
 * @param objPathDes The new object file name
 * @param objPathSrc The path to the object to be renamed
 * @param rodsCommPtr An opened iRODS connection.
 * @return A status code.
 */
int renameFile(char *objPathDes, char *objPathSrc, rcComm_t * rodsCommPtr){
    int status = 0;

    dataObjCopyInp_t dataObjRenameInp;
    bzero (&dataObjRenameInp, sizeof (dataObjRenameInp));
    rstrcpy (dataObjRenameInp.destDataObjInp.objPath, objPathDes, MAX_NAME_LEN);
    rstrcpy (dataObjRenameInp.srcDataObjInp.objPath, objPathSrc, MAX_NAME_LEN);
    status = rcDataObjRename (rodsCommPtr, &dataObjRenameInp);

    return (status);
}

/**
 * @deprecated
 * @brief testRead This function was created solely to learning purposes
 * @param conn
 */
void testRead(rcComm_t * conn){

    dataObjInp_t dataObjInp;
    openedDataObjInp_t dataObjReadInp;
    bytesBuf_t dataObjReadInpBBuf;
    int bytesRead;
    int i;

    bzero (&dataObjInp, sizeof (dataObjInp));
    bzero (&dataObjReadInp, sizeof (dataObjReadInp));
    rstrcpy (dataObjInp.objPath, "/tempZone/home/emc/remotefolder/foo", MAX_NAME_LEN);

    dataObjInp.openFlags = O_RDONLY;
    dataObjReadInp.l1descInx = rcDataObjOpen (conn, &dataObjInp);

    if (dataObjReadInp.l1descInx < 0) {

    }

    bzero (&dataObjReadInpBBuf, sizeof (dataObjReadInpBBuf));
    dataObjReadInp.len = 62;
    bytesRead = rcDataObjRead (conn, &dataObjReadInp, &dataObjReadInpBBuf);

    char * buff = (char *) dataObjReadInpBBuf.buf;

    buff[61] = '\0';

    printf("%d \n %s.\n", bytesRead = dataObjReadInpBBuf.len, buff);


    //for(i = 0; i < dataObjReadInpBBuf.len; i++){
    //    printf("%d ", buff[i]);
    //}

    // printf("\n");

    if (bytesRead < 0) {

    }
}



/**
 * @brief get_pre_cached_irods funcao não mais utilizada, ela recupera informacoes de um arquivo antes da modificacao de um atributo
 * @param path
 * @param owner
 * @return
 */
pre_op_attr get_pre_cached_irods(char *path,char** owner)
{
    pre_op_attr result;

    debug("PATHHHH  %s\n",path);


    rcComm_t* comm = rodsConnect();
    rodsLogin(comm);

    OutputQuery outputQuery;
    InputQuery inputQuery;

    inputQuery.selectSize=4;
    int *selects = (int *) malloc(inputQuery.selectSize * sizeof(int));

    selects[0] = 407;//size
    selects[1] = 411;//owner
    selects[2] = 420;//modify
    selects[3] = 403;//name

    inputQuery.selectsColumns =selects ;
    inputQuery.condSize=2;

    int *columns = (int *) malloc(inputQuery.condSize * sizeof(int));
    columns[0] = 403;//dataname
    columns[1] = 501;//path
    inputQuery.condColumns=columns;

    char **columnValues = (char **) malloc(300*inputQuery.condSize* sizeof(char));


    char* substr = (char*) malloc(strlen(path));
    strncpy(substr,path + (fileNameLastIndex(path)+1) ,strlen(path));
    char filename[strlen(path)];
    snprintf(filename, sizeof filename, "='%s'",substr);

    columnValues[0]=filename;

    strncpy(substr,path  ,fileNameLastIndex(path));
    char pathname[strlen(path)+10];
    snprintf(pathname, sizeof pathname, "='/tempZone%s'",substr);

    columnValues[1]=pathname;

    inputQuery.condValues=columnValues;




    int j = genQuery(comm,&inputQuery,&outputQuery);

    //atribuir esses tempos

    debug("d  %d\n",j);



    debug("PRE irods SIZE  %s\n",outputQuery.resultValues[0]);
    debug("PRE irods CTIME %s\n",outputQuery.resultValues[1]);
    debug("PRE irods MTIME %s\n",outputQuery.resultValues[2]);
    debug("PRE irods dataname %s\n",outputQuery.resultValues[3]);

    *owner= outputQuery.resultValues[1];






    result.pre_op_attr_u.attributes.size = atoi(outputQuery.resultValues[0]);
    result.pre_op_attr_u.attributes.ctime.seconds = 0;
    result.pre_op_attr_u.attributes.ctime.nseconds = 0;
    result.pre_op_attr_u.attributes.mtime.seconds = atoi(outputQuery.resultValues[2]);
    result.pre_op_attr_u.attributes.mtime.nseconds = 0;




    return result;

}

/**
 * @brief muda o acesso a um arquivo do irods
 * @param path
 * @param userName
 * @param accessLevel write, read, own, null
 * @param conn
 * @return
 */
int changeAttr(char *path,char *userName,char * accessLevel,rcComm_t *conn){

    int status=0;

    modAccessControlInp_t modAccessControlInp;

    modAccessControlInp.recursiveFlag = 0;

    modAccessControlInp.accessLevel = accessLevel;

    modAccessControlInp.userName = userName;

    modAccessControlInp.zone = "tempZone";

    modAccessControlInp.path = path;

    debug("SETATTR - FILE %s\n",path);
    debug("SETATTR - MOD to %s\n",accessLevel);
    debug("SETATTR - USER to %s\n",userName);


    status = rcModAccessControl(conn, &modAccessControlInp);



    return status;
}


/**
 * @brief iWrite  Writes a sequence of bytes in a iRODS file.
 *
 * This function is similar to the pwrite function from the POSIX API.
 * It writes up to len bytes, starting from a given offset.
 *
 * @param objPath The path of the iRODS file
 * @param buff The byte array to be writen
 * @param len The length of the byte array
 * @param offset The initial position to start writing the data sequence
 * on the remote file
 * @param conn An opened iRODS connection
 * @return A status code.
 */
int iWrite(char *objPath, char *buff, int len, int offset, rcComm_t *conn)
{
    dataObjInp_t dataObjInp;
    openedDataObjInp_t dataObjWriteInp;
    bytesBuf_t dataObjWriteOutBBuf;
    int bytesWrite;
    bzero (&dataObjInp, sizeof (dataObjInp));
    bzero (&dataObjWriteInp, sizeof (dataObjWriteInp));
    rstrcpy (dataObjInp.objPath, objPath, MAX_NAME_LEN);
    dataObjInp.openFlags = O_RDWR;
    addKeyVal (&dataObjInp.condInput, DEST_RESC_NAME_KW, "demoResc");

    openStat_t *openStat = NULL;

    dataObjWriteInp.l1descInx = rcDataObjOpenAndStat(conn, &dataObjInp, &openStat);

    bzero (&dataObjWriteOutBBuf, sizeof (dataObjWriteOutBBuf));

    dataObjWriteInp.len = len;
    dataObjWriteOutBBuf.buf = (void *) buff;
    dataObjWriteOutBBuf.len = len;

    fileLseekOut_t *dataObjLseekOut = NULL;

    dataObjWriteInp.offset = offset;
    dataObjWriteInp.whence = SEEK_CUR;

    int seekstatus = rcDataObjLseek (conn, &dataObjWriteInp, &dataObjLseekOut);

    if(seekstatus < 0){
        return -1;
    }

    bytesWrite = rcDataObjWrite (conn, &dataObjWriteInp, &dataObjWriteOutBBuf);

    if (bytesWrite < 0) {
        return -1;
    }

    int status1 = rcDataObjClose (conn, &dataObjWriteInp);

    return status1;
}

/**
 * @brief rodsConnectLDAP Opens an iRODS connection using LDAP authentication
 * @param uidProxy The user ID stored in the LDAP backend
 * @return An opened iRODS connection.
 */
rcComm_t* rodsConnectLDAP(int uidProxy){

    debug("Login  RUID %jd", uidProxy);

    char* userName=malloc (300 *(sizeof (char)));

    int resultLdap = getLdapName(uidProxy,userName);

    if(resultLdap<0){
        debug("CREATE - Cant Connect to LDAP", "");
        exit(0);
    }

    rcComm_t* comm = rodsConnectProxy(userName,"tempZone");

    free( userName );

    return comm;
}




/**
 * @brief Retorna o path do irods que sera utilizado nas consultas dos callbacks
 * @param id do usuario
 * @param path
 * @return
 */
char* getRodsPath(int uidProxy,char* path){
    debug("getRodsPath START", "");

    char* userName=malloc(3000*(sizeof (char)));

    int resultLdap = getLdapName(uidProxy,userName);

    if(resultLdap<0){
        debug("CREATE - Cant Connect to LDAP", "");
        resultLdap = getLdapName(1000,userName);
        //FIXME
        //exit(0);
    }

     char *rodspath = concatProxy(ZONE,userName, path);

     free( userName );

     debug("getRodsPath END", "");

     return rodspath;

}

/**
 * @brief fileSize Retrieves the file size of an iRODS file.
 * @param objPath The path of the object to be queried
 * @param conn An opened iRODS connection.
 * @return The file size.
 */
rodsLong_t fileSize(char * objPath, rcComm_t * conn ){
    dataObjInp_t dataObjInp;
    openedDataObjInp_t dataObjWriteInp;
    bzero (&dataObjInp, sizeof (dataObjInp));
    bzero (&dataObjWriteInp, sizeof (dataObjWriteInp));
    rstrcpy (dataObjInp.objPath, objPath, MAX_NAME_LEN);
    dataObjInp.openFlags = O_RDWR;
    addKeyVal (&dataObjInp.condInput, DEST_RESC_NAME_KW, "demoResc");

    openStat_t *openStat = NULL;

    dataObjWriteInp.l1descInx = rcDataObjOpenAndStat(conn, &dataObjInp, &openStat);


    if(openStat){
        return openStat->dataSize;
    }
    else{
        return 0;
    }
}

/**
 * @brief fileMode realiza uma consulta query para descobrir o acesso a um arquivo .
 * @param path The path of the object to be queried
 * @param conn An opened iRODS connection.
 * @return id da permissao .1200 own, 1120 write, 1050 read
 */
int fileMode(char * path, rcComm_t * comm ){


    OutputQuery outputQuery;
    InputQuery inputQuery;

    inputQuery.selectSize=1;
    int *selects = (int *) malloc(inputQuery.selectSize * sizeof(int));


    selects[0] = 700;//idmode

    inputQuery.selectsColumns =selects ;
    inputQuery.condSize=2;

    int *columns = (int *) malloc(inputQuery.condSize * sizeof(int));
    columns[0] = 403;//dataname
    columns[1] = 501;//path
    inputQuery.condColumns=columns;

    //char **columnValues = (char *) malloc(inputQuery.condSize* sizeof(char));
    char **columnValues = (char **) malloc(3000*inputQuery.condSize* sizeof(char));

    char* substr = (char*) malloc(strlen(path));
    strncpy(substr,path + (fileNameLastIndex(path)+1) ,strlen(path));
    char filename[strlen(path)];
    snprintf(filename, sizeof filename, "='%s'",substr);

    columnValues[0]=filename;

    strncpy(substr,path  ,fileNameLastIndex(path));
    char pathname[strlen(path)+10];
    snprintf(pathname, sizeof pathname, "='%s'",substr);

    free(substr);

    columnValues[1]=pathname;

    inputQuery.condValues=columnValues;



    int s = genQuery(comm,&inputQuery,&outputQuery);

    if(s<0){
        return -1;
    }

    char * result = malloc(sizeof(outputQuery.resultValues[0]));

    strcpy(result, outputQuery.resultValues[0]);

    free(outputQuery.resultCollumns);
    free(outputQuery.resultValues);


    return atoi(result);

}


