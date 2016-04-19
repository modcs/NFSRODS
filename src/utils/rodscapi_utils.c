#include "irodsapi/rodscapi.h"
#include <stdio.h>
#include <sys/stat.h>
#include "dataObjLseek.h"
#include "utils/rodscapi_utils.h"
#include "utils/utils.h"
#include "nfs.h"





/**
 * @brief getatt_aux Fills a fattr3_atributes with iRODS object attributes.
 * Unfortunately only two attributes were possible to be mapped, until now.
 * @param comm Connection variable
 * @param path Path to the corresponding object (directory or file)
 * @param fattr3_atributes
 *
 * Example of use:
 *  fattr3 fattr3_atributes;
    rcComm_t* comm = rodsConnect();
    int i = rodsLogin(comm);
    char* path = "tempZone/home/desiredFile.txt");
    getatt_aux(comm, path, &fattr3_atributes);
    printf("Type: %d\n", fattr3_atributes.type);
    printf("Size: %d\n", fattr3_atributes.size);
 */
int getatt_aux(nfs_fh3 fh, fattr3 fat, rcComm_t* comm, char *path, int *size)
{
    dataObjInp_t dataObjInp;
    rodsObjStat_t *rodsObjStatOut = NULL;
    bzero (&dataObjInp, sizeof (dataObjInp));
    rstrcpy (dataObjInp.objPath, path, MAX_NAME_LEN);
    int status = rcObjStat (comm, &dataObjInp, &rodsObjStatOut);

    if (status < 0)
    {
        debug("GETATT - Error - The call to iRODS was unsuccessful searching the file %s.\n", path);
    }
    else
    {
        *size = rodsObjStatOut->objSize;

        /*It is necessary to change the attributes locally (executing a SETATT) to avoid the NFS Stale File Handle error.*/
        set_aux(fh, fat, path, size);

    }
    return status;

    freeRodsObjStat (rodsObjStatOut);
}

void set_aux(nfs_fh3 fh, fattr3 fat, char *path, int size)
{
    sattr3 s;
    s.atime.set_atime_u.atime = fat.atime;
    s.gid.set_gid3_u.gid = fat.gid;
    s.mode.set_mode3_u.mode = fat.mode;
    s.mtime.set_mtime_u.mtime = fat.mtime;
    s.size.set_size3_u.size = size;
    s.uid.set_uid3_u.uid = fat.uid;
    set_attr(path, fh, s);
}

/**
 * @brief executeQuery Based on a full_path, a query is executed intending to fill the OutputQuery object with the database attributes desired.
 * @param full_path The path for a file or object
 * @param comm Connection variable
 * @param attributes_number How many attributes it is intended to show
 * @param selected_attributes The respective database attributes to be fetched
 * @param outputQuery The resulted values are filled under this variable
 * @return
 * Example of Use:
 *    rcComm_t* comm = rodsConnect();
    int i = rodsLogin(comm);
    OutputQuery *outputQuery;
    char full_path[]= "/home/airton/emc/folderserver/log.txt";

    int attributes_number = 2;
    int *selected_attributes = (int *) malloc(attributes_number * sizeof(int));
    selected_attributes[0] = 407;//size
    selected_attributes[1] = 703;//user_id
    executeQuery(full_path, comm, attributes_number, selected_attributes, outputQuery);
 */
int executeQuery(char full_path[],  rcComm_t* comm, int attributes_number, int selected_attributes[], OutputQuery outputQuery){

    InputQuery inputQuery;

    inputQuery.selectSize=attributes_number;

    inputQuery.selectsColumns =selected_attributes ;
    inputQuery.condSize=2;

    int *columns = (int *) malloc(inputQuery.condSize * sizeof(int));
    columns[0] = 403;//dataname
    columns[1] = 501;//path
    inputQuery.condColumns=columns;

    char **columnValues = (char **) malloc(300*inputQuery.condSize* sizeof(char));


    char* substr = (char*) malloc(strlen(full_path));
    strncpy(substr,full_path + (fileNameLastIndex(full_path)+1) ,strlen(full_path));
    char filename[strlen(full_path)];
    snprintf(filename, sizeof filename, "='%s'",substr);

    columnValues[0]=filename;

    strncpy(substr,full_path  ,fileNameLastIndex(full_path));
    char pathname[strlen(full_path)+10];

    snprintf(pathname, sizeof pathname, "=%s%s", ZONE, substr);

    columnValues[1]=pathname;

    inputQuery.condValues=columnValues;

    int j = genQuery(comm,&inputQuery,&outputQuery);
    int k=0;

    for(k=0;k<attributes_number;k++){
       printf("value %s \n",outputQuery.resultValues[k]);
    }
}










