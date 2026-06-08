#include "AdbTool.h"

#include <QFileInfo>
#include <QProcessEnvironment>

QString AdbTool::executable() {
  const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  const QStringList keys = {QStringLiteral("HERMES_ADB"), QStringLiteral("ADB_PATH")};
  for (const QString& key : keys) {
    const QString candidate = env.value(key).trimmed();
    if (!candidate.isEmpty() && QFileInfo(candidate).isExecutable()) {
      return candidate;
    }
  }
  return QStringLiteral("adb");
}
