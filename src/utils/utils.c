/* NFS-RODS: A Tool for Accessing iRODS Repositories
 * via the NFS Protocol
 * (C) 2016, Danilo Mendonça, Vandi Alves, Iure Fe,
 * Aleciano Lobo Junior, Francisco Airton Silva,
 * Gustavo Callou and Paulo Maciel <prmm@cin.ufpe.br>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>
#include "unistd.h";
#include "nfs.h"
#include "utils.h"
#include "json.h"

/*
 *
 * NFS-RODS Project
 *
 * Utility class for assisting operations in nfs.c class.
 *
 */


/**
 * @brief debug Saves log information at the “/var/log/syslog” location, using the header word: "NFS-RODS-DAEMON".
 * @param message Information to be logged
 * @param string Additional string that can be used as same as done with printf function: "printf("message %s",string)".
 * More information about SysLog: http://www.gnu.org/software/libc/manual/html_node/Overview-of-Syslog.html#Overview-of-Syslog
 */
void debug(char * message, char * string){
    openlog ("NFS-RODS-DAEMON", LOG_DAEMON, LOG_USER);
    syslog (LOG_ERR, message, string);
    closelog ();
}


/**
 * @brief concat Concat two strings returning the combined string.
 * @param s1
 * @param s2
 * @return
 */
char* concat(char *s1, char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
    if(result == NULL){
        debug("Error in MALLOC of Concat function from 'utils.c'",NULL);
    }

    strcpy(result, s1);
    strcat(result, s2);
    return result;
}


/**
 * @brief concatProxy Concat two strings returning the combined string.
 * @param s1
 * @param s2
 * @return
 */
char* concatProxy(char *zone, char *userName, char *path)
{
    debug("concatProxy START ","");

    char *result = malloc(3000*sizeof(char));//+1 for the zero-terminator

    if(result == NULL){
        debug("Error in MALLOC of Concat function from 'utils.c'",NULL);
    }

    strcpy(result, zone);
    strcat(result, "/home/");
    strcat(result, userName);
    strcat(result, path);

    debug("concatProxy END ","");

    return result;
}


/**
 * @brief getUserName
 * @param id
 * @param name
 * @return
 */
int getUserName(int uid,char *name){

    if(!NFS_RODS_PRIVATE_COLL){
        return 1;
    }

    struct passwd *pw = getpwuid(uid);

    if (pw)
    {
        sprintf(name,"%s",pw->pw_name);
    }else{
        return -1;
    }
    return 0;
}


/**
 * @brief rec_mkdir Recursive mkdir.
 *
 * Creates a folder in the local filesystem, and all ancestors, if necessary.
 *
 * @param dir The directory to be created
 */
void rec_mkdir(const char *dir) {
    char tmp[256];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp),"%s",dir);
    len = strlen(tmp);
    if(tmp[len - 1] == '/')
        tmp[len - 1] = 0;
    for(p = tmp + 1; *p; p++)
        if(*p == '/') {
            *p = 0;
            debug("READDIR create %s",tmp);
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    mkdir(tmp, S_IRWXU);
    debug("READDIR create %s",tmp);
}


/**
 * @brief trim Remove extra blank spaces in the start or end of a string
 * @param string The string to be trimmed
 * @return A trimmed string.
 */
char * trim(char *string)
{
    char *start;
    int len = strlen(string);
    int i;

    /* Find the first non whitespace char */
    for (i = 0; i < len; i++) {
        if (! isspace(string[i])) {
            break;
        }
    }

    if (i == len) {
        /* string is all whitespace */
        return NULL;
    }

    start = &string[i];

    /* Remove trailing white space */
    for (i = len; i > 0; i--) {
        if (isspace(string[i])) {
            string[i] = '\0';
        } else {
            break;
        }
    }

    return start;
}


/**
 * @brief substring Auxiliary function to get a substring
 * @param string The original string
 * @param position The start position
 * @param length The length of the substring
 * @return A substring.
 */
