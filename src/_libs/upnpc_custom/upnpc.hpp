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
#include <time.h>
#ifdef _WIN32
#include <winsock2.h>
#else
/* for IPPROTO_TCP / IPPROTO_UDP */
#include <netinet/in.h>
#endif
#include <ctype.h>
#include "miniupnpc.h"
#include "upnpcommands.h"
#include "portlistingparse.h"
#include "upnperrors.h"

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

/* is_int() checks if parameter is an integer or not
 * 1 for integer
 * 0 for not an integer */
static int is_int(char const* s)
{
	if(s == NULL)
		return 0;
	while(*s) {
		/* #define isdigit(c) ((c) >= '0' && (c) <= '9') */
		if(!isdigit(*s))
			return 0;
		s++;
	}
	return 1;
}

static void DisplayInfos(struct UPNPUrls * urls,
                         struct IGDdatas * data)
{
	char externalIPAddress[40];
	char connectionType[64];
	char status[64];
	char lastconnerr[64];
	unsigned int uptime = 0;
	unsigned int brUp, brDown;
	time_t timenow, timestarted;
	int r;
	if(UPNP_GetConnectionTypeInfo(urls->controlURL,
	                              data->first.servicetype,
	                              connectionType) != UPNPCOMMAND_SUCCESS)
		printf("GetConnectionTypeInfo failed.\n");
	else
		printf("Connection Type : %s\n", connectionType);
	if(UPNP_GetStatusInfo(urls->controlURL, data->first.servicetype,
	                      status, &uptime, lastconnerr) != UPNPCOMMAND_SUCCESS)
		printf("GetStatusInfo failed.\n");
	else
		printf("Status : %s, uptime=%us, LastConnectionError : %s\n",
		       status, uptime, lastconnerr);
	if(uptime > 0) {
		timenow = time(NULL);
		timestarted = timenow - uptime;
		char tt[26];
		ctime_s(tt, sizeof(tt), &timestarted);
		printf("  Time started : %s", tt);
	}
	if(UPNP_GetLinkLayerMaxBitRates(urls->controlURL_CIF, data->CIF.servicetype,
	                                &brDown, &brUp) != UPNPCOMMAND_SUCCESS) {
		printf("GetLinkLayerMaxBitRates failed.\n");
	} else {
		printf("MaxBitRateDown : %u bps", brDown);
		if(brDown >= 1000000) {
			printf(" (%u.%u Mbps)", brDown / 1000000, (brDown / 100000) % 10);
		} else if(brDown >= 1000) {
			printf(" (%u Kbps)", brDown / 1000);
		}
		printf("   MaxBitRateUp %u bps", brUp);
		if(brUp >= 1000000) {
			printf(" (%u.%u Mbps)", brUp / 1000000, (brUp / 100000) % 10);
		} else if(brUp >= 1000) {
			printf(" (%u Kbps)", brUp / 1000);
		}
		printf("\n");
	}
	r = UPNP_GetExternalIPAddress(urls->controlURL,
	                          data->first.servicetype,
							  externalIPAddress);
	if(r != UPNPCOMMAND_SUCCESS) {
		printf("GetExternalIPAddress failed. (errorcode=%d)\n", r);
	} else {
		printf("ExternalIPAddress = %s\n", externalIPAddress);
	}
}

static void GetConnectionStatus(struct UPNPUrls * urls,
                               struct IGDdatas * data)
{
	unsigned int bytessent, bytesreceived, packetsreceived, packetssent;
	DisplayInfos(urls, data);
	bytessent = UPNP_GetTotalBytesSent(urls->controlURL_CIF, data->CIF.servicetype);
	bytesreceived = UPNP_GetTotalBytesReceived(urls->controlURL_CIF, data->CIF.servicetype);
	packetssent = UPNP_GetTotalPacketsSent(urls->controlURL_CIF, data->CIF.servicetype);
	packetsreceived = UPNP_GetTotalPacketsReceived(urls->controlURL_CIF, data->CIF.servicetype);
	printf("Bytes:   Sent: %8u\tRecv: %8u\n", bytessent, bytesreceived);
	printf("Packets: Sent: %8u\tRecv: %8u\n", packetssent, packetsreceived);
}

