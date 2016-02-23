//
// Created by Саид on 14.02.16.
//

#include "Server.h"

INITIALIZE_EASYLOGGINGPP

namespace http {
    namespace server {
        Server::Server(int port)
                : port_(port) {
            el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Format,
                                               "%level %datetime{%H:%m:%s} (%func): %msg");
        }

        void Server::read_cb(struct bufferevent *bev, void *ctx) {
            size_t length = evbuffer_get_length(bufferevent_get_input(bev));
            char *data = new char[length];

            evbuffer_remove(bufferevent_get_input(bev), data, length);
            std::cout << "Input: " << data << length << std::endl;
            HttpRequestParser requestParser;
            HttpRequest request;
            HttpResponse response;

            requestParser.reset();
            //requestParser.parse(request, data, length);

            //std::cout << request.getMethod();
            /*std::ifstream fin("test.html", std::ios_base::in | std::ios_base::binary);

            if (!fin.is_open()) {
                LOG(ERROR) << "Файл не может быть открыт или создан" << std::endl;
            }
            char buff[512];

            while (fin.read(buff, sizeof(buff)).gcount() > 0) {
                response.contentAppend(buff, (unsigned long) fin.gcount());
            }

            response.setStatus(HttpResponse::OK);
            response.setHeader(Header("Content-Length", std::to_string(response.getContent().size())), 0);
            response.setHeader(Header("Content-Type", mime_types::extensionToType("html")), 1);
            evbuffer_add(bufferevent_get_output(bev), response.toString().c_str(), response.toString().length());
*/
            delete[] data;
        }

        void Server::event_cb(struct bufferevent *bev, short events, void *ctx) {
            if (events & BEV_EVENT_ERROR) {
                LOG(ERROR) << "Error: event";

                bufferevent_free(bev);
            }

            if (events & BEV_EVENT_EOF) {
                bufferevent_free(bev);
            }

            LOG(INFO) << "New event";
        }

        void Server::accept_error_cb(struct evconnlistener *listener, void *ctx) {
            struct event_base *base = evconnlistener_get_base(listener);

            int err = EVUTIL_SOCKET_ERROR();

            LOG(ERROR) << "Error: " << err << "=" << evutil_socket_error_to_string(err);
            event_base_loopexit(base, NULL);
        }

        void Server::accept_conn_cb(struct evconnlistener *listener,
                                    evutil_socket_t fd,
                                    struct sockaddr *address,
                                    int socklen,
                                    void *ctx) {
            LOG(INFO) << "New client";
            struct event_base *base = evconnlistener_get_base(listener);
            struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

            bufferevent_setcb(bev, Server::read_cb, NULL, Server::event_cb, NULL);
            bufferevent_enable(bev, EV_READ | EV_WRITE);
        }

        void Server::start() {
            struct event_base *eventBase = event_base_new();
            struct sockaddr_in sin;

            sin.sin_family = AF_INET;
            sin.sin_port = htons(port_);
            sin.sin_addr.s_addr = INADDR_ANY;

            struct evconnlistener *listener = evconnlistener_new_bind(eventBase,
                                                                      accept_conn_cb,
                                                                      NULL,
                                                                      LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
                                                                      -1,
                                                                      (struct sockaddr *) &sin, sizeof(sin));
            if (!listener) {
                LOG(ERROR) << "Couldn't create listener";
            }
            evconnlistener_set_error_cb(listener, Server::accept_error_cb);
            event_base_dispatch(eventBase);
        }
    }
}