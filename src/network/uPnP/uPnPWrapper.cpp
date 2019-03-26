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
        
        uPnPWrapper() {}
        
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

            emit initialized(retcode);
        }

        ~uPnPWrapper() {
            
            /*free*/
            FreeUPNPUrls(&urls);
            freeUPNPDevlist(devlist); devlist = 0;
            
            /*End websock*/
            WSACleanup();
        }
};