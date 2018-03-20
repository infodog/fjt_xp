#ifndef IF_CONFIG_IP_H
#define IF_CONFIG_IP_H


void getlocalip(char *strip);
int ifconfigip(char *inet, int alias, char *ip);
int ip_is_local(char *ip);
int check_mac(char *lic);


#endif