static void ListRedirections(struct UPNPUrls * urls,
                             struct IGDdatas * data)
{
	int r;
	int i = 0;
	char index[6];
	char intClient[40];
	char intPort[6];
	char extPort[6];
	char protocol[4];
	char desc[80];
	char enabled[6];
	char rHost[64];
	char duration[16];
	/*unsigned int num=0;
	UPNP_GetPortMappingNumberOfEntries(urls->controlURL, data->servicetype, &num);
	printf("PortMappingNumberOfEntries : %u\n", num);*/
	printf(" i protocol exPort->inAddr:inPort description remoteHost leaseTime\n");
	do {
		snprintf(index, 6, "%d", i);
		rHost[0] = '\0'; enabled[0] = '\0';
		duration[0] = '\0'; desc[0] = '\0';
		extPort[0] = '\0'; intPort[0] = '\0'; intClient[0] = '\0';
		r = UPNP_GetGenericPortMappingEntry(urls->controlURL,
		                               data->first.servicetype,
		                               index,
		                               extPort, intClient, intPort,
									   protocol, desc, enabled,
									   rHost, duration);
		if(r==0)
		/*
			printf("%02d - %s %s->%s:%s\tenabled=%s leaseDuration=%s\n"
			       "     desc='%s' rHost='%s'\n",
			       i, protocol, extPort, intClient, intPort,
				   enabled, duration,
				   desc, rHost);
				   */
			printf("%2d %s %5s->%s:%-5s '%s' '%s' %s\n",
			       i, protocol, extPort, intClient, intPort,
			       desc, rHost, duration);
		else
			printf("GetGenericPortMappingEntry() returned %d (%s)\n",
			       r, strupnperror(r));
		i++;
	} while(r==0);
}

static void NewListRedirections(struct UPNPUrls * urls,
                                struct IGDdatas * data)
{
	int r;
	int i = 0;
	struct PortMappingParserData pdata;
	struct PortMapping * pm;

	memset(&pdata, 0, sizeof(struct PortMappingParserData));
	r = UPNP_GetListOfPortMappings(urls->controlURL,
                                   data->first.servicetype,
	                               "0",
	                               "65535",
	                               "TCP",
	                               "1000",
	                               &pdata);
	if(r == UPNPCOMMAND_SUCCESS)
	{
		printf(" i protocol exPort->inAddr:inPort description remoteHost leaseTime\n");
		for(pm = pdata.l_head; pm != NULL; pm = pm->l_next)
		{
			printf("%2d %s %5hu->%s:%-5hu '%s' '%s' %u\n",
			       i, pm->protocol, pm->externalPort, pm->internalClient,
			       pm->internalPort,
			       pm->description, pm->remoteHost,
			       (unsigned)pm->leaseTime);
			i++;
		}
		FreePortListing(&pdata);
	}
	else
	{
		printf("GetListOfPortMappings() returned %d (%s)\n",
		       r, strupnperror(r));
	}
	r = UPNP_GetListOfPortMappings(urls->controlURL,
                                   data->first.servicetype,
	                               "0",
	                               "65535",
	                               "UDP",
	                               "1000",
	                               &pdata);
	if(r == UPNPCOMMAND_SUCCESS)
	{
		for(pm = pdata.l_head; pm != NULL; pm = pm->l_next)
		{
			printf("%2d %s %5hu->%s:%-5hu '%s' '%s' %u\n",
			       i, pm->protocol, pm->externalPort, pm->internalClient,
			       pm->internalPort,
			       pm->description, pm->remoteHost,
			       (unsigned)pm->leaseTime);
			i++;
		}
		FreePortListing(&pdata);
	}
	else
	{
		printf("GetListOfPortMappings() returned %d (%s)\n",
		       r, strupnperror(r));
	}
}

/* Test function
 * 1 - get connection type
 * 2 - get extenal ip address
 * 3 - Add port mapping
 * 4 - get this port mapping from the IGD */
