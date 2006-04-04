//
// C++ Implementation: rapiprovisioningclient
//
// Description:
//
//
// Author: Richard van den Toorn <vdtoorn@users.sourceforge.net>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "rapiprovisioningclient.h"
#include "rapiproxyconnection.h"
#include <iostream>

RapiProvisioningClient::RapiProvisioningClient( int fd, TCPServerSocket *tcpServerSocket )
    : RapiClient( fd, tcpServerSocket ), initialized(false)
{
    setBlocking();
    setReadTimeout( 5, 0 );
}


RapiProvisioningClient::~RapiProvisioningClient()
{
}


void RapiProvisioningClient::event( void )
{
    if ( !initialized ) {
        // 4Byte static conteng 0x01, 0x00, 0x00, 0x00
        unsigned char buffer[ 4 ];
        if ( readNumBytes( buffer, 4 ) != 4 ) {
            rapiProxyConnection->provisioningClientNotInitialized();
            return ;
        }
        printPackage( "RapiProvisioningClient", ( unsigned char * ) buffer, 4 );
        rapiProxyConnection->provisioningClientInitialized();

        initialized = true;
    } else {
        rapiProxyConnection->messageToApplication();
    }
}


void RapiProvisioningClient::setRapiProxyConnection( RapiProxyConnection *rapiProxyConnection)
{
    this->rapiProxyConnection = rapiProxyConnection;
}
