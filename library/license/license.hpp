#include <string>
#include <WinInet.h>
#include <VMProtectSDK.h>
#include <xorstr/include/xorstr.hpp>
#pragma comment(lib, "wininet.lib")

std::string HttpsRequest(std::string site, std::string path, std::string data) {
#ifndef VMP
	VMProtectBeginMutation("HttpsRequest");
#endif
	HINTERNET hInternet = InternetOpenA(xorstr("prism").crypt_get(), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, NULL);
	HINTERNET hConnect = InternetConnectA(hInternet, site.c_str(), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);

	const char* AcceptTypes[] = { xorstr("application/json").crypt_get(), NULL };
	HINTERNET hRequest = HttpOpenRequestA(hConnect, xorstr("POST").crypt_get(), path.c_str(), HTTP_VERSIONA, NULL, AcceptTypes,
		INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE,
		NULL);

	DWORD dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
		SECURITY_FLAG_IGNORE_REVOCATION |
		SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTP |
		SECURITY_FLAG_IGNORE_REDIRECT_TO_HTTPS |
		SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
		SECURITY_FLAG_IGNORE_CERT_CN_INVALID;
	InternetSetOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));

	bool bRequestSent = HttpSendRequestA(hRequest, NULL, 0, (void*)data.c_str(), data.length());

	DWORD statusCode = 0;
	DWORD length = sizeof(DWORD);
	HttpQueryInfo(
		hRequest,
		HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
		&statusCode,
		&length,
		NULL
	);

	if (statusCode == 400)
		return HttpsRequest(site, path, data);

	std::string res;
	char buff[1024];
	DWORD dwRead;
	while (InternetReadFile(hRequest, buff, 1, &dwRead)) {
		if (not dwRead)
			break;
		res.append(buff, dwRead);
	}

	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);

	return res;
#ifndef VMP
	VMProtectEnd();
#endif
}