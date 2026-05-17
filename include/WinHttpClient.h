#ifndef WIN_HTTP_CLIENT_H
#define WIN_HTTP_CLIENT_H

#include <string>
#include <vector>
#include <map>

struct HttpResponse {
    int status_code = 0;
    std::string body;
    std::string error_message;
};

class WinHttpClient {
public:
    static HttpResponse Post(const std::string& url, const std::map<std::string, std::string>& headers, const std::string& body);
    static HttpResponse Get(const std::string& url, const std::map<std::string, std::string>& headers);

private:
    static bool ParseUrl(const std::string& url, std::wstring& host, int& port, std::wstring& path, bool& is_https);
    static std::wstring Utf8ToWstring(const std::string& str);
    static std::string WstringToUtf8(const std::wstring& wstr);
};

#endif // WIN_HTTP_CLIENT_H
