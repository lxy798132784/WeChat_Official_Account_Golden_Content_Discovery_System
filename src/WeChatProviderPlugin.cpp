#include "WeChatProviderPlugin.h"
QString WeChatProviderPlugin::providerId() const{return QStringLiteral("wechat");}
QString WeChatProviderPlugin::displayName() const{return QStringLiteral("WeChat Official Account Provider");}
bool WeChatProviderPlugin::start(QString* errorMessage){if(!bridge_.isListening()&&!bridge_.startBridge(9000)){if(errorMessage)*errorMessage=bridge_.errorString(); return false;} if(!adbEngine_.isRunning()) adbEngine_.start(); return true;}
void WeChatProviderPlugin::stop(){adbEngine_.requestStop(); adbEngine_.wait(3000); bridge_.close();}
QVector<ContentRecord> WeChatProviderPlugin::drainRecords(){return bridge_.drainRecords();}
