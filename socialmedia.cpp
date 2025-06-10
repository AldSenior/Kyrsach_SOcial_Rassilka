#include "socialmedia.h"
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QUrlQuery>

SocialMedia::SocialMedia() {}

QString SocialMedia::postToTelegram(const QString &token, const QString &chatId,
                                    const QString &message) {
    QNetworkAccessManager manager;
    QNetworkRequest request;
    QUrl url(QString("https://api.telegram.org/bot%1/sendMessage").arg(token));
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/x-www-form-urlencoded");

    QUrlQuery query;
    query.addQueryItem("chat_id", chatId);
    query.addQueryItem("text", message);
    query.addQueryItem("parse_mode", "Markdown");
    QByteArray data = query.toString(QUrl::FullyEncoded).toUtf8();

    QNetworkReply *reply = manager.post(request, data);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QString result;
    if (reply->error() == QNetworkReply::NoError) {
        result = "Успешно отправлено в Telegram!";
    } else {
        result = "Ошибка Telegram: " + reply->errorString();
    }

    reply->deleteLater();
    return result;
}

QString SocialMedia::postToVK(const QString &token, const QString &groupId,
                              const QString &message) {
    QNetworkAccessManager manager;
    QNetworkRequest request;
    QUrl url("https://api.vk.com/method/wall.post");
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/x-www-form-urlencoded");

    QUrlQuery query;
    query.addQueryItem("access_token", token);
    query.addQueryItem("owner_id", "-" + groupId);
    query.addQueryItem("message", message);
    query.addQueryItem("v", "5.131");
    QByteArray data = query.toString(QUrl::FullyEncoded).toUtf8();

    QNetworkReply *reply = manager.post(request, data);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QString result;
    if (reply->error() == QNetworkReply::NoError) {
        result = "Успешно отправлено в ВКонтакте!";
    } else {
        result = "Ошибка ВКонтакте: " + reply->errorString();
    }

    reply->deleteLater();
    return result;
}
