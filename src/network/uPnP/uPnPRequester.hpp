// RPG-RPZ
// A simple Pen & Paper experience
// Copyright (C) 2019-2020 Guillaume Vara

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Any graphical or audio resources available within the source code may
// use a different license and copyright : please refer to their metadata
// for further details. Resources without explicit references to a
// different license and copyright still refer to this GPL.

/* $Id: upnpc.c,v 1.119 2018/03/13 23:34:46 nanard Exp $ */
/* Project : miniupnp
 * Author : Thomas Bernard
 * Copyright (c) 2005-2018 Thomas Bernard
 * This software is subject to the conditions detailed in the
 * LICENCE file provided in this distribution. */

#pragma once

#include "uPnPThread.hpp"

#include <QString>

#include "miniupnpc_custom.hpp"

class uPnPRequester : public uPnPThread {
    
 public:
        struct UPNPUrls urls;
        struct IGDdatas data;
        struct UPNPDev * devlist = 0;
        char lanaddr[64] = "unset";	/* my ip address on the LAN */
        int i;
        char * rootdescurl = 0;
        char * multicastif = 0;
        char * minissdpdpath = 0;
        int localport = UPNP_LOCAL_PORT_ANY;
        int retcode = 0;
        int error = 0;
        int ipv6 = 0;
        unsigned char ttl = 2;	/* defaulting to 2 */
        QString description = 0;
        QString targetPort = 0;

        uPnPRequester(const QString &tPort, const QString &descr) : description(descr), targetPort(tPort) {}
        
        void run() override {
            try {

                //init uPnP...
                this->_initUPnP();
                if(retcode != 0) {
                    emit uPnPError(retcode);
                    return;
                }

                auto resultTCP = this->SetRedirectAndTest(this->lanaddr, this->targetPort, this->targetPort, "TCP", "0", 0);
                auto resultUDP = this->SetRedirectAndTest(this->lanaddr, this->targetPort, this->targetPort, "UDP", "0", 0);

                if(resultTCP != 0 || resultUDP != 0) {
                    emit uPnPError(-999);
                    return;
                } else {
                    emit uPnPSuccess("TCP+UDP", this->targetPort);
                }
            
            } catch(...) { 
                qDebug() << "UPNP run : exception caught while processing";
            }

        }

        ~uPnPRequester() {

                //remove any redirect
                if(retcode == 0) {
                    this->RemoveRedirect(this->targetPort, "TCP", NULL);

                    /*free*/
                    FreeUPNPUrls(&urls);
                    freeUPNPDevlist(devlist); devlist = 0;
                }
                
                /*End websock*/
                #ifdef _WIN32
                    WSACleanup();
                #endif        
        }