static int SetRedirectAndTest(struct UPNPUrls * urls,
			       struct IGDdatas * data,
			       const char * iaddr,
			       const char * iport,
			       const char * eport,
			       const char * proto,
			       const char * leaseDuration,
			       const char * description,
			       int addAny)
{
	char externalIPAddress[40];
	char intClient[40];
	char intPort[6];
	char reservedPort[6];
	char duration[16];
	int r;

	if(!iaddr || !iport || !eport || !proto)
	{
		fprintf(stderr, "Wrong arguments\n");
		return -1;
	}
	proto = protofix(proto);
	if(!proto)
	{
		fprintf(stderr, "invalid protocol\n");
		return -1;
	}

	r = UPNP_GetExternalIPAddress(urls->controlURL,
				      data->first.servicetype,
				      externalIPAddress);
	if(r!=UPNPCOMMAND_SUCCESS)
		printf("GetExternalIPAddress failed.\n");
	else
		printf("ExternalIPAddress = %s\n", externalIPAddress);

	if (addAny) {
		r = UPNP_AddAnyPortMapping(urls->controlURL, data->first.servicetype,
					   eport, iport, iaddr, description,
					   proto, 0, leaseDuration, reservedPort);
		if(r==UPNPCOMMAND_SUCCESS)
			eport = reservedPort;
		else
			printf("AddAnyPortMapping(%s, %s, %s) failed with code %d (%s)\n",
			       eport, iport, iaddr, r, strupnperror(r));
	} else {
		r = UPNP_AddPortMapping(urls->controlURL, data->first.servicetype,
					eport, iport, iaddr, description,
					proto, 0, leaseDuration);
		if(r!=UPNPCOMMAND_SUCCESS) {
			printf("AddPortMapping(%s, %s, %s) failed with code %d (%s)\n",
			       eport, iport, iaddr, r, strupnperror(r));
			return -2;
	}
	}

	r = UPNP_GetSpecificPortMappingEntry(urls->controlURL,
					     data->first.servicetype,
					     eport, proto, NULL/*remoteHost*/,
					     intClient, intPort, NULL/*desc*/,
					     NULL/*enabled*/, duration);
	if(r!=UPNPCOMMAND_SUCCESS) {
		printf("GetSpecificPortMappingEntry() failed with code %d (%s)\n",
		       r, strupnperror(r));
		return -2;
	} else {
		printf("InternalIP:Port = %s:%s\n", intClient, intPort);
		printf("external %s:%s %s is redirected to internal %s:%s (duration=%s)\n",
		       externalIPAddress, eport, proto, intClient, intPort, duration);
	}
	return 0;
}

static int
RemoveRedirect(struct UPNPUrls * urls,
               struct IGDdatas * data,
               const char * eport,
               const char * proto,
               const char * remoteHost)
{
	int r;
	if(!proto || !eport)
	{
		fprintf(stderr, "invalid arguments\n");
		return -1;
	}
	proto = protofix(proto);
	if(!proto)
	{
		fprintf(stderr, "protocol invalid\n");
		return -1;
	}
	r = UPNP_DeletePortMapping(urls->controlURL, data->first.servicetype, eport, proto, remoteHost);
	if(r!=UPNPCOMMAND_SUCCESS) {
		printf("UPNP_DeletePortMapping() failed with code : %d\n", r);
		return -2;
	}else {
		printf("UPNP_DeletePortMapping() returned : %d\n", r);
	}
	return 0;
}

static int
RemoveRedirectRange(struct UPNPUrls * urls,
		    struct IGDdatas * data,
		    const char * ePortStart, char const * ePortEnd,
		    const char * proto, const char * manage)
{
	int r;

	if (!manage)
		manage = "0";

	if(!proto || !ePortStart || !ePortEnd)
	{
		fprintf(stderr, "invalid arguments\n");
		return -1;
	}
	proto = protofix(proto);
	if(!proto)
	{
		fprintf(stderr, "protocol invalid\n");
		return -1;
	}
	r = UPNP_DeletePortMappingRange(urls->controlURL, data->first.servicetype, ePortStart, ePortEnd, proto, manage);
	if(r!=UPNPCOMMAND_SUCCESS) {
		printf("UPNP_DeletePortMappingRange() failed with code : %d\n", r);
		return -2;
	}else {
		printf("UPNP_DeletePortMappingRange() returned : %d\n", r);
	}
	return 0;
}

