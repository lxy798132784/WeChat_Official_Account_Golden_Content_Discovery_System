#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include "content_record.h"

/**
 * @brief 内容供应商接口 / Content provider interface
 *
 * @details 所有采集插件必须实现该接口，主程序通过 Qt 插件系统解耦调用。
 * Every ingestion plugin implements this contract and is loaded by the host shell.
 */
class IContentProvider {
 public:
  virtual ~IContentProvider() = default;
  virtual QString providerId() const = 0;
  virtual QString displayName() const = 0;
  virtual bool start(QString* errorMessage) = 0;
  virtual void stop() = 0;
  virtual QVector<ContentRecord> drainRecords() = 0;
};

#define IContentProvider_iid "com.premiumcontentradar.IContentProvider/1.0"
Q_DECLARE_INTERFACE(IContentProvider, IContentProvider_iid)
