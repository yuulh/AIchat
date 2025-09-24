#include "MySqlBp.h"

void MySqlBp::setBP()
{
    // 执行一段SQL
    bp.POST("/execute", [this](const HttpReq *req, HttpResp *resp){  // 不暴露对外
        // TODO： 判断客户端ip是否为本机
        
        if(req->content_type() != TEXT_PLAIN){
            resp->set_status(HttpStatusBadRequest);
            return;
        }
        string sql =  req->body();

        resp->String(client.execute(resp, sql));
    });

    // 获取角色信息
    /* bp.GET("/assistant", [this](const HttpReq *req, HttpResp *resp){

    }); */
}