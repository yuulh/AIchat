#include "Log.h"
#include "Configuration.h"
#include <log4cpp/PropertyConfigurator.hh>
#include <cstring>
#include <wfrest/HttpServer.h>
#include <wfrest/BluePrint.h>

using namespace wfrest;

string configPath = "/usr/AIchat/server/config/config.conf";

int main()
{
    log4cpp::PropertyConfigurator::configure(CONFIG["logConfigPath"]);
    
    HttpServer svr;

    

    return 0;
}

