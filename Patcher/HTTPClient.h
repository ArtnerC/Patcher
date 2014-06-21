#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

class CHTTPClient {
public:
	CHTTPClient(CString host);
	~CHTTPClient();
	void RegisterStatus();
	bool Download(CString file, CFile &dest);
private:
	http_client* _client;
	CString _host;
	CString _path;
};