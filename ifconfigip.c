#ifdef  WIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

void getlocalip(char *strip)
{
    char hostname[256];
    struct hostent *host;
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD( 2, 2 );
    err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 )
    {
        return;
    }
    if ( LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 )
    {
        WSACleanup( );
        return;
    }

    /*
    *old get linux local ip method
    */
    gethostname(hostname, sizeof(hostname));
    host = gethostbyname(hostname);
    strcpy(strip, inet_ntoa(*((struct in_addr*)host->h_addr)));
    WSACleanup();
}

typedef struct ip_list {
    char ip[64];
    struct ip_list *next;
} ip_list;

ip_list *g_it = NULL;

static int put_ip(ip_list **it, char *ip)
{
    ip_list *tail = NULL;
    ip_list *temp = NULL;

    if (it == NULL || ip == NULL) {
        return 0;
    }
    temp = (ip_list*) malloc(sizeof(ip_list));
    strcpy(temp->ip, ip);
    if (*it == NULL) {
        *it = temp;
        (*it)->next = NULL;
    }
    else {
        tail = *it;
        while (tail->next) {
            tail = tail->next;
        }
        temp->next = tail->next;
        tail->next = temp;
    }
    return 1;
}

static int make_ip_list(ip_list **it)
{
    int i, ret = 0;
    char szHostName[128];
    ip_list *tail = NULL;

    if (gethostname(szHostName, 128) == 0) {
        struct hostent *pHost;
        pHost = gethostbyname(szHostName);
        for(i = 0; pHost != NULL && pHost->h_addr_list[i] != NULL; i++) {
            char *nowip = inet_ntoa(*((struct in_addr*)pHost->h_addr_list[i]));
            if (put_ip(it, nowip)) {
                ++ret;
            }
        }
    }
    return ret;
}

static void free_ip_list(ip_list **it)
{
    while (it && *it) {
        ip_list *temp = (*it)->next;
        free(*it);
        *it = temp;
    }
}

int ip_is_local(char *ip)
{
    ip_list *temp = NULL;

    {
        WORD wVersionRequested;
        WSADATA wsaData;
        int err;
        wVersionRequested = MAKEWORD( 2, 2 );
        err = WSAStartup( wVersionRequested, &wsaData );
        if ( err != 0 )
        {
            return -1;
        }
        if ( LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 )
        {
            WSACleanup( );
            return -1;
        }
    }

    if (g_it == NULL) {
        make_ip_list(&g_it);
    }
    temp = g_it;
    while (temp) {
        if (strcmp(ip, temp->ip) == 0) {
            WSACleanup();
            return 1;
        }
        temp = temp->next;
    }

    WSACleanup();
    return -1;
}

#else

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "ifconfigip.h"

#if defined(sun) && defined(__svr4__)
#include <sys/sockio.h>
#define	IPSYMBOL	"hme0"
// #define socklen_t int
#else
#define	IPSYMBOL	"eth0"
#endif

#define	IFI_NAME	16					/* same as IFNAMSIZ in <net/if.h> */
#define	IFI_HADDR	8					/* allow for 64-bit EUI-64 in future */
#define	IFI_ALIAS	1					/* ifi_addr is an alias */


struct ifi_info
{
    char    ifi_name[IFI_NAME];			/* interface name, null terminated */
    u_char  ifi_haddr[IFI_HADDR];		/* hardware address */
    u_short ifi_hlen;					/* #bytes in hardware address: 0, 6, 8 */
    short   ifi_flags;					/* IFF_xxx constants from <net/if.h> */
    short   ifi_myflags;				/* our own IFI_xxx flags */
    struct  sockaddr  *ifi_addr;		/* primary address */
    struct  sockaddr  *ifi_brdaddr;		/* broadcast address */
    struct  sockaddr  *ifi_dstaddr; 	/* destination address */
    struct  ifi_info  *ifi_next;	 	/* next of these structures */
};

