# NFS-RODS: A Tool for Accessing iRODS Repositories via the NFS Protocol

## Introduction

IRODS is an open source platform for managing, sharing and integrating data [1]. It has been widely adopted by organizations around the world. iRODS is released and maintained through the iRODS Consortium which involves universities, research agencies, government, and commercial organizations. It aims to drive the continued development of iRODS platform, as well as support the fundraising, development, and expasion of the iRODS user community. iRODS is supported by CentOS, Debian and OpenSuse operating systems.

NFS-RODS is a server that exposes the iRODS file system via the NFS v3 protocol. It is based on the UNFS3 server (User-space NFS)[2]. By setting up a NFS-RODS server, users can access and change iRODS directories as if they were local ones, by using the mount command.

## Features

NFS-RODS provides:

- User authentication via LDAP
- Basic file and directory operations: read, write, ls, rm, rmdir, touch, chmod, etc.

It does not support:

- Handling iRODS metadata
- NFS symbolic links


## Examples of utilization

The figure bellow depicts the examples through three commands: mkdir, for creating a folder; rm, for removing a folder; and, mv for renaming also a folder:

![Examples](https://raw.githubusercontent.com/modcs/NFSRODS/master/images/examples.png)

To show the creating process example (Figure (a)), we first executed the ils command to list the data content of our iRODS repository, which corresponds to exemp1, exemp3, exemp4 and exemp8 folders. Therefore, after executing the mkdir exemp0, we also needed to list the new content of our repository on the iRODS server to show that the exemp0 folder was created.


Similarly to the previous example, to show the remove directory operation (Figure (b)), we first executed the ils command to list the data content of our iRODS repository, which corresponds to exemp0, exemp1, exemp3, exemp4 and exemp8 folders. Therefore, after executing the rm -R exemp8 command, we also needed to list the new content of our repository on the iRODS server to show that the exemp8 folder was deleted.

Finally, the last example shows the rename operation (Figure (c)). Again, we first executed the ils command to list the data content of our iRODS repository, which corresponds to exemp0, exemp1, exemp3 and exemp4 folders. Therefore, after executing the mv exemp4 exemp5 command, we also needed to list the new content of our repository on the iRODS server to show that the exemp has been renamed for exemp5 folder.

## References

- [1] - iRODS. The integrated rule-oriented data system (irods). http://irods.org/
- [2] - UNFS. User-space NFSv3 Server. http://unfs3.sourceforge.net
