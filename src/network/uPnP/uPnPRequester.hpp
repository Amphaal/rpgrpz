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

#include <QString>

#include "uPnPThread.hpp"

#include "miniupnpc_custom.hpp"

class uPnPRequester : public uPnPThread {
 public:
    uPnPRequester(const QString &tPort, const QString &descr) : _description(descr), _targetPort(tPort) {}

    void run() override {
        try {
            // init uPnP...
            auto initOK = this->_initUPnP();
            if (!initOK) {
                emit uPnPError();
                return;
            }

            // check if has redirection already done
            auto checkSuccess = _checkIfHasRedirect();
            if (!checkSuccess) {
                emit uPnPError();
                return;
            }

            // if has no redirection already
            if(!_hasRedirect) {
                // do ask redirection
                auto redirectResult = this->_requestRedirection();

                // handle error
                if (redirectResult != 0) {
                    emit uPnPError();
                    return;
                }
            }

            // success !
            emit uPnPSuccess(_PROTOCOL, this->_targetPort);

        // on exception
        } catch(...) {
            qDebug() << "UPNP run : exception caught while processing";
            emit uPnPError();
        }
    }

    ~uPnPRequester() {
            // remove any redirect if had any
            if (_hasRedirect)
                this->_removeRedirect();

            /*free*/
            if(_IGDFound) FreeUPNPUrls(&_urls);
            if(_devicesList) freeUPNPDevlist(_devicesList);

            /*End websock*/
            #ifdef _WIN32
                WSACleanup();
            #endif
    }

 private:
    static constexpr int _IPv6 = 0; /* defaults to NO */
    static constexpr unsigned char _TTL = 2; /* defaulting to 2 */
    static constexpr int _LOCALPORT = UPNP_LOCAL_PORT_ANY;
    static constexpr int _DISCOVER_DELAY_MS = 2000;
    static inline const QString _PROTOCOL = "TCP";
    static inline const QString _LEASE_DURATION = "0";  // infinite lease

    #ifdef _WIN32
        WSADATA _wsaData;
        WORD _requestedVersion = MAKEWORD(2, 2);
    #endif

    UPNPUrls _urls;
    IGDdatas _IGDData;
    bool _IGDFound = false;
    UPNPDev* _devicesList = nullptr;
    char _lanaddr[64] = "unset"; /* my ip address on the LAN */
    char _externalIPAddress[40] = "unset"; /* my ip address on the WAN */
    bool _hasRedirect = false;

    QString _description;
    QString _targetPort;

    // returns error code if any
    int _discoverDevices() {
        // not used
        char* _multicastif = nullptr;
        char* _minissdpdpath = nullptr;

        // discover
        int error;
        _devicesList = upnpDiscover(
            _DISCOVER_DELAY_MS,
            _multicastif,
            _minissdpdpath,
            _LOCALPORT,
            _IPv6,
            _TTL,
            &error
        );

        // if not devices found...
        if(!_devicesList) {
            qDebug() << "UPNP Inst : upnpDiscover() error code=" << error;
            return error;
        }

        // iterate through devices discovered
        UPNPDev* device;
        qDebug() << "UPNP Inst : List of UPNP devices found on the network :";
        for (device = _devicesList; device; device = device->pNext) {
            qDebug() << "UPNP Inst : desc:" << device->descURL << "st:" << device->st;
        }

        // succeeded !
        return 0;
    }

    // returns if succeeded
    bool _getExternalIP() {
        // request
        int r = UPNP_GetExternalIPAddress(
            _urls.controlURL,
            _IGDData.first.servicetype,
            _externalIPAddress
        );

        // if failed
        if (r != UPNPCOMMAND_SUCCESS) {
            qDebug() << "UPNP AskRedirect : GetExternalIPAddress No IGD UPnP Device. ";
            return false;
        }

        // succeeded !
        qDebug() << "UPNP AskRedirect : ExternalIPAddress =" << _externalIPAddress;
        emit uPnPExtIpFound(_externalIPAddress);
        return true;
    }

