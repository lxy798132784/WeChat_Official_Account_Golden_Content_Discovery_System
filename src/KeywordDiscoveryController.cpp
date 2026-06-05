#include "KeywordDiscoveryController.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QSet>
#include <QTextDocumentFragment>
#include <QUrl>
#include <QUrlQuery>

namespace {
QString valueString(const QJsonObject& object, const QStringList& keys) {
  for (const QString& key : keys) {
    const QJsonValue value = object.value(key);
    if (value.isString() && !value.toString().trimmed().isEmpty()) {
      return value.toString().trimmed();
    }
  }
  return QString();
}

int valueInt(const QJsonObject& object, const QStringList& keys) {
  for (const QString& key : keys) {
    const QJsonValue value = object.value(key);
    if (value.isDouble()) {
      return value.toInt();
    }
    if (value.isString()) {
      QString text = value.toString();
      text.remove(QRegularExpression(QStringLiteral("[^0-9]")));
      if (!text.isEmpty()) {
        return text.toInt();
      }
    }
  }
  return 0;
}

QJsonArray rootArray(const QJsonDocument& document) {
  if (document.isArray()) {
    return document.array();
  }
  if (!document.isObject()) {
    return {};
  }
  const QJsonObject root = document.object();
  for (const QString& key : {QStringLiteral("articles"), QStringLiteral("items"), QStringLiteral("results"), QStringLiteral("data")}) {
    if (root.value(key).isArray()) {
      return root.value(key).toArray();
    }
  }
  return {};
}

QString decodeHtml(QString text) {
  text.remove(QRegularExpression(QStringLiteral("<[^>]+>")));
  return QTextDocumentFragment::fromHtml(text).toPlainText().simplified();
}

QString normalizeSearchUrl(QString urlText) {
  urlText.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
  QUrl url(urlText);
  if (!url.isValid()) {
    return QString();
  }
  if (url.host().contains(QStringLiteral("sogou.com"))) {
    QUrlQuery query(url);
    const QString target = query.queryItemValue(QStringLiteral("url"));
    if (!target.isEmpty()) {
      url = QUrl::fromPercentEncoding(target.toUtf8());
    }
  }
  if (!url.isValid() || url.scheme().isEmpty()) {
    return QString();
  }
  return url.toString(QUrl::FullyEncoded);
}
}  // namespace

KeywordDiscoveryController::KeywordDiscoveryController(QObject* parent) : QObject(parent) {}

bool KeywordDiscoveryController::isSearching() const {
  return searching_;
}

QVector<KeywordDiscoveryResult> KeywordDiscoveryController::results() const {
  return results_;
}

int KeywordDiscoveryController::maxResultsPerKeyword() const {
  return maxResultsPerKeyword_;
}

void KeywordDiscoveryController::setMaxResultsPerKeyword(int count) {
  maxResultsPerKeyword_ = qBound(1, count, 50);
}

QStringList KeywordDiscoveryController::parseKeywords(const QString& text) {
  QStringList output;
  const QStringList parts = text.split(QRegularExpression(QStringLiteral("[\\r\\n,，;；]+")), Qt::SkipEmptyParts);
  for (const QString& part : parts) {
    const QString keyword = part.trimmed();
    if (!keyword.isEmpty() && !output.contains(keyword)) {
      output.push_back(keyword);
    }
  }
  return output;
}

bool KeywordDiscoveryController::matchesHotCriteria(const KeywordDiscoveryResult& result, int minimumReadCount,
                                                    int minimumLikeCount, int minimumCommentCount,
                                                    int minimumHotScore) {
  return result.readNum >= minimumReadCount && result.likeNum >= minimumLikeCount &&
         result.commentNum >= minimumCommentCount && result.hotScore >= minimumHotScore;
}

QString KeywordDiscoveryController::searchUrlForKeyword(const QString& keyword) {
  QUrl url(QStringLiteral("https://weixin.sogou.com/weixin"));
  QUrlQuery query;
  query.addQueryItem(QStringLiteral("type"), QStringLiteral("2"));
  query.addQueryItem(QStringLiteral("query"), keyword.trimmed());
  url.setQuery(query);
  return url.toString();
}

double KeywordDiscoveryController::estimateHotScore(int readNum, int likeNum, int commentNum) {
  return static_cast<double>(readNum) + static_cast<double>(likeNum) * 20.0 + static_cast<double>(commentNum) * 50.0;
}

