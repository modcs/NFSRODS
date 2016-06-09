/* NFS-RODS: A Tool for Accessing iRODS Repositories
 * via the NFS Protocol
 * (C) 2016, Danilo Mendonça, Vandi Alves, Iure Fe,
 * Aleciano Lobo Junior, Francisco Airton Silva,
 * Gustavo Callou and Paulo Maciel <prmm@cin.ufpe.br>
 */

#include <ldap.h>

char * trim(char *string);

void debug(char * message, char * string);

char* concat(char *s1, char *s2);

char* concatProxy(char *zone, char *userName, char *path);

int getUserName(int id,char *name);

void rec_mkdir(const char *dir);

char* substring(char *string, int position, int length);

void toArray(char newString[],char *value);

int removedirectoryrecursively(const char *dirname);

void configureEnv();

char *ZONE;