static int err_quit(char *s)
{
    printf("Get local ip failed: %s", s);
    exit(0);
}

static struct ifi_info* get_ifi_info(int family, int doaliases)
{
    struct ifi_info *ifi, *ifihead, **ifipnext;
    int sockfd, len, lastlen, flags, myflags;
    char *ptr, *buf, lastname[IFNAMSIZ], *cptr;
    struct ifconf ifc;
    struct ifreq *ifr, ifrcopy;
    struct sockaddr_in *sinptr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    lastlen = 0;
    len = 100 * sizeof(struct ifreq);	/* initial buffer size guess */
    for ( ; ; ) {
        buf = malloc(len);
        ifc.ifc_len = len;
        ifc.ifc_buf = buf;
        if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) {
        /*
        if (h_errno != EINVAL || lastlen != 0)
        err_sys("ioctl error");
            */
        } else {
            if (ifc.ifc_len == lastlen)
                break;										/* success, len has not changed */
            lastlen = ifc.ifc_len;
        }
        len += 10 * sizeof(struct ifreq);				    /* increment */
        free(buf);
    }
    ifihead = NULL;
    ifipnext = &ifihead;
    lastname[0] = 0;
    /* end get_ifi_info1 */

    /* include get_ifi_info2 */
    for (ptr = buf; ptr < buf + ifc.ifc_len; ) {
        ifr = (struct ifreq *) ptr;

#ifdef	HAVE_SOCKADDR_SA_LEN
        len = max(sizeof(struct sockaddr), ifr->ifr_addr.sa_len);
#else
        switch (ifr->ifr_addr.sa_family) {
#ifdef	IPV6
        case AF_INET6:
            len = sizeof(struct sockaddr_in6);
            break;
#endif
        case AF_INET:
        default:
            len = sizeof(struct sockaddr);
            break;
        }
#endif										/* HAVE_SOCKADDR_SA_LEN */
        ptr += sizeof(ifr->ifr_name) + len;	/* for next one in buffer */

        if (ifr->ifr_addr.sa_family != family)
            continue;						/* ignore if not desired address family */

        myflags = 0;
        if ( (cptr = strchr(ifr->ifr_name, ':')) != NULL)
            *cptr = 0;						/* replace colon will null */
        if (strncmp(lastname, ifr->ifr_name, IFNAMSIZ) == 0) {
            if (doaliases == 0)
                continue;					/* already processed this interface */
            myflags = IFI_ALIAS;
        }
        memcpy(lastname, ifr->ifr_name, IFNAMSIZ);

        ifrcopy = *ifr;
        ioctl(sockfd, SIOCGIFFLAGS, &ifrcopy);
        flags = ifrcopy.ifr_flags;
        if ((flags & IFF_UP) == 0)
            continue;						/* ignore if interface not up */

        ifi = calloc(1, sizeof(struct ifi_info));
        *ifipnext = ifi;					/* prev points to this new one */
        ifipnext = &ifi->ifi_next;			/* pointer to next one goes here */

        ifi->ifi_flags = flags;				/* IFF_xxx values */
        ifi->ifi_myflags = myflags;			/* IFI_xxx values */
        memcpy(ifi->ifi_name, ifr->ifr_name, IFI_NAME);
        ifi->ifi_name[IFI_NAME-1] = '\0';
        /* end get_ifi_info2 */
        /* include get_ifi_info3 */
        switch (ifr->ifr_addr.sa_family) {
        case AF_INET:
            sinptr = (struct sockaddr_in *) &ifr->ifr_addr;
            if (ifi->ifi_addr == NULL) {
                ifi->ifi_addr = calloc(1, sizeof(struct sockaddr_in));
                memcpy(ifi->ifi_addr, sinptr, sizeof(struct sockaddr_in));

#ifdef	SIOCGIFBRDADDR
                if (flags & IFF_BROADCAST) {
                    ioctl(sockfd, SIOCGIFBRDADDR, &ifrcopy);
                    sinptr = (struct sockaddr_in *) &ifrcopy.ifr_broadaddr;
                    ifi->ifi_brdaddr = calloc(1, sizeof(struct sockaddr_in));
                    memcpy(ifi->ifi_brdaddr, sinptr, sizeof(struct sockaddr_in));
                }
#endif

#ifdef	SIOCGIFDSTADDR
                if (flags & IFF_POINTOPOINT) {
                    ioctl(sockfd, SIOCGIFDSTADDR, &ifrcopy);
                    sinptr = (struct sockaddr_in *) &ifrcopy.ifr_dstaddr;
                    ifi->ifi_dstaddr = calloc(1, sizeof(struct sockaddr_in));
                    memcpy(ifi->ifi_dstaddr, sinptr, sizeof(struct sockaddr_in));
                }
#endif
            }
            break;

        default:
            break;
        }
    }
    free(buf);
    return(ifihead);						/* pointer to first structure in linked list */
}
/* end get_ifi_info3 */

