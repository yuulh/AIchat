#include "Log.h"
#include "Configuration.h"
#include "bluePrint/BluePrint.h"
#include <log4cpp/PropertyConfigurator.hh>
#include <cstring>
#include <wfrest/HttpServer.h>
#include <wfrest/BluePrint.h>

using namespace wfrest;

// string configPath = "/usr/AIchat/server/config/config.conf";

int main()
{
    log4cpp::PropertyConfigurator::configure(CONFIG["logConfigPath"]);
    
    // 启动SNI，解决SSL错误问题
    WFGlobalSettings settings = GLOBAL_SETTINGS_DEFAULT;
	settings.endpoint_params.use_tls_sni = true;
	WORKFLOW_library_init(&settings);

    HttpServer svr;

    ChatBp chatBp(CONFIG["API_KEY"]);
    chatBp.setBP();
    svr.register_blueprint(chatBp.getBP(), "/api");

    UserBp userBp;
    userBp.setBP();
    svr.register_blueprint(userBp.getBP(), "/user");


    if (svr.start(atoi(CONFIG["PORT"].c_str())) == 0){
        fprintf(stderr, "服务启动在: %s\n", CONFIG["PORT"].c_str());
        LOG_INFO("Server started");
        getchar();
        svr.stop();
    } else {
        fprintf(stderr, "Cannot start server");
        exit(1);
    }

    return 0;
}