    // returns if succeeded
    bool _getValidIGD() {
        // request
        int result;
        result = UPNP_GetValidIGD(_devicesList, &_urls, &_IGDData, _lanaddr, sizeof(_lanaddr));

        // if no IGD found
        if(!result) return false;

        // more diag messages
        switch (result) {
            case 1:
                qDebug() << "UPNP Inst : Found valid IGD :" << _urls.controlURL;
                break;
            case 2:
                qDebug() << "UPNP Inst : Found a (not connected?) IGD :" << _urls.controlURL;
                qDebug() << "UPNP Inst : Trying to continue anyway";
                break;
            case 3:
                qDebug() << "UPNP Inst : UPnP device found. Is it an IGD ? :" << _urls.controlURL;
                qDebug() << "UPNP Inst : Trying to continue anyway";
                break;
            default:
                qDebug() << "UPNP Inst : Found device (igd ?) :" << _urls.controlURL;
                qDebug() << "UPNP Inst : Trying to continue anyway";
        }
        qDebug() << "UPNP Inst : Local LAN ip address" << _lanaddr;

        // succeeded !
        _IGDFound = true;
        return true;
    }

    // returns if succeeded
    bool _initUPnP() {
        /* start websock if Windows platform */
        #ifdef _WIN32
            auto nResult = WSAStartup(_requestedVersion, &_wsaData);
            if (nResult != NO_ERROR) {
                qDebug() << "UPNP Inst : Cannot init socket with WSAStartup !";
                return false;
            }
        #endif

        /* discover devices */
        if (_discoverDevices() != 0) {
            qDebug() << "UPNP Inst : No IGD UPnP Device found on the network !";
            return false;
        }

        /* get IGD */
        if(!_getValidIGD()) {
            qDebug() << "UPNP Inst : No valid UPNP Internet Gateway Device found.";
            return false;
        }

        /* get external IP */
        if(!_getExternalIP()) {
            return false;
        }

        // succeeded !
        return true;
    }

    // returns if request succeeded
    bool _checkIfHasRedirect() {
        // getter args
        char intClient[40];
        char intPort[6];
        char duration[16];

        // request
        auto result = UPNP_GetSpecificPortMappingEntry(
            _urls.controlURL,
            _IGDData.first.servicetype,
            qUtf8Printable(_targetPort),
            qUtf8Printable(_PROTOCOL),
            NULL /*remoteHost*/,
            intClient,
            intPort,
            NULL /*desc*/,
            NULL /*enabled*/,
            duration
       );

        // no redirect acked
        if(result == 714) {
            _hasRedirect = false;
            return true;
        }

        // if any code
        if (result != UPNPCOMMAND_SUCCESS) {
            qDebug() << "UPNP AskRedirect : GetSpecificPortMappingEntry() failed with code" << result <<"(" << strupnperror(result) << ")";
            return false;
        }

        // else, has redirect
        qDebug() << "UPNP AskRedirect : external" << _externalIPAddress << ":" << qUtf8Printable(_targetPort) << qUtf8Printable(_PROTOCOL)
                 << "is redirected to internal" << intClient << ":" << intPort << "(duration=" << duration << ")";
        _hasRedirect = true;
        return true;
    }

    // return error code if any
    int _requestRedirection() {
        auto result = UPNP_AddPortMapping(
                _urls.controlURL,
                _IGDData.first.servicetype,
                qUtf8Printable(_targetPort),
                qUtf8Printable(_targetPort),
                _lanaddr,
                qUtf8Printable(_description),
                qUtf8Printable(_PROTOCOL),
                NULL /*remoteHost*/,
                qUtf8Printable(_LEASE_DURATION)
            );

        // check if error
        if (result != UPNPCOMMAND_SUCCESS) {
            qDebug() << "UPNP AskRedirect : AddPortMapping(" << qUtf8Printable(_targetPort) << "," << qUtf8Printable(_targetPort) << "," << _lanaddr
                     << ") failed with code" << result << "(" << strupnperror(result) << ")";
            return result;
        }

        // success !
        _hasRedirect = true;
        return 0;
    }

    // returns if request succeeded
    bool _removeRedirect() {
        // request
        auto result = UPNP_DeletePortMapping(
            _urls.controlURL,
            _IGDData.first.servicetype,
            qUtf8Printable(_targetPort),
            qUtf8Printable(_PROTOCOL),
            NULL /*remoteHost*/
        );

        // check error
        if (result != UPNPCOMMAND_SUCCESS) {
            qDebug() << "UPNP RemoveRedirect : UPNP_DeletePortMapping() failed with code :" << result << "";
            return false;
        }

        // success
        qDebug() << "UPNP RemoveRedirect : UPNP_DeletePortMapping() succeeded !";
        _hasRedirect = false;
        return true;
    }
};
