#include "WinHttpClient.h"
#include <windows.h>
#include <winhttp.h>
#include <iostream>

#pragma comment(lib, "winhttp.lib")

std::wstring WinHttpClient::Utf8ToWstring(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::string WinHttpClient::WstringToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

bool WinHttpClient::ParseUrl(const std::string& url, std::wstring& host, int& port, std::wstring& path, bool& is_https) {
    std::wstring wurl = Utf8ToWstring(url);
    URL_COMPONENTS urlComp = { 0 };
    urlComp.dwStructSize = sizeof(urlComp);
    
    urlComp.dwHostNameLength  = (DWORD)-1;
    urlComp.dwUrlPathLength   = (DWORD)-1;
    urlComp.dwSchemeLength    = (DWORD)-1;
    urlComp.dwExtraInfoLength = (DWORD)-1;

    if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.length(), 0, &urlComp)) {
        return false;
    }

    host = std::wstring(urlComp.lpszHostName, urlComp.dwHostNameLength);
    path = std::wstring(urlComp.lpszUrlPath, urlComp.dwUrlPathLength);
    if (urlComp.dwExtraInfoLength > 0 && urlComp.lpszExtraInfo != nullptr) {
        path += std::wstring(urlComp.lpszExtraInfo, urlComp.dwExtraInfoLength);
    }
    
    port = urlComp.nPort;
    is_https = (urlComp.nScheme == INTERNET_SCHEME_HTTPS);
    return true;
}

HttpResponse WinHttpClient::Post(const std::string& url, const std::map<std::string, std::string>& headers, const std::string& body) {
    HttpResponse response;
    std::wstring host, path;
    int port;
    bool is_https;

    if (!ParseUrl(url, host, port, path, is_https)) {
        response.error_message = "Invalid URL format.";
        return response;
    }

    HINTERNET hSession = WinHttpOpen(L"PersonalAIAgent/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        response.error_message = "WinHttpOpen failed.";
        return response;
    }

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
    if (!hConnect) {
        response.error_message = "WinHttpConnect failed.";
        WinHttpCloseHandle(hSession);
        return response;
    }

    DWORD dwOpenRequestFlag = is_https ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, dwOpenRequestFlag);
    if (!hRequest) {
        response.error_message = "WinHttpOpenRequest failed.";
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return response;
    }

    std::wstring wHeaders;
    for (const auto& header : headers) {
        wHeaders += Utf8ToWstring(header.first + ": " + header.second + "\r\n");
    }

    BOOL bResults = WinHttpSendRequest(hRequest, wHeaders.c_str(), (DWORD)wHeaders.length(), (LPVOID)body.c_str(), (DWORD)body.length(), (DWORD)body.length(), 0);
    
    if (bResults) {
        bResults = WinHttpReceiveResponse(hRequest, NULL);
    }

    if (bResults) {
        DWORD dwStatusCode = 0;
        DWORD dwSize = sizeof(dwStatusCode);
        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BYPASS, &dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX);
        response.status_code = dwStatusCode;

        DWORD dwAvailableSize = 0;
        do {
            dwAvailableSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwAvailableSize)) break;
            if (dwAvailableSize == 0) break;

            std::vector<char> buffer(dwAvailableSize + 1, 0);
            DWORD dwDownloaded = 0;
            if (WinHttpReadData(hRequest, (LPVOID)buffer.data(), dwAvailableSize, &dwDownloaded)) {
                response.body.append(buffer.data(), dwDownloaded);
            }
        } while (dwAvailableSize > 0);
    } else {
        response.error_message = "Failed to send request or receive response. Error code: " + std::to_string(GetLastError());
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return response;
}

HttpResponse WinHttpClient::Get(const std::string& url, const std::map<std::string, std::string>& headers) {
    HttpResponse response;
    std::wstring host, path;
    int port;
    bool is_https;

    if (!ParseUrl(url, host, port, path, is_https)) {
        response.error_message = "Invalid URL format.";
        return response;
    }

    HINTERNET hSession = WinHttpOpen(L"PersonalAIAgent/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return response;

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), port, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return response; }

    DWORD dwOpenRequestFlag = is_https ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, dwOpenRequestFlag);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return response; }

    std::wstring wHeaders;
    for (const auto& header : headers) {
        wHeaders += Utf8ToWstring(header.first + ": " + header.second + "\r\n");
    }

    BOOL bResults = WinHttpSendRequest(hRequest, wHeaders.empty() ? WINHTTP_NO_ADDITIONAL_HEADERS : wHeaders.c_str(), (DWORD)wHeaders.length(), WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    if (bResults) bResults = WinHttpReceiveResponse(hRequest, NULL);

    if (bResults) {
        DWORD dwStatusCode = 0;
        DWORD dwSize = sizeof(dwStatusCode);
        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BYPASS, &dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX);
        response.status_code = dwStatusCode;

        DWORD dwAvailableSize = 0;
        do {
            dwAvailableSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwAvailableSize)) break;
            if (dwAvailableSize == 0) break;
            std::vector<char> buffer(dwAvailableSize + 1, 0);
            DWORD dwDownloaded = 0;
            if (WinHttpReadData(hRequest, (LPVOID)buffer.data(), dwAvailableSize, &dwDownloaded)) {
                response.body.append(buffer.data(), dwDownloaded);
            }
        } while (dwAvailableSize > 0);
    } else {
        response.error_message = "Error code: " + std::to_string(GetLastError());
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return response;
}
