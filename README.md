# NFS-RODS: A Tool for Accessing iRODS Repositories via the NFS Protocol

## Introduction

IRODS is an open source platform for managing, sharing and integrating data. It has been widely adopted by organizations around the world. iRODS is released and maintained through the iRODS Consortium which involves universities, research agencies, government, and commercial organizations. It aims to drive the continued development of iRODS platform, as well as support the fundraising, development, and expasion of the iRODS user community. iRODS is supported by CentOS, Debian and OpenSuse operating systems.

NFS-RODS is a server that exposes the iRODS file system via the NFS v3 protocol. It is based on the UNFS3 server (User-space NFS). By setting up a NFS-RODS server, users can access and change iRODS directories as if they were local ones, by using the mount command.

## Features

NFS-RODS provides:

- User authentication via LDAP
- Basic file and directory operations: read, write, ls, rm, rmdir, touch, chmod, etc.

It does not support:

- Handling iRODS metadata
- NFS symbolic links


## Examples of utilization
