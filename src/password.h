/* NFS-RODS: A Tool for Accessing iRODS Repositories
 * via the NFS Protocol
 * (C) 2016, Danilo Mendonça, Vandi Alves, Iure Fe,
 * Aleciano Lobo Junior, Francisco Airton Silva,
 * Gustavo Callou and Paulo Maciel <prmm@cin.ufpe.br>
 *
 * Original Copyright notice
 * UNFS3 NFS protocol procedures
 * (C) 2004, Pascal Schmidt
 * see file LICENSE for license details
 */

int gen_nonce(char *nonce);

void mnt_cmd_argument(char **dpath, const char *cmd, char *arg, size_t maxlen);

void otp_digest(char nonce[32], 
		char *password, 
		char hexdigest[32]);
