//
// Project: clibparser
// Created by bajdcc
//

#include "stdafx.h"
#include "cnet.h"
#include "cvfs.h"
#include "ui/window/Window.h"
#include <curl/curl.h>
#include "base64/b64.h"
#include <base/utils.h>

#define LOG_NET 1

namespace clib {

    int cnet::req_id = 0;

    string_t cnet::http_get(const string_t& url, bool& post, string_t& postfield, bool& bin) {
        if (url.find("/http/") != string_t::npos) {
            auto u = url.substr(6);
            string_t flag;
            for (size_t i = 0; i < u.length() && u[i] != '/'; i++) {
                if (u[i] == '!') {
                    flag = u.substr(0, i);
                    u = u.substr(i + 1);
                    break;
                }
            }
            auto flags = std::split(flag, ',');
            std::unordered_set<string_t> flagset(flags.begin(), flags.end());
            auto prefix = flagset.find("https") != flagset.end() ? "https://" : "http://";
            post = flagset.find("post") != flagset.end();
            bin = flagset.find("bin") != flagset.end();
            if (post) {
                auto f = u.rfind('!');
                postfield = u.substr(f + 1);
                u = u.substr(0, f);
            }
            return prefix + u;
        }
        return "";
    }

    static size_t net_http_get_process(void* data, size_t size, size_t nmemb, std::vector<char> * bindata)
    {
        auto sizes = size * nmemb;
        char* d = (char*)data;
        for (size_t i = 0; i < sizes; ++i)
            bindata->push_back(*d++);
        return sizes;
    }

    void pass_event(evutil_socket_t fd, short event, void* arg)
    {
        auto response = (net_http_response*)arg;
        if (*response->received == 0)
        {
            auto net = response->net;
            net->set_response(response->data);
        }
        else if (*response->received == 1)
        {
            delete response->received;
        }
        delete response;
    }

    CString cnet::Utf8ToStringT(LPCSTR str)
    {
        _ASSERT(str);
        USES_CONVERSION;
        auto length = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
        CString s;
        auto buf = s.GetBuffer(length + 1);
        ZeroMemory(buf, (length + 1) * sizeof(WCHAR));
        MultiByteToWideChar(CP_UTF8, 0, str, -1, buf, length);
        s.ReleaseBuffer();
        return s;
    }

