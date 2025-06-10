#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QComboBox>
#include <QListWidget>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QSettings>
#include <QJsonDocument>
#include "database.h"
#include "scheduler.h"
#include "socialmedia.h"
#include "settingsdialog.h"
#include "posttarget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void postToSelected();
    void schedulePost();
    void saveTemplate();
    void loadTemplate();
    void openSettings();
    void showScheduledContextMenu(const QPoint &pos);
    void editScheduledPost();
    void deleteScheduledPost();
    void refreshHistory();
    void refreshScheduledPosts();
    void refreshNotifications();

private:
    void loadTargets();
    PostTarget getTargetById(const QString &id);
    Database *db;
    SocialMedia *socialMedia;
    Scheduler *scheduler;
    QTextEdit *messageEdit;
    QListWidget *targetList;
    QDateTimeEdit *scheduleTimeEdit;
    QLineEdit *templateNameEdit;
    QTextEdit *templateContentEdit;
    QComboBox *templateCombo;
    QListWidget *historyList;
    QListWidget *scheduledList;
    QListWidget *notificationList;
    QVector<PostTarget> targets;
};

#endif // MAINWINDOW_H
