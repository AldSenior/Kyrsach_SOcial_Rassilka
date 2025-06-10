#include "scheduler.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSettings>
#include "posttarget.h"
#include "logger.h"

Scheduler::Scheduler(Database *db, SocialMedia *sm, QObject *parent) : QObject(parent), db(db), socialMedia(sm) {
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Scheduler::processScheduledPosts);
}

void Scheduler::start() {
    timer->start(1000);
}

void Scheduler::processScheduledPosts() {
    Logger::info("Processing scheduled posts...");
    QVector<ScheduledPost> duePosts = db->loadAndDeleteDuePosts();
    if (duePosts.isEmpty()) {
        Logger::info("Found 0 due posts");
        return;
    }

    QSettings settings;
    QJsonDocument targetsDoc = QJsonDocument::fromJson(settings.value("post_targets").toByteArray());
    QMap<QString, PostTarget> targetMap;
    for (const auto& val : targetsDoc.array()) {
        QJsonObject obj = val.toObject();
        PostTarget target;
        target.id = obj["id"].toString();
        target.platform = obj["platform"].toString();
        target.token = obj["token"].toString();
        target.target_id = obj["target_id"].toString();
        targetMap[target.id] = target;
    }

    for (const auto& post : duePosts) {
        QJsonDocument doc = QJsonDocument::fromJson(post.targets.toUtf8());
        for (const auto& targetId : doc.array()) {
            QString id = targetId.toString();
            if (targetMap.contains(id)) {
                PostTarget target = targetMap[id];
                QString result;
                if (target.platform == "Telegram") {
                    result = socialMedia->postToTelegram(target.token, target.target_id, post.content);
                } else if (target.platform == "VK") {
                    result = socialMedia->postToVK(target.token, target.target_id, post.content);
                }
                db->saveNotification(
                    target.platform,
                    result.contains("Успешно") ? "Отправлено" : "Ошибка",
                    result
                    );
                db->savePost(target.platform, post.content);
            }
        }
    }
}
