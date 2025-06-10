#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QUuid>
#include <QDir>
#include <QFileInfo>
#include "logger.h"

Database::Database(const std::string &dbPath) {
    // Ensure the directory exists
    QFileInfo dbFileInfo(QString::fromStdString(dbPath));
    QDir dir;
    if (!dir.exists(dbFileInfo.absolutePath())) {
        if (dir.mkpath(dbFileInfo.absolutePath())) {
            Logger::info("Created database directory: " + dbFileInfo.absolutePath());
        } else {
            Logger::error("Failed to create database directory: " + dbFileInfo.absolutePath());
        }
    }

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QString::fromStdString(dbPath));
    if (!db.open()) {
        Logger::error("Failed to open database: " + db.lastError().text());
    } else {
        Logger::info("Database opened successfully: " + QString::fromStdString(dbPath));
    }
}

Database::~Database() {
    if (db.isOpen()) {
        db.close();
    }
}

void Database::init() {
    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS posts (id INTEGER PRIMARY KEY, platform TEXT NOT NULL, content TEXT NOT NULL, timestamp TEXT NOT NULL)");
    query.exec("CREATE TABLE IF NOT EXISTS templates (id TEXT PRIMARY KEY, name TEXT NOT NULL, content TEXT NOT NULL)");
    query.exec("CREATE TABLE IF NOT EXISTS scheduled_posts (id TEXT PRIMARY KEY, content TEXT NOT NULL, schedule_time TEXT NOT NULL, targets TEXT NOT NULL)");
    query.exec("CREATE TABLE IF NOT EXISTS notifications (id TEXT PRIMARY KEY, platform TEXT NOT NULL, status TEXT NOT NULL, message TEXT NOT NULL, timestamp TEXT NOT NULL)");

    if (query.exec("PRAGMA table_info(scheduled_posts)")) {
        QStringList columns;
        while (query.next()) {
            columns << query.value(1).toString();
        }
        Logger::info("Scheduled_posts columns: " + columns.join(", "));
    } else {
        Logger::error("Failed to check scheduled_posts table: " + query.lastError().text());
    }

    Logger::info("Database initialized");
}

void Database::savePost(const QString &platform, const QString &content) {
    QSqlQuery query;
    query.prepare("INSERT INTO posts (platform, content, timestamp) VALUES (?, ?, ?)");
    query.addBindValue(platform);
    query.addBindValue(content);
    query.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    if (!query.exec()) {
        Logger::error("Failed to save post: " + query.lastError().text());
    } else {
        Logger::info("Post saved for " + platform + ": " + content.left(50) + "...");
    }
}

QVector<Post> Database::loadPosts() {
    QVector<Post> posts;
    QSqlQuery query("SELECT platform, content, timestamp FROM posts ORDER BY timestamp DESC");
    while (query.next()) {
        Post post;
        post.platform = query.value(0).toString();
        post.content = query.value(1).toString();
        post.time = query.value(2).toString();
        posts.append(post);
    }
    Logger::info(QString("Loaded %1 posts").arg(posts.size()));
    return posts;
}

void Database::saveTemplate(const QString &name, const QString &content) {
    QSqlQuery query;
    query.prepare("INSERT INTO templates (id, name, content) VALUES (?, ?, ?)");
    query.addBindValue(QUuid::createUuid().toString());
    query.addBindValue(name);
    query.addBindValue(content);
    if (!query.exec()) {
        Logger::error("Failed to save template: " + query.lastError().text());
    } else {
        Logger::info("Template saved: " + name);
    }
}

QVector<Template> Database::loadTemplates() {
    QVector<Template> templates;
    QSqlQuery query("SELECT id, name, content FROM templates");
    while (query.next()) {
        Template t;
        t.id = query.value(0).toString();
        t.name = query.value(1).toString();
        t.content = query.value(2).toString();
        templates.append(t);
    }
    Logger::info(QString("Loaded %1 templates").arg(templates.size()));
    return templates;
}

QString Database::loadTemplateContent(const QString &name) {
    QSqlQuery query;
    query.prepare("SELECT content FROM templates WHERE name = ?");
    query.addBindValue(name);
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    Logger::error("Failed to load template content for: " + name);
    return "";
}