/* include free_ifi_info */
static void free_ifi_info(struct ifi_info *ifihead)
{
    struct ifi_info	*ifi, *ifinext;

    for (ifi = ifihead; ifi != NULL; ifi = ifinext) {
        if (ifi->ifi_addr != NULL)
            free(ifi->ifi_addr);
        if (ifi->ifi_brdaddr != NULL)
            free(ifi->ifi_brdaddr);
        if (ifi->ifi_dstaddr != NULL)
            free(ifi->ifi_dstaddr);
        ifinext = ifi->ifi_next;			/* can't fetch ifi_next after free() */
        free(ifi);							/* the ifi_info{} itself */
    }
}
/* end free_ifi_info */

static struct ifi_info* Get_ifi_info(int family, int doaliases)
{
    struct ifi_info	*ifi;

    if ( (ifi = get_ifi_info(family, doaliases)) == NULL)
        err_quit("get_ifi_info error");
    return(ifi);
}

static char* sock_ntop_host(const struct sockaddr *sa, socklen_t salen)
{
    static char str[128];					/* Unix domain is largest */

    switch (sa->sa_family) {
    case AF_INET: {
        struct sockaddr_in	*sin = (struct sockaddr_in *) sa;

        if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
            return(NULL);
        return(str);
                  }

#ifdef	IPV6
        case AF_INET6: {
            struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;

            if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str)) == NULL)
                return(NULL);
            return(str);
                       }
#endif

                       /*
                       #ifdef	AF_UNIX
                       case AF_UNIX: {
                       struct sockaddr_un	*unp = (struct sockaddr_un *) sa;
                       if (unp->sun_path[0] == '\0')
                       strcpy(str, "(no pathname bound)");
                       else
                       snprintf(str, sizeof(str), "%s", unp->sun_path);
                       return(str);
                       }
                       #endif
            */

#ifdef	HAVE_SOCKADDR_DL_STRUCT
            case AF_LINK:
                {
                    struct sockaddr_dl *sdl = (struct sockaddr_dl *) sa;

                    if (sdl->sdl_nlen > 0)
                        snprintf(str, sizeof(str), "%*s",
                        sdl->sdl_nlen, &sdl->sdl_data[0]);
                    else
                        snprintf(str, sizeof(str), "AF_LINK, index=%d", sdl->sdl_index);
                    return(str);
                }
#endif
                    default:
                        snprintf(str, sizeof(str), "sock_ntop_host: unknown AF_xxx: %d, len %d", sa->sa_family, salen);
                        return(str);
    }

    return (NULL);
}

static char* Sock_ntop_host(const struct sockaddr *sa, socklen_t salen)
{
    char *ptr;

    if ((ptr = sock_ntop_host(sa, salen)) == NULL)
        err_quit("sock_ntop_host error");		/* inet_ntop() sets errno */

    return(ptr);
}

/* Export function ifconfigip() */