/* IGD:2, functions for service WANIPv6FirewallControl:1 */
static void GetFirewallStatus(struct UPNPUrls * urls, struct IGDdatas * data)
{
	unsigned int bytessent, bytesreceived, packetsreceived, packetssent;
	int firewallEnabled = 0, inboundPinholeAllowed = 0;

	UPNP_GetFirewallStatus(urls->controlURL_6FC, data->IPv6FC.servicetype, &firewallEnabled, &inboundPinholeAllowed);
	printf("FirewallEnabled: %d & Inbound Pinhole Allowed: %d\n", firewallEnabled, inboundPinholeAllowed);
	printf("GetFirewallStatus:\n   Firewall Enabled: %s\n   Inbound Pinhole Allowed: %s\n", (firewallEnabled)? "Yes":"No", (inboundPinholeAllowed)? "Yes":"No");

	bytessent = UPNP_GetTotalBytesSent(urls->controlURL_CIF, data->CIF.servicetype);
	bytesreceived = UPNP_GetTotalBytesReceived(urls->controlURL_CIF, data->CIF.servicetype);
	packetssent = UPNP_GetTotalPacketsSent(urls->controlURL_CIF, data->CIF.servicetype);
	packetsreceived = UPNP_GetTotalPacketsReceived(urls->controlURL_CIF, data->CIF.servicetype);
	printf("Bytes:   Sent: %8u\tRecv: %8u\n", bytessent, bytesreceived);
	printf("Packets: Sent: %8u\tRecv: %8u\n", packetssent, packetsreceived);
}

/* Test function
 * 1 - Add pinhole
 * 2 - Check if pinhole is working from the IGD side */
static void SetPinholeAndTest(struct UPNPUrls * urls, struct IGDdatas * data,
					const char * remoteaddr, const char * eport,
					const char * intaddr, const char * iport,
					const char * proto, const char * lease_time)
{
	char uniqueID[8];
	/*int isWorking = 0;*/
	int r;
	char proto_tmp[8];

	if(!intaddr || !remoteaddr || !iport || !eport || !proto || !lease_time)
	{
		fprintf(stderr, "Wrong arguments\n");
		return;
	}
	if(atoi(proto) == 0)
	{
		const char * protocol;
		protocol = protofix(proto);
		if(protocol && (strcmp("TCP", protocol) == 0))
		{
			snprintf(proto_tmp, sizeof(proto_tmp), "%d", IPPROTO_TCP);
			proto = proto_tmp;
		}
		else if(protocol && (strcmp("UDP", protocol) == 0))
		{
			snprintf(proto_tmp, sizeof(proto_tmp), "%d", IPPROTO_UDP);
			proto = proto_tmp;
		}
		else
		{
			fprintf(stderr, "invalid protocol\n");
			return;
		}
	}
	r = UPNP_AddPinhole(urls->controlURL_6FC, data->IPv6FC.servicetype, remoteaddr, eport, intaddr, iport, proto, lease_time, uniqueID);
	if(r!=UPNPCOMMAND_SUCCESS)
		printf("AddPinhole([%s]:%s -> [%s]:%s) failed with code %d (%s)\n",
		       remoteaddr, eport, intaddr, iport, r, strupnperror(r));
	else
	{
		printf("AddPinhole: ([%s]:%s -> [%s]:%s) / Pinhole ID = %s\n",
		       remoteaddr, eport, intaddr, iport, uniqueID);
		/*r = UPNP_CheckPinholeWorking(urls->controlURL_6FC, data->servicetype_6FC, uniqueID, &isWorking);
		if(r!=UPNPCOMMAND_SUCCESS)
			printf("CheckPinholeWorking() failed with code %d (%s)\n", r, strupnperror(r));
		printf("CheckPinholeWorking: Pinhole ID = %s / IsWorking = %s\n", uniqueID, (isWorking)? "Yes":"No");*/
	}
}

/* Test function
 * 1 - Check if pinhole is working from the IGD side
 * 2 - Update pinhole */