QVector<KeywordDiscoveryResult> KeywordDiscoveryController::parseResultsJson(const QByteArray& data, QString* errorMessage) {
  QJsonParseError parseError;
  const QJsonDocument document = QJsonDocument::fromJson(data, &parseError);
  if (parseError.error != QJsonParseError::NoError) {
    if (errorMessage != nullptr) {
      *errorMessage = parseError.errorString();
    }
    return {};
  }
  const QJsonArray array = rootArray(document);
  if (array.isEmpty()) {
    if (errorMessage != nullptr) {
      *errorMessage = QStringLiteral("JSON must contain an array or an articles/items/results/data array");
    }
    return {};
  }
  QVector<KeywordDiscoveryResult> results;
  for (const QJsonValue& value : array) {
    if (!value.isObject()) {
      continue;
    }
    const QJsonObject object = value.toObject();
    KeywordDiscoveryResult result;
    result.keyword = valueString(object, {QStringLiteral("keyword"), QStringLiteral("query")});
    result.title = valueString(object, {QStringLiteral("title"), QStringLiteral("article_title"), QStringLiteral("name")});
    result.accountName = valueString(object, {QStringLiteral("account_name"), QStringLiteral("account"), QStringLiteral("publisher"), QStringLiteral("source")});
    result.category = valueString(object, {QStringLiteral("category"), QStringLiteral("industry"), QStringLiteral("tag")});
    result.url = valueString(object, {QStringLiteral("url"), QStringLiteral("article_url"), QStringLiteral("link")});
    result.readNum = valueInt(object, {QStringLiteral("read_num"), QStringLiteral("read"), QStringLiteral("reads"), QStringLiteral("read_count")});
    result.likeNum = valueInt(object, {QStringLiteral("like_num"), QStringLiteral("like"), QStringLiteral("likes"), QStringLiteral("like_count")});
    result.commentNum = valueInt(object, {QStringLiteral("comment_num"), QStringLiteral("comments"), QStringLiteral("comment_count")});
    result.hotScore = object.value(QStringLiteral("hot_score")).isDouble()
                          ? object.value(QStringLiteral("hot_score")).toDouble()
                          : estimateHotScore(result.readNum, result.likeNum, result.commentNum);
    if (!result.url.isEmpty()) {
      results.push_back(result);
    }
  }
  std::sort(results.begin(), results.end(), [](const KeywordDiscoveryResult& left, const KeywordDiscoveryResult& right) {
    return left.hotScore > right.hotScore;
  });
  return results;
}

