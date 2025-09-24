#ifndef MYSQLBP_H
#define MYSQLBP_H

#include "../client/MySqlClient.h"
#include "BpBase.h"

class MySqlBp : public BpBase {
    MySqlClient client;
public:
    void setBP() override;
};

#endif