int ifconfigip(char *inet, int alias, char *ip)
{
    struct ifi_info	*ifi, *ifihead;
    struct sockaddr	*sa;
    u_char			*ptr;
    int				i, family, doaliases;

    if (inet == NULL)
        err_quit("<inet4 | inet6>");

    if (strcmp(inet, "inet4") == 0)
        family = AF_INET;
#ifdef	IPV6
    else if (strcmp(inet, "inet6") == 0)
        family = AF_INET6;
#endif
    else
        err_quit("invalid <address-family>");
    doaliases = alias;

    for (ifihead = ifi = Get_ifi_info(family, doaliases); ifi != NULL; ifi = ifi->ifi_next)
    {
        if (ifi->ifi_name && (strcmp(ifi->ifi_name, IPSYMBOL) == 0))
        {
            if ((sa = ifi->ifi_addr) != NULL)
            {
                strcpy(ip, Sock_ntop_host(sa, sizeof(*sa)));
                return 0;
            }
        }
    }
    free_ifi_info(ifihead);

    ip = NULL;
    return 1;
}

void getlocalip(char *strip)
{
    ifconfigip("inet4", 0, strip);
}


typedef struct ip_list {
    char ip[64];
    struct ip_list *next;
} ip_list;

ip_list *g_it = NULL;

int put_ip(ip_list **it, char *ip)
{
    ip_list *tail = NULL;
    ip_list *temp = NULL;

    if (it == NULL || ip == NULL) {
        return 0;
    }
    temp = (ip_list*) malloc(sizeof(ip_list));
    strcpy(temp->ip, ip);
    if (*it == NULL) {
        *it = temp;
        (*it)->next = NULL;
    }
    else {
        tail = *it;
        while (tail->next) {
            tail = tail->next;
        }
        temp->next = tail->next;
        tail->next = temp;
    }
    return 1;
}

int make_ip_list(ip_list **it)
{
    int i, ret = 0;
    char szHostName[128];
    ip_list *tail = NULL;

    if (gethostname(szHostName, 128) == 0) {
        struct hostent *pHost;
        pHost = gethostbyname(szHostName);
        for(i = 0; pHost != NULL && pHost->h_addr_list[i] != NULL; i++) {
            char *nowip = inet_ntoa(*((struct in_addr*)pHost->h_addr_list[i]));
            if (put_ip(it, nowip)) {
                ++ret;
            }
        }
    }
    return ret;
}

void free_ip_list(ip_list **it)
{
    while (it && *it) {
        ip_list *temp = (*it)->next;
        free(*it);
        *it = temp;
    }
}

int ifconfigiplist(char *inet, int alias, char *ip)
{
    struct ifi_info	*ifi, *ifihead;
    struct sockaddr	*sa;
    u_char			*ptr;
    int				i, family, doaliases;

    if (inet == NULL)
        err_quit("<inet4 | inet6>");

    if (strcmp(inet, "inet4") == 0)
        family = AF_INET;
#ifdef	IPV6
    else if (strcmp(inet, "inet6") == 0)
        family = AF_INET6;
#endif
    else
        err_quit("invalid <address-family>");
    doaliases = alias;

    for (ifihead = ifi = Get_ifi_info(family, doaliases); ifi != NULL; ifi = ifi->ifi_next)
    {
        if (ifi->ifi_name)
        {
            if ((sa = ifi->ifi_addr) != NULL)
            {
                strcpy(ip, Sock_ntop_host(sa, sizeof(*sa)));
                put_ip(&g_it, ip);
            }
        }
    }
    free_ifi_info(ifihead);

    ip = NULL;
    return 1;
}

int ip_is_local(char *ip)
{
    char strip[1024];
    ip_list *temp = NULL;

    if (g_it == NULL) {
        ifconfigiplist("inet4", 0, strip);
    }
    temp = g_it;
    while (temp) {
        if (strcmp(ip, temp->ip) == 0) {
            return 1;
        }
        temp = temp->next;
    }

    return -1;
}

#endif