static void GetPinholeAndUpdate(struct UPNPUrls * urls, struct IGDdatas * data,
					const char * uniqueID, const char * lease_time)
{
	int isWorking = 0;
	int r;

	if(!uniqueID || !lease_time)
	{
		fprintf(stderr, "Wrong arguments\n");
		return;
	}
	r = UPNP_CheckPinholeWorking(urls->controlURL_6FC, data->IPv6FC.servicetype, uniqueID, &isWorking);
	printf("CheckPinholeWorking: Pinhole ID = %s / IsWorking = %s\n", uniqueID, (isWorking)? "Yes":"No");
	if(r!=UPNPCOMMAND_SUCCESS)
		printf("CheckPinholeWorking() failed with code %d (%s)\n", r, strupnperror(r));
	if(isWorking || r==709)
	{
		r = UPNP_UpdatePinhole(urls->controlURL_6FC, data->IPv6FC.servicetype, uniqueID, lease_time);
		printf("UpdatePinhole: Pinhole ID = %s with Lease Time: %s\n", uniqueID, lease_time);
		if(r!=UPNPCOMMAND_SUCCESS)
			printf("UpdatePinhole: ID (%s) failed with code %d (%s)\n", uniqueID, r, strupnperror(r));
	}
}

/* Test function
 * Get pinhole timeout
 */
static void GetPinholeOutboundTimeout(struct UPNPUrls * urls, struct IGDdatas * data,
					const char * remoteaddr, const char * eport,
					const char * intaddr, const char * iport,
					const char * proto)
{
	int timeout = 0;
	int r;

	if(!intaddr || !remoteaddr || !iport || !eport || !proto)
	{
		fprintf(stderr, "Wrong arguments\n");
		return;
	}

	r = UPNP_GetOutboundPinholeTimeout(urls->controlURL_6FC, data->IPv6FC.servicetype, remoteaddr, eport, intaddr, iport, proto, &timeout);
	if(r!=UPNPCOMMAND_SUCCESS)
		printf("GetOutboundPinholeTimeout([%s]:%s -> [%s]:%s) failed with code %d (%s)\n",
		       intaddr, iport, remoteaddr, eport, r, strupnperror(r));
	else
		printf("GetOutboundPinholeTimeout: ([%s]:%s -> [%s]:%s) / Timeout = %d\n", intaddr, iport, remoteaddr, eport, timeout);
}

static void
GetPinholePackets(struct UPNPUrls * urls,
               struct IGDdatas * data, const char * uniqueID)
{
	int r, pinholePackets = 0;
	if(!uniqueID)
	{
		fprintf(stderr, "invalid arguments\n");
		return;
	}
	r = UPNP_GetPinholePackets(urls->controlURL_6FC, data->IPv6FC.servicetype, uniqueID, &pinholePackets);
	if(r!=UPNPCOMMAND_SUCCESS)
		printf("GetPinholePackets() failed with code %d (%s)\n", r, strupnperror(r));
	else
		printf("GetPinholePackets: Pinhole ID = %s / PinholePackets = %d\n", uniqueID, pinholePackets);
}

static void
CheckPinhole(struct UPNPUrls * urls,
               struct IGDdatas * data, const char * uniqueID)
{
	int r, isWorking = 0;
	if(!uniqueID)
	{
		fprintf(stderr, "invalid arguments\n");
		return;
	}
	r = UPNP_CheckPinholeWorking(urls->controlURL_6FC, data->IPv6FC.servicetype, uniqueID, &isWorking);
	if(r!=UPNPCOMMAND_SUCCESS)
		printf("CheckPinholeWorking() failed with code %d (%s)\n", r, strupnperror(r));
	else
		printf("CheckPinholeWorking: Pinhole ID = %s / IsWorking = %s\n", uniqueID, (isWorking)? "Yes":"No");
}

static void
RemovePinhole(struct UPNPUrls * urls,
               struct IGDdatas * data, const char * uniqueID)
{
	int r;
	if(!uniqueID)
	{
		fprintf(stderr, "invalid arguments\n");
		return;
	}
	r = UPNP_DeletePinhole(urls->controlURL_6FC, data->IPv6FC.servicetype, uniqueID);
	printf("UPNP_DeletePinhole() returned : %d\n", r);
}