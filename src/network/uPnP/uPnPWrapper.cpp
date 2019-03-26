#include "uPnPThread.h"

class uPnPWrapper : public uPnPThread {
    
    public:
        struct UPNPUrls urls;
        struct IGDdatas data;
        struct UPNPDev * devlist = 0;
        char lanaddr[64] = "unset";	/* my ip address on the LAN */
        int i;
        const char * rootdescurl = 0;
        const char * multicastif = 0;
        const char * minissdpdpath = 0;
        int localport = UPNP_LOCAL_PORT_ANY;
        int retcode = 0;
        int error = 0;
        int ipv6 = 0;
        unsigned char ttl = 2;	/* defaulting to 2 */
        const char * description = 0;
        const char * targetPort = 0;

        uPnPWrapper(char * targetPort, char * description) : targetPort(targetPort), description(description)  { }
        
        void run() override {

            /*start websock*/
            WSADATA wsaData;
            int nResult = WSAStartup(MAKEWORD(2,2), &wsaData);
            if(nResult != NO_ERROR) return;
            
            /*discover*/
            if( rootdescurl || (devlist = upnpDiscover(2000, multicastif, minissdpdpath, localport, ipv6, ttl, &error))) {
                
                struct UPNPDev * device;
                
                if(devlist) {

                    qDebug() << "List of UPNP devices found on the network :\n";
                    
                    for(device = devlist; device; device = device->pNext) {
                        qDebug() << " desc: " << device->descURL << "\n st:" << device->st << "\n\n";
                    }

                } else if(!rootdescurl) {
                    qDebug() << "upnpDiscover() error code=" << error << "\n";
                }

                i = 1;

                if( (rootdescurl && UPNP_GetIGDFromUrl(rootdescurl, &urls, &data, lanaddr, sizeof(lanaddr)))
                || (i = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr)))) {
                    
                    switch(i) {
                        case 1:
                            qDebug() << "Found valid IGD : " << urls.controlURL << "\n";
                            break;
                        case 2:
                            qDebug() << "Found a (not connected?) IGD : " << urls.controlURL << "\n";
                            qDebug() << "Trying to continue anyway\n";
                            break;
                        case 3:
                            qDebug() << "UPnP device found. Is it an IGD ? : " << urls.controlURL << "\n";
                            qDebug() << "Trying to continue anyway\n";
                            break;
                        default:
                            qDebug() << "Found device (igd ?) : " << urls.controlURL << "\n";
                            qDebug() << "Trying to continue anyway\n";
                    }

                    qDebug() << "Local LAN ip address : " << lanaddr << "\n";

                }
                else {
                    qDebug() << "No valid UPNP Internet Gateway Device found.\n";
                    retcode = 1;
                }
            }
            else {
                qDebug() << "No IGD UPnP Device found on the network !\n";
                retcode = 1;
            }

            auto result = this->SetRedirectAndTest(lanaddr, this->targetPort, this->targetPort, "TCP", "0", 0);

            emit initialized(result, this->targetPort);
        }

        ~uPnPWrapper() {
            
            this->RemoveRedirect(this->targetPort, "TCP", NULL);

            /*free*/
            FreeUPNPUrls(&urls);
            freeUPNPDevlist(devlist); devlist = 0;
            
            /*End websock*/
            WSACleanup();
        }


    private:

        int SetRedirectAndTest(const char * iaddr,
                        const char * iport,
                        const char * eport,
                        const char * proto,
                        const char * leaseDuration,
                        int addAny) {

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

            r = UPNP_GetExternalIPAddress(urls.controlURL,
                            data.first.servicetype,
                            externalIPAddress);
            if(r!=UPNPCOMMAND_SUCCESS)
                printf("GetExternalIPAddress failed.\n");
            else
                printf("ExternalIPAddress = %s\n", externalIPAddress);

            if (addAny) {
                r = UPNP_AddAnyPortMapping(urls.controlURL, data.first.servicetype,
                            eport, iport, iaddr, description,
                            proto, 0, leaseDuration, reservedPort);
                if(r==UPNPCOMMAND_SUCCESS)
                    eport = reservedPort;
                else
                    printf("AddAnyPortMapping(%s, %s, %s) failed with code %d (%s)\n",
                        eport, iport, iaddr, r, strupnperror(r));
            } else {
                r = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype,
                            eport, iport, iaddr, description,
                            proto, NULL/*remoteHost*/, leaseDuration);
                if(r!=UPNPCOMMAND_SUCCESS) {
                    printf("AddPortMapping(%s, %s, %s) failed with code %d (%s)\n",
                        eport, iport, iaddr, r, strupnperror(r));
                    return -2;
            }
            }

            r = UPNP_GetSpecificPortMappingEntry(urls.controlURL,
                                data.first.servicetype,
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

        int RemoveRedirect(const char * eport,
                    const char * proto,
                    const char * remoteHost) {
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
            r = UPNP_DeletePortMapping(urls.controlURL, data.first.servicetype, eport, proto, remoteHost);
            if(r!=UPNPCOMMAND_SUCCESS) {
                printf("UPNP_DeletePortMapping() failed with code : %d\n", r);
                return -2;
            }else {
                printf("UPNP_DeletePortMapping() returned : %d\n", r);
            }
            return 0;
        }

        void ListRedirections() {
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

            qDebug() << " i protocol exPort->inAddr:inPort description remoteHost leaseTime\n";

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
                    qDebug() << i << " " << protocol << " " << extPort << "->" << intClient << ":" << intPort << " '" << desc << "' '" << rHost << "' " << duration << "\n";
                else
                    qDebug() << "GetGenericPortMappingEntry() returned " << r << " (" << strupnperror(r) << ")\n";
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
                qDebug() << "GetConnectionTypeInfo failed.\n";
            else
                qDebug() << "Connection Type : " << connectionType << "\n";
            if(UPNP_GetStatusInfo(urls.controlURL, data.first.servicetype,
                                status, &uptime, lastconnerr) != UPNPCOMMAND_SUCCESS)
                qDebug() << "GetStatusInfo failed.\n";
            else
                qDebug() << "Status : " << status << ", uptime=" << uptime << ", LastConnectionError : " << lastconnerr << "\n";
            if(uptime > 0) {
                timenow = time(NULL);
                timestarted = timenow - uptime;
                qDebug() << "  Time started : " << ctime(&timestarted);
            }
            if(UPNP_GetLinkLayerMaxBitRates(urls.controlURL_CIF, data.CIF.servicetype,
                                            &brDown, &brUp) != UPNPCOMMAND_SUCCESS) {
                qDebug() << "GetLinkLayerMaxBitRates failed.\n";
            } else {
                qDebug() << "MaxBitRateDown : " << brDown << " bps";
                if(brDown >= 1000000) {
                    qDebug() << " (" << brDown / 1000000 << "." << (brDown / 100000) % 10 << " Mbps)";
                } else if(brDown >= 1000) {
                    qDebug() << " (" << brDown / 1000 << " Kbps)";
                }
                qDebug() << "   MaxBitRateUp " << brUp << " bps";
                if(brUp >= 1000000) {
                    qDebug() << " (" << brUp / 1000000 << "." << (brUp / 100000) % 10 << " Mbps)";
                } else if(brUp >= 1000) {
                    qDebug() << " (" << brUp / 1000 << " Kbps)";
                }
                qDebug() << "\n";
            }
            r = UPNP_GetExternalIPAddress(urls.controlURL,
                                    data.first.servicetype,
                                    externalIPAddress);
            if(r != UPNPCOMMAND_SUCCESS) {
                qDebug() << "GetExternalIPAddress failed. (errorcode=" << r << ")\n";
            } else {
                qDebug() << "ExternalIPAddress = " << externalIPAddress << "\n";
            }
        }
};