/*
#pragma comment(lib, "ws2_32.lib")

  int main(int argc, char *argv[])
  {
  char strip[1024];
  // ifconfigiplist("inet4", 0, strip);

    getlocalip(strip);
    printf("ip is %s\n", strip);

      make_ip_list(&g_it);
      while (g_it) {
      printf(g_it->ip);
      printf("\n");
      g_it = g_it->next;
      }

        if (ip_is_local("10.10.10.25") < 0) {
        printf("ip err\n");
        }
        else {
        printf("ip ok\n");
        }
        }

//*/


#ifdef WIN32


#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <lm.h>
#include <assert.h>

#pragma comment(lib, "Netapi32.lib")

static void PrintMACaddress(unsigned char MACData[])
{
	printf("MAC Address: %02X-%02X-%02X-%02X-%02X-%02X\n",
		MACData[0], MACData[1], MACData[2], MACData[3], MACData[4], MACData[5]);
}

static void GetMACaddress(char *strMac)
{
	DWORD i;
	unsigned char MACData[8];
	WKSTA_TRANSPORT_INFO_0 *pwkti;
	DWORD dwEntriesRead;
	DWORD dwTotalEntries;
	BYTE *pbBuffer;

	NET_API_STATUS dwStatus = NetWkstaTransportEnum(
		NULL,
		0,
		&pbBuffer,
		MAX_PREFERRED_LENGTH,
		&dwEntriesRead,
		&dwTotalEntries,
		NULL);
    if (dwStatus != NERR_Success) {
        printf("Can't get Mac address!\n");
        getchar();
        exit(0);
    }
	assert(dwStatus == NERR_Success);
	pwkti = (WKSTA_TRANSPORT_INFO_0 *)pbBuffer;
	for(i=1; i< dwEntriesRead; i++)
	{
		swscanf((wchar_t *)pwkti[i].wkti0_transport_address, L"%2hx%2hx%2hx%2hx%2hx%2hx",
			&MACData[0], &MACData[1], &MACData[2], &MACData[3], &MACData[4], &MACData[5]);
		sprintf(strMac, "%02X-%02X-%02X-%02X-%02X-%02X",
			MACData[0], MACData[1], MACData[2], MACData[3], MACData[4], MACData[5]);
		PrintMACaddress(MACData);
	}
	dwStatus = NetApiBufferFree(pbBuffer);
	assert(dwStatus == NERR_Success);
}

int MACaddressOK(char *INstrMac)
{
	DWORD i;
	int flag = -1;
	char strMac[64];
	unsigned char MACData[8];
	WKSTA_TRANSPORT_INFO_0 *pwkti;
	DWORD dwEntriesRead;
	DWORD dwTotalEntries;
	BYTE *pbBuffer = NULL;

	NET_API_STATUS dwStatus = NetWkstaTransportEnum(
		NULL,
		0,
		&pbBuffer,
		MAX_PREFERRED_LENGTH,
		&dwEntriesRead,
		&dwTotalEntries,
		NULL);
    if (dwStatus != NERR_Success) {
		return -1;
    }
	pwkti = (WKSTA_TRANSPORT_INFO_0 *)pbBuffer;
	for(i=1; i< dwEntriesRead; i++) {
		swscanf((wchar_t *)pwkti[i].wkti0_transport_address, L"%2hx%2hx%2hx%2hx%2hx%2hx",
			&MACData[0], &MACData[1], &MACData[2], &MACData[3], &MACData[4], &MACData[5]);
		sprintf(strMac, "%02X-%02X-%02X-%02X-%02X-%02X",
			MACData[0], MACData[1], MACData[2], MACData[3], MACData[4], MACData[5]);
		if (stricmp(strMac, INstrMac) == 0) {
			flag = 0;
			break;
		}
	}
	dwStatus = NetApiBufferFree(pbBuffer);

	return flag;
}


#else


#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#ifdef __APPLE__
#include <net/if_dl.h>
#endif
#include <ifaddrs.h>
#include <unistd.h>