/**
 * @brief genQuery Query any information from iRODS database. See rodsGenQuery.c.
 * @param conn Connection variable
 * @param inputQuery
 * @param outputQuery
 * @return
 */
int genQuery(rcComm_t * conn, InputQuery *inputQuery, OutputQuery *outputQuery)
{
    printf("start gen query\n");
    genQueryInp_t queryInput;
    genQueryOut_t *queryOutput;
    int status = 0, prevStatus = 0;

    // zero rods api data structures
    memset(&queryInput, 0, sizeof(genQueryInp_t));
    queryOutput = NULL;

    // set rods api select array sizes
    queryInput.selectInp.len = inputQuery->selectSize;
    queryInput.maxRows = 100;

    // allocate new arrays for rods api
    queryInput.selectInp.inx = (int *) malloc(inputQuery->selectSize * sizeof(int));
    queryInput.selectInp.value = (int *) malloc(inputQuery->selectSize * sizeof(int));

    int i;
    // build rods api select arrays
    for (i=0; i <  inputQuery->selectSize; i++)
    {
        queryInput.selectInp.inx[i] = inputQuery->selectsColumns[i];
        queryInput.selectInp.value[i] = 0;
    }

    // set rods api condition array sizes
    queryInput.sqlCondInp.len = inputQuery->condSize;

    // allocate new arrays for rods api
    queryInput.sqlCondInp.inx = (int *) malloc(inputQuery->condSize * sizeof(int));


    //alocar separadamente
    //queryInput.sqlCondInp.value = (char **) malloc(3000*inputQuery->condSize * sizeof(char));



    queryInput.sqlCondInp.value = malloc( inputQuery->condSize * sizeof(char*));

    for (i = 0; i < inputQuery->condSize; i++)
        queryInput.sqlCondInp.value[i] = malloc((2900) * sizeof(char));




    for (i=0; i < inputQuery->condSize ; i++)
    {
        queryInput.sqlCondInp.inx[i] = inputQuery->condColumns[i];
        queryInput.sqlCondInp.value[i] = inputQuery->condValues[i];
    }



    // try to execute a generic query
    if (!(status = rcGenQuery(conn, &queryInput, &queryOutput)))
    {

        int *resultColumns = (int *) malloc(inputQuery->selectSize * sizeof(int));
        char **resultValues = (char **) malloc(3000*inputQuery->selectSize* sizeof(char));

        // iterate while there are results to process
        do {
            int j;
            for (i = 0; i < queryOutput->rowCnt; i++)
            {
                for (j = 0; j < queryOutput->attriCnt; j++)
                {

                    resultColumns[j] = inputQuery->selectsColumns[j];
                    resultValues[j] = queryOutput->sqlResult[j].value;
                    //printf("******value*****%s\n",queryOutput->sqlResult[j].value);

                }
            }

            outputQuery->resultCollumns=resultColumns;
            outputQuery->resultValues=resultValues;
            outputQuery->size=inputQuery->selectSize;

            // if there are no more results to query, exit loop
            if (!queryOutput->continueInx)
                break;

            // otherwise continue fetching query results
            queryInput.continueInx = queryOutput->continueInx;
            prevStatus = status;

            status = rcGenQuery(conn, &queryInput, &queryOutput);
        } while (!status);

    }else{
        debug("erro","");
    }

     //free rods api allocated resources
    free(queryInput.selectInp.inx);
    free(queryInput.selectInp.value);

    //for (i = 0; i < inputQuery->condSize; i++)
    //    free(queryInput.sqlCondInp.value[i]);



    free(queryInput.sqlCondInp.inx);
    free(queryInput.sqlCondInp.value);

    free(queryOutput);

    // let's not return no more rows found as an error
    if (status == CAT_NO_ROWS_FOUND)
        status = prevStatus;

    return (status);
}