void Database::schedulePost(const QString &content, const QDateTime &scheduleTime, const QString &targets) {
    QSqlQuery query;
    query.prepare("INSERT INTO scheduled_posts (id, content, schedule_time, targets) VALUES (?, ?, ?, ?)");
    QString id = QUuid::createUuid().toString();
    query.addBindValue(id);
    query.addBindValue(content);
    query.addBindValue(scheduleTime.toString("yyyy-MM-dd HH:mm:ss"));
    query.addBindValue(targets);
    if (!query.exec()) {
        Logger::error("Failed to schedule post: " + query.lastError().text() + ". Query: " + query.lastQuery());
    } else {
        Logger::info("Post scheduled successfully with ID: " + id);
    }
}

QVector<ScheduledPost> Database::loadScheduledPosts() {
    QVector<ScheduledPost> posts;
    QSqlQuery query("SELECT id, content, schedule_time, targets FROM scheduled_posts ORDER BY schedule_time");
    while (query.next()) {
        ScheduledPost post;
        post.id = query.value(0).toString();
        post.content = query.value(1).toString();
        post.schedule_time = query.value(2).toString();
        post.targets = query.value(3).toString();
        posts.append(post);
    }
    Logger::info(QString("Loaded %1 scheduled posts").arg(posts.size()));
    return posts;
}

QVector<ScheduledPost> Database::loadAndDeleteDuePosts() {
    QVector<ScheduledPost> posts;
    QSqlQuery selectQuery;
    selectQuery.prepare("SELECT id, content, schedule_time, targets FROM scheduled_posts WHERE schedule_time <= ?");
    selectQuery.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    if (selectQuery.exec()) {
        while (selectQuery.next()) {
            ScheduledPost post;
            post.id = selectQuery.value(0).toString();
            post.content = selectQuery.value(1).toString();
            post.schedule_time = selectQuery.value(2).toString();
            post.targets = selectQuery.value(3).toString();
            posts.append(post);
        }
        for (const auto &post : posts) {
            QSqlQuery deleteQuery;
            deleteQuery.prepare("DELETE FROM scheduled_posts WHERE id = ?");
            deleteQuery.addBindValue(post.id);
            if (!deleteQuery.exec()) {
                Logger::error("Failed to delete scheduled post: " + deleteQuery.lastError().text());
            }
        }
        Logger::info(QString("Loaded and deleted %1 due posts").arg(posts.size()));
    } else {
        Logger::error("Failed to load due posts: " + selectQuery.lastError().text() + ". Query: " + selectQuery.lastQuery());
    }
    return posts;
}

void Database::deleteScheduledPost(const QString &id) {
    QSqlQuery query;
    query.prepare("DELETE FROM scheduled_posts WHERE id = ?");
    query.addBindValue(id);
    if (!query.exec()) {
        Logger::error("Failed to delete scheduled post: " + id);
    } else {
        Logger::info("Deleted scheduled post: " + id);
    }
}

void Database::saveNotification(const QString &platform, const QString &status, const QString &message) {
    QSqlQuery query;
    query.prepare("INSERT INTO notifications (id, platform, status, message, timestamp) VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(QUuid::createUuid().toString());
    query.addBindValue(platform);
    query.addBindValue(status);
    query.addBindValue(message);
    query.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    if (query.exec()) {
        query.exec("DELETE FROM notifications WHERE id NOT IN (SELECT id FROM notifications ORDER BY timestamp DESC LIMIT 50)");
        Logger::info("Notification saved: " + platform + " - " + status);
    } else {
        Logger::error("Failed to save notification: " + query.lastError().text());
    }
}

QVector<Notification> Database::loadNotifications() {
    QVector<Notification> notifications;
    QSqlQuery query("SELECT platform, status, message, timestamp FROM notifications ORDER BY timestamp DESC");
    while (query.next()) {
        Notification notification;
        notification.platform = query.value(0).toString();
        notification.status = query.value(1).toString();
        notification.message = query.value(2).toString();
        notification.time = query.value(3).toString();
        notifications.append(notification);
    }
    Logger::info(QString("Loaded %1 notifications").arg(notifications.size()));
    return notifications;
}
