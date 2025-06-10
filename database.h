#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QString>
#include <QVector>
#include <QDateTime>

struct Post {
    QString platform;
    QString content;
    QString time;
};

struct Template {
    QString id;
    QString name;
    QString content;
};

struct ScheduledPost {
    QString id;
    QString content;
    QString schedule_time;
    QString targets; // JSON array of target IDs
};

struct Notification {
    QString platform;
    QString status;
    QString message;
    QString time;
};

class Database {
public:
    Database(const std::string &dbPath);
    ~Database();
    void init();
    void savePost(const QString &platform, const QString &content);
    QVector<Post> loadPosts();
    void saveTemplate(const QString &name, const QString &content);
    QVector<Template> loadTemplates();
    QString loadTemplateContent(const QString &name);
    void schedulePost(const QString &content, const QDateTime &scheduleTime, const QString &targets);
    QVector<ScheduledPost> loadScheduledPosts();
    QVector<ScheduledPost> loadAndDeleteDuePosts();
    void deleteScheduledPost(const QString &id);
    void saveNotification(const QString &platform, const QString &status, const QString &message);
    QVector<Notification> loadNotifications();

private:
    QSqlDatabase db;
};

#endif // DATABASE_H
