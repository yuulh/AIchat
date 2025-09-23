#ifndef BPBASE_H
#define BPBASE_H

#include <wfrest/BluePrint.h>
#include <string>
#include <functional>

class BpBase {
protected:
    wfrest::BluePrint bp;
public:
    virtual void setBP();  // 注册蓝图路由规则
    /* void SET(const std::string &path,
             std::function<void(const wfrest::HttpReq *req, wfrest::HttpResp *resp)> callback)
    {
        bp.GET(path, callback);
    } */
};


#endif