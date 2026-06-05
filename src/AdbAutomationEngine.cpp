#include "AdbAutomationEngine.h"
#include <QMutexLocker>
#include <QProcess>
#include <QRandomGenerator>
AdbAutomationEngine::AdbAutomationEngine(QObject* parent):QThread(parent){}
void AdbAutomationEngine::enqueueSeed(const QString& seed){QMutexLocker lock(&mutex_); seeds_.enqueue(seed);}
void AdbAutomationEngine::requestStop(){QMutexLocker lock(&mutex_); stopRequested_=true;}
void AdbAutomationEngine::run(){emit automationLog(QStringLiteral("ADB automation loop started")); while(true){QString seed; {QMutexLocker lock(&mutex_); if(stopRequested_) break; if(!seeds_.isEmpty()) seed=seeds_.dequeue();} if(seed.isEmpty()){msleep(250); continue;} QProcess process; process.start(QStringLiteral("adb"), {QStringLiteral("shell"), QStringLiteral("input"), QStringLiteral("swipe"), QStringLiteral("500"), QStringLiteral("1500"), QStringLiteral("500"), QStringLiteral("600")}); process.waitForFinished(2500); emit automationLog(QStringLiteral("Processed seed: %1").arg(seed)); msleep(QRandomGenerator::global()->bounded(3500,7501));} emit automationLog(QStringLiteral("ADB automation loop stopped"));}
