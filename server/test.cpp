#include "workflow/MySQLResult.h"
#include "wfrest/HttpServer.h"

using namespace wfrest;
using namespace protocol;

int main(int argc, char **argv)
{
    HttpServer svr;
 
    svr.GET("/mysql00", [](const HttpReq *req, HttpResp *resp)
    {
        resp->MySQL("mysql://yuu:123456@localhost", "SHOW DATABASES");

    });

    svr.GET("/mysql01", [](const HttpReq *req, HttpResp *resp)
    {
        std::string url = "mysql://yuu:123456@localhost";
        resp->MySQL(url, "SHOW DATABASES", [resp](Json *json) 
        {
            Json js;
            js["rows"] = (*json)["result_set"][0]["rows"];
            resp->String(js.dump());
        });
    });

    svr.GET("/mysql02", [](const HttpReq *req, HttpResp *resp)
    {
        std::string url = "mysql://yuu:123456@localhost";

        resp->MySQL(url, "SHOW DATABASES", [resp](MySQLResultCursor *cursor) 
        {
            std::string res;
            std::vector<MySQLCell> arr;
            while (cursor->fetch_row(arr))
            {
                res.append(arr[0].as_string());
                res.append("\n");
            }
            resp->String(std::move(res));
        });

    });

    svr.GET("/multi", [](const HttpReq *req, HttpResp *resp)
    {
        resp->MySQL("mysql://yuu:123456@localhost/AIchat", "SHOW DATABASES; SELECT * FROM assistant");
    });

    svr.POST("/client", [](const HttpReq *req, HttpResp *resp)
    {
        std::string &sql = req->body();
        resp->MySQL("mysql://yuu:123456@localhost/AIchat", std::move(sql));
    });

    if (svr.start(5484) == 0)
    {
        getchar();
        svr.stop();
    } else
    {
        fprintf(stderr, "Cannot start server\n");
        exit(1);
    }
    return 0;
}