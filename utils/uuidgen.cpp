/********************************************************************************
*                                                                               *
*         G e n e r a t e   U n i v e r s a l l y   U n i q u e  I D ' s        *
*                                                                               *
*********************************************************************************
* Copyright (C) 1998 by Jeroen van der Zijp.   All Rights Reserved.             *
*********************************************************************************
* This library is free software; you can redistribute it and/or                 *
* modify it under the terms of the GNU Library General Public                   *
* License as published by the Free Software Foundation; either                  *
* version 2 of the License, or (at your option) any later version.              *
*                                                                               *
* This library is distributed in the hope that it will be useful,               *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU             *
* Library General Public License for more details.                              *
*                                                                               *
* You should have received a copy of the GNU Library General Public             *
* License along with this library; if not, write to the Free                    *
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            *
*********************************************************************************
* $Id: uuidgen.cpp,v 1.4 1998/05/29 02:25:56 jeroen Exp $                        *
********************************************************************************/
#include "xincs.h"
#include "fxdefs.h"



#include <fcntl.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif
#ifdef HAVE_NET_IF_H
#include <net/if.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_SRANDOM
#define srand(x) 	srandom(x)
#define rand() 		random()
#endif

#if defined(_POWER) || defined(_IBMR2)
extern "C" int gettimeofday(struct timeval *tv, struct timezone *tz);
#endif



/*

  To do:
  
  - This program should print these UUID's a bit differently.
  
*/

typedef FXuchar FXUUID[16];


/*
 * Offset between 15-Oct-1582 and 1-Jan-70
 */
#define TIME_OFFSET_HIGH 0x01B21DD2
#define TIME_OFFSET_LOW  0x13814000


struct uuid {
  FXuint    time_low;
  FXushort  time_mid;
  FXushort  time_hi_and_version;
  FXushort  clock_seq;
  FXuchar   node[6];
  };



void uuid_clear(FXUUID uu){
  memset(uu, 0, 16);
  }


void uuid_pack(struct uuid *uu, FXUUID ptr){
  FXuint   tmp;
  FXuchar   *out = ptr;

  tmp = uu->time_low;
  out[3] = (FXuchar) tmp;
  tmp >>= 8;
  out[2] = (FXuchar) tmp;
  tmp >>= 8;
  out[1] = (FXuchar) tmp;
  tmp >>= 8;
  out[0] = (FXuchar) tmp;

  tmp = uu->time_mid;
  out[5] = (FXuchar) tmp;
  tmp >>= 8;
  out[4] = (FXuchar) tmp;

  tmp = uu->time_hi_and_version;
  out[7] = (FXuchar) tmp;
  tmp >>= 8;
  out[6] = (FXuchar) tmp;

  tmp = uu->clock_seq;
  out[9] = (FXuchar) tmp;
  tmp >>= 8;
  out[8] = (FXuchar) tmp;

  memcpy(out+10, uu->node, 6);
  }


void uuid_unpack(FXUUID in, struct uuid *uu){
  FXuchar  *ptr = in;
  FXuint  tmp;

  tmp = *ptr++;
  tmp = (tmp << 8) | *ptr++;
  tmp = (tmp << 8) | *ptr++;
  tmp = (tmp << 8) | *ptr++;
  uu->time_low = tmp;

  tmp = *ptr++;
  tmp = (tmp << 8) | *ptr++;
  uu->time_mid = tmp;

  tmp = *ptr++;
  tmp = (tmp << 8) | *ptr++;
  uu->time_hi_and_version = tmp;

  tmp = *ptr++;
  tmp = (tmp << 8) | *ptr++;
  uu->clock_seq = tmp;

  memcpy(uu->node, ptr, 6);
  }


/*
 * Generate a series of random bytes.  Use /dev/urandom if possible,
 * and if not, use srandom/random.
 */
static void get_random_bytes(void *buf, int nbytes){
  char *cp=(char*)buf;
  static int fd=-2;
  int i;

  if(fd==-2){
    fd=open("/dev/urandom",O_RDONLY);
    srand((getpid()<<16)^getuid()^time(0));
    }
  if(fd>0){
    i=read(fd,cp,nbytes);
    if(i==nbytes) return;
    if(i>0) {
      nbytes-=i;
      cp+=i;
      }
    }
  for(i=0; i<nbytes; i++) *cp++ = rand()&0xFF;
  }


/*
 * Get the ethernet hardware address, if we can find it...
 */