 private:
        void _initUPnP() {

            #ifdef _WIN32
                /*start websock*/
                WSADATA wsaData;
                int nResult = WSAStartup(MAKEWORD(2,2), &wsaData);
                if(nResult != NO_ERROR) {
                    qDebug() << "UPNP Inst : Cannot init socket with WSAStartup !";
                };
            #endif
            
            /*discover*/
            if( rootdescurl || (devlist = upnpDiscover(2000, multicastif, minissdpdpath, localport, ipv6, ttl, &error))) {
                
                struct UPNPDev * device;
                
                if(devlist) {

                    qDebug() << "UPNP Inst : List of UPNP devices found on the network :";
                    
                    for(device = devlist; device; device = device->pNext) {
                        qDebug() << "UPNP Inst : desc:" << device->descURL << "st:" << device->st;
                    }

                } else if(!rootdescurl) {
                    qDebug() << "UPNP Inst : upnpDiscover() error code=" << error;
                }

                i = 1;

                if( (rootdescurl && UPNP_GetIGDFromUrl(rootdescurl, &urls, &data, lanaddr, sizeof(lanaddr)))
                || (i = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr)))) {
                    
                    switch(i) {
                        case 1:
                            qDebug() << "UPNP Inst : Found valid IGD :" << urls.controlURL;
                            break;
                        case 2:
                            qDebug() << "UPNP Inst : Found a (not connected?) IGD :" << urls.controlURL;
                            qDebug() << "UPNP Inst : Trying to continue anyway";
                            break;
                        case 3:
                            qDebug() << "UPNP Inst : UPnP device found. Is it an IGD ? :" << urls.controlURL;
                            qDebug() << "UPNP Inst : Trying to continue anyway";
                            break;
                        default:
                            qDebug() << "UPNP Inst : Found device (igd ?) :" << urls.controlURL;
                            qDebug() << "UPNP Inst : Trying to continue anyway";
                    }

                    qDebug() << "UPNP Inst : Local LAN ip address" << lanaddr;

                    char externalIPAddress[40];
                    int r = UPNP_GetExternalIPAddress(urls.controlURL,
                        data.first.servicetype,
                        externalIPAddress);
                    if(r != UPNPCOMMAND_SUCCESS)
                        qDebug() << "UPNP AskRedirect : GetExternalIPAddress No IGD UPnP Device. ";
                    else {
                        qDebug() << "UPNP AskRedirect : ExternalIPAddress =" << externalIPAddress;
                        emit uPnPExtIpFound(externalIPAddress);
                    }

                }
                else {
                    qDebug() << "UPNP Inst : No valid UPNP Internet Gateway Device found.";
                    retcode = 1;
                }
            }
            else {
                qDebug() << "UPNP Inst : No IGD UPnP Device found on the network !";
                retcode = 1;
            }
        }

        int SetRedirectAndTest(const char * iaddr,
                        const QString &iport,
                        QString eport,
                        const char * proto,
                        const char * leaseDuration,
                        int addAny) {

            char externalIPAddress[40];
            char intClient[40];
            char intPort[6];
            char reservedPort[6];
            char duration[16];
            int r;

            if(!iaddr || iport.isEmpty() || eport.isEmpty() || !proto) {
                qDebug() << "UPNP AskRedirect : Wrong arguments";
                return -1;
            }
            proto = protofix(proto);
            if(!proto) {
                qDebug() << "UPNP AskRedirect : invalid protocol";
                return -1;
            }

            r = UPNP_GetExternalIPAddress(urls.controlURL,
                            data.first.servicetype,
                            externalIPAddress);
            if(r!=UPNPCOMMAND_SUCCESS)
                qDebug() << "UPNP AskRedirect : GetExternalIPAddress No IGD UPnP Device.";

            if (addAny) {
                r = UPNP_AddAnyPortMapping(
                            urls.controlURL, 
                            data.first.servicetype,
                            qUtf8Printable(eport), 
                            qUtf8Printable(iport), 
                            iaddr, 
                            qUtf8Printable(description),
                            proto, 
                            0, 
                            leaseDuration, 
                            reservedPort
                    );
                if(r==UPNPCOMMAND_SUCCESS)
                    eport = reservedPort;
                else
                    qDebug() << "UPNP AskRedirect : AddAnyPortMapping(" << eport << "," << iport << "," << iaddr << ") failed with code" << r << "(" << strupnperror(r) << ")";
            } else {
                r = UPNP_AddPortMapping(
                        urls.controlURL, 
                        data.first.servicetype,
                        qUtf8Printable(eport), 
                        qUtf8Printable(iport), 
                        iaddr, 
                        qUtf8Printable(description),
                        proto, 
                        NULL/*remoteHost*/, 
                        leaseDuration
                    );
                if(r!=UPNPCOMMAND_SUCCESS) {
                    qDebug() << "UPNP AskRedirect : AddPortMapping(" << eport << "," << iport << "," << iaddr << ") failed with code" << r << "(" << strupnperror(r) << ")";
                    return -2;
                }
            }

            r = UPNP_GetSpecificPortMappingEntry(
                    urls.controlURL,
                    data.first.servicetype,
                    qUtf8Printable(eport), 
                    proto, 
                    NULL/*remoteHost*/,
                    intClient, 
                    intPort, 
                    NULL/*desc*/,
                    NULL/*enabled*/, 
                    duration
                );
            if(r!=UPNPCOMMAND_SUCCESS) {
                qDebug() << "UPNP AskRedirect : GetSpecificPortMappingEntry() failed with code" << r <<"(" << strupnperror(r) << ")";
                return -2;
            } else {
                qDebug() << "UPNP AskRedirect : external" << externalIPAddress << ":" << eport << proto << "is redirected to internal" << intClient << ":" << intPort << "(duration=" << duration << ")";
            }
            return 0;
        }

        int RemoveRedirect(const QString &eport,
                    const char * proto,
                    const char * remoteHost) {
            int r;
            if(!proto || eport.isEmpty())
            {
                qDebug() << "UPNP RemoveRedirect : invalid arguments";
                return -1;
            }
            proto = protofix(proto);
            if(!proto)
            {
                qDebug() << "UPNP RemoveRedirect : protocol invalid";
                return -1;
            }
            r = UPNP_DeletePortMapping(
                urls.controlURL, 
                data.first.servicetype, 
                qUtf8Printable(eport), 
                proto, 
                remoteHost
            );
            if(r!=UPNPCOMMAND_SUCCESS) {
                qDebug() << "UPNP RemoveRedirect : UPNP_DeletePortMapping() failed with code :" << r << "";
                return -2;
            }else {
                qDebug() << "UPNP RemoveRedirect : UPNP_DeletePortMapping() returned :" << r << "";
            }
            return 0;
        }

        void ListRedirections() {
            int r;
            i = 0;
            char index[6];
            char intClient[40];
            char intPort[6];
            char extPort[6];
            char protocol[4];
            char desc[80];
            char enabled[6];
            char rHost[64];
            char duration[16];

            qDebug() << "UPNP List : i protocol exPort->inAddr:inPort description remoteHost leaseTime";

            do {
                snprintf(index, 6, "%d", i);
                rHost[0] = '\0'; enabled[0] = '\0';
                duration[0] = '\0'; desc[0] = '\0';
                extPort[0] = '\0'; intPort[0] = '\0'; intClient[0] = '\0';
                r = UPNP_GetGenericPortMappingEntry(urls.controlURL,
                                            data.first.servicetype,
                                            index,
                                            extPort, intClient, intPort,
                                            protocol, desc, enabled,
                                            rHost, duration);
                if(r==0)
                    qDebug() << "UPNP List :" << i << protocol << extPort << "->" << intClient << ":" << intPort << "'" << desc << "''" << rHost << "'" << duration << "";
                else
                    qDebug() << "UPNP List : GetGenericPortMappingEntry() returned" << r << "(" << strupnperror(r) << ")";
                i++;
            } while(r==0);
        }

        void DisplayInfos() {
            char externalIPAddress[40];
            char connectionType[64];
            char status[64];
            char lastconnerr[64];
            unsigned int uptime = 0;
            unsigned int brUp, brDown;
            time_t timenow, timestarted;
            int r;
            if(UPNP_GetConnectionTypeInfo(urls.controlURL,
                                        data.first.servicetype,
                                        connectionType) != UPNPCOMMAND_SUCCESS)
                qDebug() << "UPNP Info : GetConnectionTypeInfo failed.";
            else
                qDebug() << "UPNP Info : Connection Type :" << connectionType << "";
            if(UPNP_GetStatusInfo(urls.controlURL, data.first.servicetype,
                                status, &uptime, lastconnerr) != UPNPCOMMAND_SUCCESS)
                qDebug() << "UPNP Info : GetStatusInfo failed.";
            else
                qDebug() << "UPNP Info : Status :" << status << ", uptime=" << uptime << ", LastConnectionError :" << lastconnerr << "";
            if(uptime > 0) {
                timenow = time(NULL);
                timestarted = timenow - uptime;
                //char tt[26];
                //ctime_s(tt, sizeof(tt), &timestarted);
                auto tt = ctime(&timestarted);
                qDebug() << "UPNP Info :  Time started :" << tt;
            }
            if(UPNP_GetLinkLayerMaxBitRates(urls.controlURL_CIF, data.CIF.servicetype,
                                            &brDown, &brUp) != UPNPCOMMAND_SUCCESS) {
                qDebug() << "UPNP Info : GetLinkLayerMaxBitRates failed.";
            } else {
                qDebug() << "UPNP Info : MaxBitRateDown :" << brDown << " bps";
                if(brDown >= 1000000) {
                    qDebug() << "UPNP Info : (" << brDown / 1000000 << "." << (brDown / 100000) % 10 << "Mbps)";
                } else if(brDown >= 1000) {
                    qDebug() << "UPNP Info : (" << brDown / 1000 << " Kbps)";
                }
                qDebug() << "UPNP Info : MaxBitRateUp " << brUp << " bps";
                if(brUp >= 1000000) {
                    qDebug() << "UPNP Info : (" << brUp / 1000000 << "." << (brUp / 100000) % 10 << "Mbps)";
                } else if(brUp >= 1000) {
                    qDebug() << "UPNP Info : (" << brUp / 1000 << "Kbps)";
                }
                qDebug() << "";
            }
            r = UPNP_GetExternalIPAddress(urls.controlURL,
                                    data.first.servicetype,
                                    externalIPAddress);
            if(r != UPNPCOMMAND_SUCCESS) {
                qDebug() << "UPNP Info : GetExternalIPAddress failed. (errorcode=" << r << ")";
            } else {
                qDebug() << "UPNP Info : ExternalIPAddress=" << externalIPAddress << "";
            }
        }
};