QVector<KeywordDiscoveryResult> KeywordDiscoveryController::parseSearchHtml(const QString& keyword, const QByteArray& html) {
  const QString page = QString::fromUtf8(html);
  QVector<KeywordDiscoveryResult> results;
  QSet<QString> seen;

  QRegularExpression itemRe(QStringLiteral(R"(<li[^>]*id=["']sogou_vr_11002601_box_[^>]*>(.*?)</li>)"),
                            QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);
  QRegularExpression linkRe(QStringLiteral(R"(<a[^>]+href=["']([^"']+)["'][^>]*>(.*?)</a>)"),
                            QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);
  QRegularExpression accountRe(QStringLiteral(R"(<a[^>]+class=["'][^"']*account[^"']*["'][^>]*>(.*?)</a>)"),
                               QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);
  QRegularExpressionMatchIterator iterator = itemRe.globalMatch(page);
  while (iterator.hasNext()) {
    const QString block = iterator.next().captured(1);
    const QRegularExpressionMatch linkMatch = linkRe.match(block);
    if (!linkMatch.hasMatch()) {
      continue;
    }
    QString url = normalizeSearchUrl(linkMatch.captured(1));
    if (url.isEmpty() || !url.contains(QStringLiteral("mp.weixin.qq.com")) || seen.contains(url)) {
      continue;
    }
    seen.insert(url);
    KeywordDiscoveryResult result;
    result.keyword = keyword;
    result.title = decodeHtml(linkMatch.captured(2));
    const QRegularExpressionMatch accountMatch = accountRe.match(block);
    result.accountName = accountMatch.hasMatch() ? decodeHtml(accountMatch.captured(1)) : QString();
    result.url = url;
    result.hotScore = estimateHotScore(result.readNum, result.likeNum, result.commentNum);
    results.push_back(result);
  }

  if (results.isEmpty()) {
    QRegularExpression mpLinkRe(QStringLiteral(R"(href=["']([^"']*mp\.weixin\.qq\.com[^"']*)["'][^>]*>(.*?)</a>)"),
                                QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator linkIterator = mpLinkRe.globalMatch(page);
    while (linkIterator.hasNext()) {
      const QRegularExpressionMatch match = linkIterator.next();
      QString url = normalizeSearchUrl(match.captured(1));
      if (url.isEmpty() || seen.contains(url)) {
        continue;
      }
      seen.insert(url);
      KeywordDiscoveryResult result;
      result.keyword = keyword;
      result.title = decodeHtml(match.captured(2));
      result.url = url;
      result.hotScore = estimateHotScore(result.readNum, result.likeNum, result.commentNum);
      results.push_back(result);
    }
  }
  return results;
}

QString KeywordDiscoveryController::resultsToQueueText(const QVector<KeywordDiscoveryResult>& results, int minimumReadCount) {
  return resultsToQueueText(results, minimumReadCount, 0, 0, 0);
}

QString KeywordDiscoveryController::resultsToQueueText(const QVector<KeywordDiscoveryResult>& results, int minimumReadCount,
                                                       int minimumLikeCount, int minimumCommentCount,
                                                       int minimumHotScore) {
  QStringList urls;
  for (const KeywordDiscoveryResult& result : results) {
    if (matchesHotCriteria(result, minimumReadCount, minimumLikeCount, minimumCommentCount, minimumHotScore) &&
        !result.url.isEmpty() && !urls.contains(result.url)) {
      urls.push_back(result.url);
    }
  }
  return urls.join(QStringLiteral("\n"));
}

void KeywordDiscoveryController::searchKeywords(const QString& text) {
  if (searching_) {
    emit logMessage(QStringLiteral("Keyword search is already running"));
    return;
  }
  pendingKeywords_ = parseKeywords(text);
  results_.clear();
  if (pendingKeywords_.isEmpty()) {
    emit logMessage(QStringLiteral("No keyword provided"));
    emit searchFinished(results_);
    return;
  }
  searching_ = true;
  emit searchStarted();
  emit logMessage(QStringLiteral("Keyword search started: %1 keyword(s)").arg(pendingKeywords_.size()));
  startNextKeyword();
}

void KeywordDiscoveryController::cancelSearch() {
  if (!searching_) {
    return;
  }
  pendingKeywords_.clear();
  finishSearch();
}

void KeywordDiscoveryController::startNextKeyword() {
  if (!searching_) {
    return;
  }
  if (pendingKeywords_.isEmpty()) {
    finishSearch();
    return;
  }
  const QString keyword = pendingKeywords_.takeFirst();
  QNetworkRequest request(QUrl(searchUrlForKeyword(keyword)));
  request.setHeader(QNetworkRequest::UserAgentHeader,
                    QStringLiteral("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36"));
  QNetworkReply* reply = network_.get(request);
  connect(reply, &QNetworkReply::finished, this, [this, reply, keyword]() { handleReply(reply, keyword); });
}

void KeywordDiscoveryController::handleReply(QNetworkReply* reply, const QString& keyword) {
  reply->deleteLater();
  if (reply->error() != QNetworkReply::NoError) {
    emit logMessage(QStringLiteral("Keyword search failed for [%1]: %2").arg(keyword, reply->errorString()));
    startNextKeyword();
    return;
  }
  QVector<KeywordDiscoveryResult> parsed = parseSearchHtml(keyword, reply->readAll());
  if (parsed.size() > maxResultsPerKeyword_) {
    parsed.resize(maxResultsPerKeyword_);
  }
  QSet<QString> seen;
  for (const KeywordDiscoveryResult& existing : results_) {
    seen.insert(existing.url);
  }
  int added = 0;
  for (const KeywordDiscoveryResult& result : parsed) {
    if (!result.url.isEmpty() && !seen.contains(result.url)) {
      results_.push_back(result);
      seen.insert(result.url);
      ++added;
    }
  }
  emit logMessage(QStringLiteral("Keyword [%1] discovered %2 candidate article(s)").arg(keyword).arg(added));
  startNextKeyword();
}

void KeywordDiscoveryController::finishSearch() {
  searching_ = false;
  std::sort(results_.begin(), results_.end(), [](const KeywordDiscoveryResult& left, const KeywordDiscoveryResult& right) {
    return left.hotScore > right.hotScore;
  });
  emit logMessage(QStringLiteral("Keyword search finished: %1 candidate article(s)").arg(results_.size()));
  emit searchFinished(results_);
}
