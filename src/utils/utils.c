#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include <syslog.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
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
 * @brief concat Concat two strings returning the combined string.
 * @param s1
 * @param s2
 * @return
 */
char* concatProxy(char *zone, char *userName, char *path)
{
    debug("concatProxy START ","");
    //char *result = malloc(strlen(userName)+strlen(path)+strlen(zone)+1);//+1 for the zero-terminator
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



char *LDAPHOST="armazenamento";
char *LDAP_DN = "cn=admin, dc=irods, dc=example, dc=org";
char *USERS_LDAP_DN = "OU=users,DC=irods,DC=example,DC=org";
char *LDAPPSS = "admin";
int  LDAPVERSION = LDAP_VERSION3;
int LDAPPORT=LDAP_PORT;

/**
 * @brief getLdapConnection
 * @param ldap
 * @return
 */
int getLdapConnection(LDAP **ldap){


    //LDAP *ldap;
    int result;

    if ( ( *ldap = ldap_init( LDAPHOST, LDAPPORT ) ) == NULL ) {
       debug( "ldap_init error","" );
       return( -1 );
    }

    /* The LDAP_OPT_PROTOCOL_VERSION session preference specifies the client */
    /* is an LDAPv3 client. */
    result = ldap_set_option(*ldap, LDAP_OPT_PROTOCOL_VERSION, &LDAPVERSION);

    if ( result != LDAP_OPT_SUCCESS ) {
        debug( "ldap_init error","" );
        return( -1 );
    } else {
      debug("Set LDAPv3 client version.","");
    }


    /* Attempt authentication. */
    if ( ldap_simple_bind_s( *ldap, LDAP_DN, LDAPPSS ) != LDAP_SUCCESS ) {
       debug( "ldap_simple_bind_s error","" );
       return( -1 );
    }


    return 0;


}

/**
 * @brief getLdapName
 * @param id
 * @param name
 * @return
 */
int getLdapName(int id,char *name){

    LDAP *ldap=NULL;

    int r = getLdapConnection(&ldap);


    if(r!=0){
        debug("error Ldap Connection","");
        return -1;
    }


    int result;

   // The search scope must be either LDAP_SCOPE_SUBTREE or LDAP_SCOPE_ONELEVEL
   int  scope          = LDAP_SCOPE_SUBTREE;
   // The search filter, "(objectClass=*)" returns everything. Windows can return
   // 1000 objects in one search. Otherwise, "Size limit exceeded" is returned.
   char *baseFilter = "(&(objectClass=*)(uidNumber=%d))";
   char *filter=  malloc( 200*(sizeof (char)));
   sprintf(filter, baseFilter, id);
    //char *filter = "(&(objectClass=*)(uidNumber=1001))";
    // The attribute list to be returned, use {NULL} for getting all attributes
   char *attrs[]       = {"cn"};
   // Specify if only attribute types (1) or both type and value (0) are returned
   int  attrsonly      = 0;
   // entries_found holds the number of objects found for the LDAP search
   int  entries_found  = 0;
   // dn holds the DN name string of the object(s) returned by the search
   char *dn            = "";
   // attribute holds the name of the object(s) attributes returned
   char *attribute     = "";
   // values is  array to hold the attribute values of the object(s) attributes
   char **values;
   // i is the for loop variable to cycle through the values[i]
   int  i              = 0;

   LDAPMessage *answer, *entry;
   BerElement *ber;



 /* STEP 3: Do the LDAP search. */
 result = ldap_search_s(ldap, USERS_LDAP_DN, scope, filter,
                        attrs, attrsonly, &answer);


 if ( result != LDAP_SUCCESS ) {
   debug("Search Ldap Error","");
   return( -1 );
 }

 /* Return the number of objects found during the search */
 entries_found = ldap_count_entries(ldap, answer);
 if ( entries_found == 0 ) {
   debug("Search Ldap Error non foud users","");
   return( -1 );
 }

 /* cycle through all objects returned with our search */
 for ( entry = ldap_first_entry(ldap, answer);
       entry != NULL;
       entry = ldap_next_entry(ldap, entry)) {

   /* Print the DN string of the object */
   dn = ldap_get_dn(ldap, entry);


   // cycle through all returned attributes
   for ( attribute = ldap_first_attribute(ldap, entry, &ber);
         attribute != NULL;
         attribute = ldap_next_attribute(ldap, entry, ber)) {

     /* Print the attribute name */

     if ((values = ldap_get_values(ldap, entry, attribute)) != NULL) {

       /* cycle through all values returned for this attribute */
       for (i = 0; values[i] != NULL; i++) {

           strcpy(name, trim(values[i]));
         /* print each value of a attribute here */


       }
       ldap_value_free(values);
     }
   }
   ldap_memfree(dn);
 }

 ldap_msgfree(answer);
 ldap_unbind(ldap);



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
            //printf("Deleting: %s\n", path);
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
    //printf("Deleting: %s\n", dirname);
    rmdir(dirname);

    return 1;
}



