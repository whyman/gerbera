//
// Created by ian on 12/24/21.
//

#include <netinet/in.h>
#include <string>
#include "ui_server.h"
#include "web_request_handler.h"
#include "pages.h"



UIServer::UIServer(std::shared_ptr<Context> context,
    std::shared_ptr<ContentManager> contentManager,
    std::shared_ptr<UpnpXMLBuilder> xmlbuilder,
    std::string iface, in_port_t port) :
context(context),
    contentManager(contentManager),
    xmlbuilder(xmlbuilder)
{
    server.set_exception_handler([](const auto& req, auto& res, std::exception &e) {
        res.status = 500;
        auto fmt = "<h1>Error 500</h1><p>%s</p>";
        char buf[BUFSIZ];
        snprintf(buf, sizeof(buf), fmt, e.what());
        res.set_content(buf, "text/html");
        log_error("UI: {}", e.what());
    });

    server.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("Hello World!", "text/plain");
    });

    server.Get("/content/interface", [this](const httplib::Request& req, httplib::Response& res) {
        auto params = req.params;

        auto it = params.find(URL_REQUEST_TYPE);
        std::string page = it != params.end() && !it->second.empty() ? it->second : "index";

        auto handler = Web::createWebRequestHandler(this->context, this->contentManager, this->xmlbuilder, page);

        auto target = req.target;
        log_info("target: {}", target);

        auto info = UpnpFileInfo_new();
        handler->getInfo(req.target.c_str(), info);
        auto ioHandler = handler->open(req.target.c_str(), UPNP_READ);

        char* buf = new char[1024];
        ioHandler->read(buf, 1024 -1);

        res.set_chunked_content_provider(UpnpFileInfo_get_ContentType(info), )
//        handler->open();
//        handler->read();
//        handler->close();

        UpnpFileInfo_delete(info);
        res.set_content("Hello World!", "text/plain");
    });

    server.listen("0.0.0.0", 55556);
}
