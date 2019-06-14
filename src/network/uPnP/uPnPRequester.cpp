#include "uPnPThread.hpp"

#include <QString>

class uPnPRequester : public uPnPThread {
    
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

        uPnPRequester(const char * tPort, const char * descr, QObject * parent = nullptr) :
            targetPort(tPort), 
            description(descr) {
            this->setParent(parent);
        }
        
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
                qWarning() << "UPNP run : exception caught while processing";
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
                WSACleanup();        
        }


    private:

        void _initUPnP() {

            /*start websock*/
            WSADATA wsaData;
            int nResult = WSAStartup(MAKEWORD(2,2), &wsaData);
            if(nResult != NO_ERROR) {
                 qWarning() << "UPNP Inst : Cannot init socket with WSAStartup !\n";
            };
            
            /*discover*/
            if( rootdescurl || (devlist = upnpDiscover(2000, multicastif, minissdpdpath, localport, ipv6, ttl, &error))) {
                
                struct UPNPDev * device;
                
                if(devlist) {

                    qDebug() << "UPNP Inst : List of UPNP devices found on the network :\n";
                    
                    for(device = devlist; device; device = device->pNext) {
                        qDebug() << "UPNP Inst :  desc: " << device->descURL << "\n st:" << device->st << "\n\n";
                    }

                } else if(!rootdescurl) {
                    qWarning() << "UPNP Inst : upnpDiscover() error code=" << error << "\n";
                }

                i = 1;

                if( (rootdescurl && UPNP_GetIGDFromUrl(rootdescurl, &urls, &data, lanaddr, sizeof(lanaddr)))
                || (i = UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr)))) {
                    
                    switch(i) {
                        case 1:
                            qDebug() << "UPNP Inst : Found valid IGD : " << urls.controlURL << "\n";
                            break;
                        case 2:
                            qDebug() << "UPNP Inst : Found a (not connected?) IGD : " << urls.controlURL << "\n";
                            qDebug() << "UPNP Inst : Trying to continue anyway\n";
                            break;
                        case 3:
                            qDebug() << "UPNP Inst : UPnP device found. Is it an IGD ? : " << urls.controlURL << "\n";
                            qDebug() << "UPNP Inst : Trying to continue anyway\n";
                            break;
                        default:
                            qDebug() << "UPNP Inst : Found device (igd ?) : " << urls.controlURL << "\n";
                            qDebug() << "UPNP Inst : Trying to continue anyway\n";
                    }

                    qDebug() << "UPNP Inst : Local LAN ip address " << lanaddr << "\n";

                    char externalIPAddress[40];
                    int r = UPNP_GetExternalIPAddress(urls.controlURL,
                        data.first.servicetype,
                        externalIPAddress);
                    if(r != UPNPCOMMAND_SUCCESS)
                        qWarning() << "UPNP AskRedirect : GetExternalIPAddress No IGD UPnP Device.\n";
                    else {
                        qDebug() << "UPNP AskRedirect : ExternalIPAddress = " << externalIPAddress <<"\n";
                        emit uPnPExtIpFound(externalIPAddress);
                    }

                }
                else {
                    qWarning() << "UPNP Inst : No valid UPNP Internet Gateway Device found.\n";
                    retcode = 1;
                }
            }
            else {
                qWarning() << "UPNP Inst : No IGD UPnP Device found on the network !\n";
                retcode = 1;
            }
        }

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
                qWarning() << "UPNP AskRedirect : Wrong arguments\n";
                return -1;
            }
            proto = protofix(proto);
            if(!proto)
            {
                qWarning() << "UPNP AskRedirect : invalid protocol\n";
                return -1;
            }

            r = UPNP_GetExternalIPAddress(urls.controlURL,
                            data.first.servicetype,
                            externalIPAddress);
            if(r!=UPNPCOMMAND_SUCCESS)
                qWarning() << "UPNP AskRedirect : GetExternalIPAddress No IGD UPnP Device.\n";
            else {
                qDebug() << "UPNP AskRedirect : ExternalIPAddress = " << externalIPAddress <<"\n";
            }

            if (addAny) {
                r = UPNP_AddAnyPortMapping(urls.controlURL, data.first.servicetype,
                            eport, iport, iaddr, description,
                            proto, 0, leaseDuration, reservedPort);
                if(r==UPNPCOMMAND_SUCCESS)
                    eport = reservedPort;
                else
                    qWarning() << "UPNP AskRedirect : AddAnyPortMapping(" << eport  <<", "<< iport <<", "<< iaddr <<") failed with code "<< r <<" ("<< strupnperror(r) <<")\n";
            } else {
                r = UPNP_AddPortMapping(urls.controlURL, data.first.servicetype,
                            eport, iport, iaddr, description,
                            proto, NULL/*remoteHost*/, leaseDuration);
                if(r!=UPNPCOMMAND_SUCCESS) {
                    qWarning() << "UPNP AskRedirect : AddPortMapping(" << eport  <<", "<< iport <<", "<< iaddr <<") failed with code "<< r <<" ("<< strupnperror(r) <<")\n";
                    return -2;
                }
            }

            r = UPNP_GetSpecificPortMappingEntry(urls.controlURL,
                                data.first.servicetype,
                                eport, proto, NULL/*remoteHost*/,
                                intClient, intPort, NULL/*desc*/,
                                NULL/*enabled*/, duration);
            if(r!=UPNPCOMMAND_SUCCESS) {
                qWarning() << "UPNP AskRedirect : GetSpecificPortMappingEntry() failed with code "<< r <<" (" << strupnperror(r) << ")\n";
                return -2;
            } else {
                qDebug() << "UPNP AskRedirect : InternalIP:Port = " << intClient << ":" << intPort << "\n";
                qDebug() << "UPNP AskRedirect : external " << externalIPAddress << ":" << eport <<" " << proto << " is redirected to internal " << intClient << ":" << intPort << " (duration=" << duration << ")\n";
            }
            return 0;
        }

        int RemoveRedirect(const char * eport,
                    const char * proto,
                    const char * remoteHost) {
            int r;
            if(!proto || !eport)
            {
                qWarning() << "UPNP RemoveRedirect : invalid arguments\n";
                return -1;
            }
            proto = protofix(proto);
            if(!proto)
            {
                qWarning() << "UPNP RemoveRedirect : protocol invalid\n";
                return -1;
            }
            r = UPNP_DeletePortMapping(urls.controlURL, data.first.servicetype, eport, proto, remoteHost);
            if(r!=UPNPCOMMAND_SUCCESS) {
                qWarning() << "UPNP RemoveRedirect : UPNP_DeletePortMapping() failed with code : " << r << "\n";
                return -2;
            }else {
                qDebug() << "UPNP RemoveRedirect : UPNP_DeletePortMapping() returned : " << r <<"\n";
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

            qDebug() << "UPNP List : i protocol exPort->inAddr:inPort description remoteHost leaseTime\n";

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
                    qDebug() << "UPNP List : " << i << " " << protocol << " " << extPort << "->" << intClient << ":" << intPort << " '" << desc << "' '" << rHost << "' " << duration << "\n";
                else
                    qDebug() << "UPNP List : GetGenericPortMappingEntry() returned " << r << " (" << strupnperror(r) << ")\n";
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
                qWarning() << "UPNP Info : GetConnectionTypeInfo failed.\n";
            else
                qDebug() << "UPNP Info : Connection Type : " << connectionType << "\n";
            if(UPNP_GetStatusInfo(urls.controlURL, data.first.servicetype,
                                status, &uptime, lastconnerr) != UPNPCOMMAND_SUCCESS)
                qWarning() << "UPNP Info : GetStatusInfo failed.\n";
            else
                qDebug() << "UPNP Info : Status : " << status << ", uptime=" << uptime << ", LastConnectionError : " << lastconnerr << "\n";
            if(uptime > 0) {
                timenow = time(NULL);
                timestarted = timenow - uptime;
                qDebug() << "UPNP Info :  Time started : " << ctime(&timestarted);
            }
            if(UPNP_GetLinkLayerMaxBitRates(urls.controlURL_CIF, data.CIF.servicetype,
                                            &brDown, &brUp) != UPNPCOMMAND_SUCCESS) {
                qWarning() << "UPNP Info : GetLinkLayerMaxBitRates failed.\n";
            } else {
                qDebug() << "UPNP Info : MaxBitRateDown : " << brDown << " bps";
                if(brDown >= 1000000) {
                    qDebug() << "UPNP Info : (" << brDown / 1000000 << "." << (brDown / 100000) % 10 << " Mbps)";
                } else if(brDown >= 1000) {
                    qDebug() << "UPNP Info : (" << brDown / 1000 << " Kbps)";
                }
                qDebug() << "UPNP Info :   MaxBitRateUp " << brUp << " bps";
                if(brUp >= 1000000) {
                    qDebug() << "UPNP Info : (" << brUp / 1000000 << "." << (brUp / 100000) % 10 << " Mbps)";
                } else if(brUp >= 1000) {
                    qDebug() << "UPNP Info : (" << brUp / 1000 << " Kbps)";
                }
                qDebug() << "\n";
            }
            r = UPNP_GetExternalIPAddress(urls.controlURL,
                                    data.first.servicetype,
                                    externalIPAddress);
            if(r != UPNPCOMMAND_SUCCESS) {
                qWarning() << "UPNP Info : GetExternalIPAddress failed. (errorcode=" << r << ")\n";
            } else {
                qDebug() << "UPNP Info : ExternalIPAddress = " << externalIPAddress << "\n";
            }
        }
};