static int get_node_id(unsigned char *node_id){
#ifdef HAVE_NET_IF_H
  int             sd;
  struct ifreq    ifr, *ifrp;
  struct ifconf   ifc;
  char buf[1024];
  int             n, i;
  unsigned char   *a;
	
/*
 * BSD 4.4 defines the size of an ifreq to be
 * max(sizeof(ifreq), sizeof(ifreq.ifr_name)+ifreq.ifr_addr.sa_len
 * However, under earlier systems, sa_len isn't present, so the size is 
 * just sizeof(struct ifreq)
 */
#ifdef HAVE_SA_LEN
#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#define ifreq_size(i) max(sizeof(struct ifreq), sizeof((i).ifr_name)+(i).ifr_addr.sa_len)
#else
#define ifreq_size(i) sizeof(struct ifreq)
#endif 

  sd=socket(AF_INET,SOCK_DGRAM,IPPROTO_IP);
  if(sd<0) {
    return -1;
    }
  memset(buf, 0, sizeof(buf));
  ifc.ifc_len = sizeof(buf);
  ifc.ifc_buf = buf;
  if(ioctl(sd,SIOCGIFCONF,(char *)&ifc)<0){
    close(sd);
    return -1;
    }
  n=ifc.ifc_len;
  for(i=0; i<n; i+=ifreq_size(*ifr)){
    ifrp = (struct ifreq *)((caddr_t)ifc.ifc_buf+i);
    strncpy(ifr.ifr_name, ifrp->ifr_name, IFNAMSIZ);
#ifdef SIOCGIFHWADDR
    if (ioctl(sd, SIOCGIFHWADDR, &ifr) < 0) continue;
    a = (unsigned char *) &ifr.ifr_hwaddr.sa_data;
#else
#ifdef SIOCGENADDR
    if (ioctl(sd, SIOCGENADDR, &ifr) < 0) continue;
    a = (unsigned char *) ifr.ifr_enaddr;
#else
    /*
     * XXX we don't have a way of getting the hardware
     * address
     */
    close(sd);
    return 0;
#endif 
#endif 
    if (!a[0] && !a[1] && !a[2] && !a[3] && !a[4] && !a[5]) continue;
    if (node_id) {
      memcpy(node_id, a, 6);
      close(sd);
      return 1;
      }
    }
  close(sd);
#endif
  return 0;
  }

/* Assume that the gettimeofday() has microsecond granularity */
#define MAX_ADJUSTMENT 10

static int get_clock(FXuint *clock_high, FXuint *clock_low, FXushort *ret_clock_seq){
  static int            adjustment=0;
  static struct timeval last={0,0};
  static FXushort       clock_seq;
  struct timeval        tv;
  unsigned long long    clock;
	
try_again:
  gettimeofday(&tv, 0);
  if((last.tv_sec==0) && (last.tv_usec==0)){
    get_random_bytes(&clock_seq,sizeof(clock_seq));
    clock_seq&=0x1FFF;
    last=tv;
    last.tv_sec--;
    }
  if((tv.tv_sec<last.tv_sec) || ((tv.tv_sec==last.tv_sec) && (tv.tv_usec<last.tv_usec))){
    clock_seq=(clock_seq+1)&0x1FFF;
    adjustment=0;
    } 
  else if((tv.tv_sec==last.tv_sec) && (tv.tv_usec==last.tv_usec)){
    if(adjustment>=MAX_ADJUSTMENT) goto try_again;
    adjustment++;
    } 
  else
    adjustment = 0;

  clock=tv.tv_usec*10+adjustment;
  clock+=((unsigned long long) tv.tv_sec)*10000000;
  clock+=(((unsigned long long) 0x01B21DD2)<<32)+0x13814000;    // bidibidibidi... right buck!
  *clock_high=clock>>32;
  *clock_low=clock;
  *ret_clock_seq=clock_seq;
  return 0;
  }



void uuid_generate(FXUUID out){
  static unsigned char node_id[6];
  static int has_init = 0;
  struct uuid uu;
  FXuint  clock_mid;

  if(!has_init) {
    if(get_node_id(node_id)<=0){
      get_random_bytes(node_id,6);
      /*
       * Set multicast bit, to prevent conflicts
       * with IEEE 802 addresses obtained from
       * network cards
       */
      node_id[0]|=0x80;
      }
    has_init=1;
    }
  get_clock(&clock_mid, &uu.time_low, &uu.clock_seq);
  uu.clock_seq|=0x8000;
  uu.time_mid = (FXushort) clock_mid;
  uu.time_hi_and_version = (clock_mid >> 16) | 0x1000;
  memcpy(uu.node, node_id, 6);
  uuid_pack(&uu, out);
  }



void uuid_unparse(FXUUID uu, char *out){
 struct uuid uuid;
 uuid_unpack(uu, &uuid);
 sprintf(out,"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
         uuid.time_low, uuid.time_mid, uuid.time_hi_and_version,
         uuid.clock_seq >> 8, uuid.clock_seq & 0xFF,
         uuid.node[0], uuid.node[1], uuid.node[2],
         uuid.node[3], uuid.node[4], uuid.node[5]);
  }


int main(int argc,char **argv){
  FXUUID uu;
  char uuidstring[1000];
  int i;

  uuid_clear(uu);

  uuid_generate(uu);

  uuid_unparse(uu,uuidstring);
  
  printf("Standard format UUID = %s\n",uuidstring);
  
  printf("Hex format UUID      = ");
  for(i=0; i<16; i++) printf("%02x",uu[i]);
  printf("\n");
  
  printf("Ascii format UUID    = \"");
  for(i=0; i<16; i++) printf("\\%03o",uu[i]);
  printf("\"\n");
  }


  
