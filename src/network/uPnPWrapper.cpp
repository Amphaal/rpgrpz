#include <miniupnpc.h>
#include <upnperrors.h>
#include <winsock.h>

class uPnPWrapper {
    public:
        uPnPWrapper() {
                WSADATA wsaData; 
                int nResult = WSAStartup(MAKEWORD(2,2), &wsaData); 
                if(nResult != NO_ERROR) return;

                int error = 0;
                //get a list of upnp devices (asks on the broadcast address and returns the responses)
                struct UPNPDev *upnp_dev = upnpDiscover(20,    //timeout in milliseconds
                                                        NULL,  //multicast address, default = "239.255.255.250"
                                                        NULL,  //minissdpd socket, default = "/var/run/minissdpd.sock"
                                                        0,     //source port, default = 1900
                                                        0,
                                                        0,
                                                        &error); //error output
                auto c = strupnperror(error);
                freeUPNPDevlist(upnp_dev);
        }
}