void change_MAC(unsigned char *p, int ether, char *szmac)
{
    #if defined(SIOCGIFHWADDR)
	int s,i;
	char *ptr = szmac;
	struct  ifreq  devea;

	*ptr = '\0';
	if ((s = socket(AF_INET,SOCK_DGRAM,0)) < 0) {
		return;
	}
	sprintf(devea.ifr_name, "eth%d", ether);
	if (ioctl(s,SIOCGIFHWADDR,&devea) < 0) {
		NULL;
	}
	else {
		for (i = 0; i < 6; i++) {
			if (i) *ptr++ = '-';
			sprintf(ptr, "%02X", i[devea.ifr_hwaddr.sa_data] & 0xff);
			ptr += 2;
		}
	}
	close(s);
    *ptr = '\0';
    #else
        struct ifaddrs* iflist;
        char if_name[32];
        char mac_addr[128];
        char *ptr = szmac;
        if (getifaddrs(&iflist) == 0) {
            sprintf(if_name, "eth%d", ether);
            for (struct ifaddrs* cur = iflist; cur; cur = cur->ifa_next) {
                if ((cur->ifa_addr->sa_family == AF_LINK) &&
                        (strcmp(cur->ifa_name, if_name) == 0) &&
                        cur->ifa_addr) {
                    struct sockaddr_dl* sdl = (struct sockaddr_dl*)cur->ifa_addr;
                    memcpy(mac_addr, LLADDR(sdl), sdl->sdl_alen);
                    for (int i = 0; i < 6; i++) {
                        if (i) *ptr++ = '-';
                        sprintf(ptr, "%02X", mac_addr[i] & 0xff);
                        ptr += 2;
                    }
                    break;
                }
            }
            freeifaddrs(iflist);
            *ptr = '\0';
        }
    #endif
}

void GetMACaddress(char *szmac)
{
	int ether = 0;
	unsigned char mac[6] = "\0\0\0\0\0\0";
	change_MAC(mac, ether, szmac);
}

int MACaddressOK(char *INstrMac)
{
	int ether = 0;
	char szmac[64];

	for ( ; ; ) {
		unsigned char mac[6] = "\0\0\0\0\0\0";
		change_MAC(mac, ether, szmac);
		if (strlen(szmac) < 1) {
			return -1;
		}
		if (strcasecmp(szmac, INstrMac) == 0) {
			return 0;
		}
		++ether;
	}
}


#endif


#include "config.h"


static char * strnistr(char *s1, char *s2, int n)
{
    int i;
    int l;
    l = strlen(s2);
    for(i=0; i<n; i++)
    {
        if (strnicmp(s1+i, s2, l)==0)
        {
            return s1+i;
        }
    }
    return NULL;
}


// <BindMAC><00-FF-D7-CD-18-BB>
int check_mac(char *lic)
{
	char mac[1024];
	char *ptr = NULL;
	char *pend = NULL;

	ptr = strnistr(lic, "<BindMAC><", strlen(lic));
	if (ptr == NULL) {
		return 0;
	}
	ptr += 10;
	pend = strchr(ptr, '>');
	if (pend == NULL) {
		return 0;
	}
	if (pend - ptr > sizeof(mac)) {
		return 0;
	}
	memcpy(mac, ptr, pend - ptr);
	mac[pend - ptr] = '\0';
	ptr = mac;
	while (*ptr != '\0') {
		if (*ptr == ':') {
			*ptr = '-';
		}
		++ptr;
	}

	return MACaddressOK(mac);
	// return 0;
}


/*
int main(int argc, char* argv[])
{
int r = 0;
char buff[1024];

  GetMACaddress(buff);
  printf("Mac address: <%s>\n", buff);
  if (argc > 1) {
		r = MACaddressOK(argv[1]);
		printf("result : %d\n", r);
		}

		  getchar();
		  return 0;
		  }
//*/

