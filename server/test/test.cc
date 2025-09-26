// wfrest接收jsonPOST请求


#include "wfrest/HttpServer.h"
#include "wfrest/Json.h"
#include <workflow/Workflow.h>
#include <iostream>
#include <functional>

using namespace std;
using namespace wfrest;
using f = function<void(int)>;

int main()
{
	Json json = Json::parse(R"({"name": true})");

	json["name"] = false;

	if(json["name"]) cout << 1;
	else cout << 0;
	

	getchar();
}