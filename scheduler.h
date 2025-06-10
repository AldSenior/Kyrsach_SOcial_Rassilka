#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <QObject>
#include <QTimer>
#include "database.h"
#include "socialmedia.h"

class Scheduler : public QObject {
    Q_OBJECT
public:
    Scheduler(Database *db, SocialMedia *sm, QObject *parent = nullptr);
    void start();

private slots:
    void processScheduledPosts();

private:
    Database *db;
    SocialMedia *socialMedia;
    QTimer *timer;
};

#endif // SCHEDULER_H
