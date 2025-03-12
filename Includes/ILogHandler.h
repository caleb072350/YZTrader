#pragma once 
#include "YzMarcos.h"
#include "YzSType.h"

NS_WTP_BEGIN

class ILogHandler
{
public:
    virtual void handleLogAppend(WTSLogLevel ll, const char * msg) = 0;
};

NS_WTP_END