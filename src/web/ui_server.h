//
// Created by ian on 12/24/21.
//

#ifndef __UI_SERVER_H__
#define __UI_SERVER_H__

#include "contrib/httplib.h"
#include "context.h"
#include "content/content_manager.h"
#include "upnp_xml.h"

class UIServer {
public:
    UIServer(std::shared_ptr<Context> context,
             std::shared_ptr<ContentManager> contentManager,
             std::shared_ptr<UpnpXMLBuilder> xmlbuilder,
             std::string iface,
             in_port_t port);

private:
    // HTTP Server for the UI
    httplib::Server server;

    std::shared_ptr<Context> context;
    std::shared_ptr<ContentManager> contentManager;
    std::shared_ptr<UpnpXMLBuilder> xmlbuilder;
};

#endif //__UI_SERVER_H__