char* substring(char *string, int position, int length)
{
    char *pointer;
    int c;
    pointer = malloc(length+1);

    if (pointer == NULL)
    {
        printf("Unable to allocate memory.\n");
        exit(1);
    }

    for (c = 0 ; c < length ; c++)
    {
        *(pointer+c) = *(string+position-1);
        string++;
    }

    *(pointer+c) = '\0';

    return pointer;
}


/**
 * @brief toArray Copy the contents of a pointer to an array
 * @param newString The destination array
 * @param value The memory region to be copied
 */
void toArray(char newString[],char *value){

    int var=0;
    while (*(value+var)!='\0') {
        newString[var]= *(value+var);
        var++;
    }
    newString[var]='\0';
}


/**
 * @brief removedirectoryrecursively Performs a "rm -R" operation in the local filesystem
 * @param dirname The directory to be deleted
 * @return A status code.
 */
int removedirectoryrecursively(const char *dirname)
{
    DIR *dir;
    struct dirent *entry;
    char path[PATH_MAX];

    dir = opendir(dirname);
    if (dir == NULL) {
        perror("Error opendir()");
        return 0;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            snprintf(path, (size_t) PATH_MAX, "%s/%s", dirname, entry->d_name);
            if (entry->d_type == DT_DIR) {
                removedirectoryrecursively(path);
            }

            /*
             * Here, the actual deletion must be done.  Beacuse this is
             * quite a dangerous thing to do, and this program is not very
             * well tested, we are just printing as if we are deleting.
             */
            remove(path);
            /*
             * When you are finished testing this and feel you are ready to do the real
             * deleting, use this: remove*STUB*(path);
             * (see "man 3 remove")
             * Please note that I DONT TAKE RESPONSIBILITY for data you delete with this!
             */
        }

    }
    closedir(dir);

    /*
     * Now the directory is emtpy, finally delete the directory itself. (Just
     * printing here, see above)
     */
    rmdir(dirname);

    return 1;
}


char* readFile(char * fileName, long * fileSize){
    char *buffer;
    FILE *fh = fopen(fileName, "rb");
    if ( fh != NULL )
    {
        fseek(fh, 0L, SEEK_END);
        long s = ftell(fh);
        *fileSize = s;
        rewind(fh);
        buffer = malloc(s);
        if ( buffer != NULL )
        {
            fread(buffer, s, 1, fh);
            fclose(fh); fh = NULL;
        }
        if (fh != NULL) fclose(fh);
    }

    return buffer;
}


void processJSON(json_value* value){
    int length = value->u.object.length;

    int i=0;
    for(i = 0; i < length; i++){
        char* string = value->u.object.values[i].name;

        if (strcmp(string, "irods_zone") == 0) {

            ZONE =  value->u.object.values[i].value->u.string.ptr;

        } else if (strcmp(string, "use_private_collection") == 0) {

            int result = value->u.object.values[i].value->u.boolean;

            if(result==1)
                NFS_RODS_PRIVATE_COLL = TRUE;
            else
                NFS_RODS_PRIVATE_COLL = FALSE;
        }
    }
}


const char *getConfUserName()
{
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    if (pw)
    {
        return pw->pw_name;
    }

    return "";
}


void configureEnv(){
    char * home = "/home/";
    char * username = getConfUserName();
    char * dir = concat(home, username);
    char * file = concat(dir, "/.nfsrods-config.json");

    if( access( file, F_OK ) == -1 ) {

        NFS_RODS_PRIVATE_COLL = FALSE;

        return;

    }else{

        long fileSize;
        char* contents = readFile(file, &fileSize);
        json_char* json = (json_char*)contents;
        json_value* value = json_parse(json,fileSize);
        processJSON(value);
        debug("Configure NFS_RODS_PRIVATE_COLL_ACTIVATED %d",NFS_RODS_PRIVATE_COLL);
        debug("Configure Zone %s",ZONE);
    }

}
