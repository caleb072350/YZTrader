#pragma once 

#include <string>
#include <unordered_map>

#include "../Includes/ILogHandler.h"

NS_WTP_BEGIN
class WTSVariant;
class WtDataStorage;
NS_WTP_END

USING_NS_WTP;

class WtRunner : public ILogHandler
{
public:
    WtRunner();
    ~WtRunner();

public:
    void init(const std::string& filename);

    bool config(const std::string& filename);

    void run(bool bAsync = false);

private:
    bool initTraders(WTSVariant* cfgTrader);
    bool initParsers(WTSVariant* cfgParser);
	bool initExecuters(WTSVariant* cfgExecuter);
	bool initDataMgr();
	bool initEvtNotifier();
	bool initCtaStrategies();
	bool initHftStrategies();
	bool initActionPolicy();

    bool initEngine();

//////////////////////////////////////////////////////////////////////////
//ILogHandler
public:
	virtual void handleLogAppend(WTSLogLevel ll, const char* msg) override;

private:
	WTSVariant*			_config;
	TraderAdapterMgr	_traders;
	ParserAdapterMgr	_parsers;
	WtExecuterFactory	_exe_factory;
};