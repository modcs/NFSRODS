#include <ldap.h>

#define ZONE "/tempZone"

char * trim(char *string);

void debug(char * message, char * string);

char* concat(char *s1, char *s2);

char* concatProxy(char *zone, char *userName, char *path);

int getLdapConnection(LDAP **ldap);

int getLdapName(int id,char *name);

void rec_mkdir(const char *dir);

char* substring(char *string, int position, int length);

void toArray(char newString[],char *value);

int removedirectoryrecursively(const char *dirname);






