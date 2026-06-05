#pragma once

#include <QString>

enum class UiLanguage { English, Chinese };

class UiText final {
 public:
  static QString text(const QString& key, UiLanguage language);
};
