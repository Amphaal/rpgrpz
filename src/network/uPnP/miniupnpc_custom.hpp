#pragma once

/* $Id: upnpc.c,v 1.119 2018/03/13 23:34:46 nanard Exp $ */
/* Project : miniupnp
 * Author : Thomas Bernard
 * Copyright (c) 2005-2018 Thomas Bernard
 * This software is subject to the conditions detailed in the
 * LICENCE file provided in this distribution. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
	#include <time.h>
	#include <windows.h>
#else
/* for IPPROTO_TCP / IPPROTO_UDP */
	#include <sys/time.h>
	#include <netinet/in.h>
#endif
#include <ctype.h>
#include <miniupnpc.h>
#include <upnpcommands.h>
#include <portlistingparse.h>
#include <upnperrors.h>

/* protofix() checks if protocol is "UDP" or "TCP"
 * returns NULL if not */
static const char * protofix(const char * proto)
{
	static const char proto_tcp[4] = { 'T', 'C', 'P', 0};
	static const char proto_udp[4] = { 'U', 'D', 'P', 0};
	int i, b;
	for(i=0, b=1; i<4; i++)
		b = b && (   (proto[i] == proto_tcp[i])
		          || (proto[i] == (proto_tcp[i] | 32)) );
	if(b)
		return proto_tcp;
	for(i=0, b=1; i<4; i++)
		b = b && (   (proto[i] == proto_udp[i])
		          || (proto[i] == (proto_udp[i] | 32)) );
	if(b)
		return proto_udp;
	return 0;
}