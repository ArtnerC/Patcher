#include "stdafx.h"
#include "HTTPClient.h"

CHTTPClient::CHTTPClient(CString host) {
	_client = new http_client(_host.CharToOemA);
}

CHTTPClient::~CHTTPClient() {
	delete _client;
}

void CHTTPClient::RegisterStatus(/*Function Pointer*/) {
	//m_StatusFunc = arg
}

bool CHTTPClient::Download(CString file, CFile &dest) {
	_client->request(methods::GET, _path.CharToOemA + L"/" + file.CharToOemA).then([dest](http_response response) {
		if (response.status_code() != status_codes::OK) {
			return;
		}
		
		if (response.headers().content_length() <= 0) {
			return;
		}

		auto bodyStream = response.body();

		bodyStream.read()
		dest.Write()

	});
	return false;
}