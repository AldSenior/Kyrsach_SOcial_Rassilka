#ifndef SOCIALMEDIA_H
#define SOCIALMEDIA_H

#include <QString>

class SocialMedia {
public:
    SocialMedia();
    QString postToTelegram(const QString &token, const QString &chatId, const QString &message);
    QString postToVK(const QString &token, const QString &groupId, const QString &message);
};
#endif