    CString cnet::GBKToStringT(LPCSTR str)
    {
        _ASSERT(str);
        USES_CONVERSION;
        auto length = MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);
        CString s;
        auto buf = s.GetBuffer(length + 1);
        ZeroMemory(buf, (length + 1) * sizeof(WCHAR));
        MultiByteToWideChar(CP_ACP, 0, str, -1, buf, length);
        s.ReleaseBuffer();
        return s;
    }

    CStringA cnet::StringTToUtf8(CString str)
    {
        USES_CONVERSION;
        auto length = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
        CStringA s;
        auto buf = s.GetBuffer(length + 1);
        ZeroMemory(buf, (length + 1) * sizeof(CHAR));
        WideCharToMultiByte(CP_UTF8, 0, str, -1, buf, length, nullptr, nullptr);
        return s;
    }

    int cnet::get_id()
    {
        return req_id++;
    }

    void net_http_thread(net_http_request * request)
    {
        auto url = request->url;
        auto response = new net_http_response();
        auto post = request->post;
        auto postfield = request->postfield;
        auto bin = request->bin;
        response->net = request->net;
        response->received = request->received;
        response->id = request->id;
        response->b64 = request->b64;
        response->post = request->post;
        response->bin = request->bin;
        auto curl = curl_easy_init();
        delete request;
        request = nullptr;
        if (curl) {
            auto params = std::split(postfield, '&');
            postfield.clear();
            for (auto& p : params)
            {
                auto p2 = std::split(p, '=');
                if (p2.size() == 2)
                {
                    postfield += p2[0];
                    postfield += '=';
                    auto utf8 = cnet::StringTToUtf8(CString(p2[1].c_str()));
                    auto pf = curl_easy_escape(curl, utf8.GetBuffer(0), utf8.GetLength());
                    postfield += pf;
                    curl_free(pf);
                    postfield += '&';
                }
            }
            if (!postfield.empty() && postfield.back() == '&')
            {
                postfield.pop_back();
            }
            params.clear();
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.102 Safari/537.36");
            if (post)
            {
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfield.c_str());
            }
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 60L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, TRUE);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, TRUE);
            std::vector<char> bindata;
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &bindata);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &net_http_get_process);
            auto res = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response->code);
            if (res == CURLE_OK)
            {
                if (bin) {
                    response->data = bindata;
                }
                else {
                    char* content_type;
                    curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type);
                    string_t text;
                    text.resize(bindata.size());
                    text.assign(bindata.begin(), bindata.end());
                    auto ct = CStringA(content_type);
                    if (ct.Find("UTF-8")) {
                        auto as = CStringA(cnet::Utf8ToStringT(text.c_str()));
                        auto ast = string_t(as.GetBuffer(0));
                        response->data = std::vector<char>(ast.begin(), ast.end());
                        /*auto success = true;
                        for (size_t i = 0; i < text.length() && i < response->data.size(); ++i) {
                            if (response->data[i] == 63 && text[i] < 0) {
                                success = false;
                                break;
                            }
                        }
                        if (!success) {
                            response->data = std::vector<char>(text.begin(), text.end());
                        }*/
                    }
                    else
                        response->data = std::vector<char>(text.begin(), text.end());
                }
                auto ev = window->get_event();
                struct timeval tv;
                auto evt = evtimer_new(ev, &pass_event, response);
                evutil_timerclear(&tv);
                tv.tv_sec = 0;
                tv.tv_usec = 100;
                evtimer_add(evt, &tv);
            }
            curl_easy_cleanup(curl);
        }
    }

    void start_net_thread(evutil_socket_t fd, short event, void* arg)
    {
        std::thread th(net_http_thread, (net_http_request*)arg);
        th.detach();
    }

    int net_http_get_internal(vfs_node_stream_net * net, bool b64 = false, bool post = false, string_t postfield = "", bool bin = false)
    {
        auto request = new net_http_request();
        request->net = net;
        request->received = net->get_received();
        request->b64 = b64;
        request->url = net->get_url();
        request->post = post;
        request->bin = bin;
        if (post)
            request->postfield = postfield;
        auto ev = window->get_event();
        struct timeval tv;
        auto evt = evtimer_new(ev, &start_net_thread, request);
        evutil_timerclear(&tv);
        tv.tv_sec = 0;
        tv.tv_usec = 100;
        evtimer_add(evt, &tv);
        return 0;
    }

    int net_http_get(vfs_node_stream_net* net, bool post, const string_t& postfield, bool bin)
    {
        return net_http_get_internal(net, false, post, postfield, bin);
    }

    vfs_node_stream_net::vfs_node_stream_net(const vfs_mod_query * mod, vfs_stream_t s, vfs_stream_call * call, const string_t & path) :
        vfs_node_dec(mod), stream(s), call(call) {
        url = call->stream_net(stream, path, post, postfield, bin);
        if (url == "")
        {
            received = new int(2);
        }
        else
        {
            received = new int(0);
            id = net_http_get(this, post, postfield, bin);
        }
    }

    vfs_node_stream_net::~vfs_node_stream_net()
    {
        if (*received == 2)
        {
            delete received;
        }
        else
        {
            *received = 1;
        }
    }

    bool vfs_node_stream_net::get_data(std::vector<byte>& data) const
    {
        if (*received != 2) return false;
        std::transform(
            content.cbegin(),
            content.cend(),
            std::back_inserter(data),
            [](const char c) {
                return (byte)c;
            });
        return true;
    }

    int vfs_node_stream_net::get_length() const
    {
        if (*received != 2) return -1;
        return (int)content.size();
    }

    vfs_node_dec* vfs_node_stream_net::create(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call, const string_t& path)
    {
        return new vfs_node_stream_net(mod, s, call, path);
    }

    string_t vfs_node_stream_net::get_url() const
    {
        return url;
    }

    int* vfs_node_stream_net::get_received() const
    {
        return received;
    }

    void vfs_node_stream_net::set_response(const std::vector<char> & resp)
    {
        content = resp;
        *received = 2;
    }

    bool vfs_node_stream_net::available() const {
        return *received == 2 && idx < content.size();
    }

    int vfs_node_stream_net::index() const {
        if (*received == 0) return WAIT_CHAR;
        return idx < content.size() ? content[idx] : READ_EOF;
    }

    void vfs_node_stream_net::advance() {
        if (available())
            idx++;
    }

    int vfs_node_stream_net::write(byte c) {
        return -1;
    }

    int vfs_node_stream_net::truncate() {
        return -1;
    }

    int vfs_node_stream_server::net_server_thread(
        void* cls,
        MHD_Connection* connection,
        const char* url,
        const char* method,
        const char* version,
        const char* upload_data,
        size_t* upload_data_size,
        void** ptr)
    {
        auto server = (vfs_node_stream_server*)cls;
        MHD_Response* response;
        int ret;

        if (0 != strcmp(method, "GET"))
            return MHD_NO; /* unexpected method */
        if (0 != *upload_data_size)
            return MHD_NO; /* upload data in a GET!? */
        *ptr = NULL; /* clear context pointer */
        pthread_mutex_lock(&server->enter);
        server->ref++;
        server->gen_req(url);
        if (server->state == 5) {
            pthread_mutex_unlock(&server->enter);
            return MHD_NO;
        }
        auto size = server->resp.size();
        auto data = (char*)malloc(size);
        CopyMemory(data, server->resp.data(), size);
        server->reset();
        response = MHD_create_response_from_buffer(size,
            (void*)data,
            MHD_RESPMEM_MUST_FREE);
        server->ref--;
        pthread_mutex_unlock(&server->enter);
        ret = MHD_queue_response(connection,
            MHD_HTTP_OK,
            response);
        MHD_destroy_response(response);
        return ret;
    }

    vfs_node_stream_server::vfs_node_stream_server(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call, const string_t& path, int port) :
        vfs_node_dec(mod), stream(s), call(call), port(port) {
        daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
            port,
            NULL,
            NULL,
            &net_server_thread,
            this,
            MHD_OPTION_END);
        if (!daemon) {
            state = 3;
            return;
        }
        sem_init(&sem, 0, 0);
        pthread_mutex_init(&enter, nullptr);
        state = 1;
    }

    vfs_node_stream_server::~vfs_node_stream_server()
    {
        auto ref2 = ref;
        state = 5;
        for (auto i = 0; i < ref2; i++) {
            sem_post(&sem);
        }
        if (daemon)
            MHD_stop_daemon(daemon);
        if (sem)
            sem_destroy(&sem);
        if (enter)
            pthread_mutex_destroy(&enter);
    }

    vfs_node_dec* vfs_node_stream_server::create(const vfs_mod_query* mod, vfs_stream_t s, vfs_stream_call* call, const string_t& path)
    {
        static string_t pat{ R"(/server/([0-9]{1,5}))" };
        static std::regex re(pat);
        std::smatch res;
        if (std::regex_match(path, res, re)) {
            auto op = res[1].str();
            int port = atoi(op.c_str());
            if (port > 0 && port < 65536) {
                return new vfs_node_stream_server(mod, s, call, path, port);
            }
        }
        return nullptr;
    }

    void vfs_node_stream_server::gen_req(const char* url)
    {
        std::stringstream ss;
        ss << "{";
        ss << "url:\"" << url << "\"";
        ss << "}";
        req_string = ss.str();
        state = 2;
        sem_wait(&sem);
    }

    void vfs_node_stream_server::reset()
    {
        error = 0;
        state = 1;
        req_string.clear();
        resp.clear();
        idx = 0;
    }

    bool vfs_node_stream_server::available() const {
        return state == 2 && idx < req_string.size();
    }

    int vfs_node_stream_server::index() const {
        if (state == 3)
            return READ_ERROR;
        if (state == 2) {
            if (idx < req_string.size())
                return req_string[idx];
            return READ_EOF;
        }
        return WAIT_CHAR;
    }

    void vfs_node_stream_server::advance() {
        if (available())
            idx++;
    }

    int vfs_node_stream_server::write(byte c) {
        if (state != 2)
            return -1;
        resp.push_back(c);
        return 0;
    }

    int vfs_node_stream_server::truncate() {
        if (state != 2) {
            error++;
            if (error >= 5) {
                state = 3;
                return -2;
            }
            return -1;
        }
        state = 4;
        sem_post(&sem);
        return 0;
    }

    bool vfs_node_stream_server::set_data(const std::vector<byte>& data)
    {
        if (state != 2)
            return false;
        resp.resize(data.size());
        std::copy(data.begin(), data.end(), resp.begin());
        state = 4;
        sem_post(&sem);
        return true;
    }
}