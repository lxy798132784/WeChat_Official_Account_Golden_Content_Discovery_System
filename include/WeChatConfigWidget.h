#pragma once

#include <QWidget>

class QCheckBox;
class QSpinBox;
class QTextEdit;

/**
 * @brief 微信插件配置组件
 * WeChat plugin configuration widget
 *
 * @details 暴露本地桥端口、ADB 自动化开关和运行日志区域。
 * Exposes local bridge port, ADB automation switch, and runtime log area.
 */
class WeChatConfigWidget final : public QWidget {
  Q_OBJECT
 public:
  explicit WeChatConfigWidget(QWidget* parent = nullptr);
  quint16 bridgePort() const;
  bool adbAutomationEnabled() const;
  void appendLog(const QString& message);

 private:
  QSpinBox* portSpinBox_ = nullptr;
  QCheckBox* adbCheckBox_ = nullptr;
  QTextEdit* logView_ = nullptr;
};
