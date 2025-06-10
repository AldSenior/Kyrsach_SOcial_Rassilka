#include "mainwindow.h"
#include <QFileDialog>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QUuid>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Сервис публикации в соцсети");
    resize(800, 600);

    db = new Database("/home/arthur/social_media_services/data/social_media_posts.db");
    db->init();
    socialMedia = new SocialMedia;
    scheduler = new Scheduler(db, socialMedia, this);
    scheduler->start();

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Кнопка настроек
    QPushButton *settingsBtn = new QPushButton("Настройки");
    QObject::connect(settingsBtn, &QPushButton::clicked, this, &MainWindow::openSettings);
    mainLayout->addWidget(settingsBtn);

    // Секция создания поста
    QGroupBox *postGroup = new QGroupBox("Создать пост");
    QVBoxLayout *postLayout = new QVBoxLayout;
    messageEdit = new QTextEdit;
    messageEdit->setPlaceholderText("Введите текст поста...");
    postLayout->addWidget(new QLabel("Сообщение:"));
    postLayout->addWidget(messageEdit);

    targetList = new QListWidget;
    targetList->setSelectionMode(QAbstractItemView::NoSelection);
    loadTargets();
    postLayout->addWidget(new QLabel("Цели публикации:"));
    postLayout->addWidget(targetList);

    scheduleTimeEdit = new QDateTimeEdit;
    scheduleTimeEdit->setDateTime(QDateTime::currentDateTime());
    postLayout->addWidget(new QLabel("Время публикации (опционально):"));
    postLayout->addWidget(scheduleTimeEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QPushButton *postBtn = new QPushButton("Опубликовать");
    QPushButton *scheduleBtn = new QPushButton("Запланировать пост");
    QObject::connect(postBtn, &QPushButton::clicked, this, &MainWindow::postToSelected);
    QObject::connect(scheduleBtn, &QPushButton::clicked, this, &MainWindow::schedulePost);
    buttonLayout->addWidget(postBtn);
    buttonLayout->addWidget(scheduleBtn);
    postLayout->addLayout(buttonLayout);
    postGroup->setLayout(postLayout);
    mainLayout->addWidget(postGroup);

    // Секция шаблонов
    QGroupBox *templateGroup = new QGroupBox("Шаблоны");
    QVBoxLayout *templateLayout = new QVBoxLayout;
    templateNameEdit = new QLineEdit;
    templateNameEdit->setPlaceholderText("Название шаблона...");
    templateContentEdit = new QTextEdit;
    templateContentEdit->setPlaceholderText("Содержимое шаблона (Markdown)...");
    templateCombo = new QComboBox;
    QPushButton *saveTemplateBtn = new QPushButton("Сохранить шаблон");
    QPushButton *loadTemplateBtn = new QPushButton("Загрузить шаблон");
    QObject::connect(saveTemplateBtn, &QPushButton::clicked, this, &MainWindow::saveTemplate);
    QObject::connect(loadTemplateBtn, &QPushButton::clicked, this, &MainWindow::loadTemplate);
    templateLayout->addWidget(new QLabel("Название шаблона:"));
    templateLayout->addWidget(templateNameEdit);
    templateLayout->addWidget(new QLabel("Содержимое шаблона:"));
    templateLayout->addWidget(templateContentEdit);
    templateLayout->addWidget(new QLabel("Существующие шаблоны:"));
    templateLayout->addWidget(templateCombo);
    templateLayout->addWidget(saveTemplateBtn);
    templateLayout->addWidget(loadTemplateBtn);
    templateGroup->setLayout(templateLayout);
    mainLayout->addWidget(templateGroup);

    // История постов
    QGroupBox *historyGroup = new QGroupBox("История постов");
    QVBoxLayout *historyLayout = new QVBoxLayout;
    historyList = new QListWidget;
    historyLayout->addWidget(historyList);
    historyGroup->setLayout(historyLayout);
    mainLayout->addWidget(historyGroup);

    // Запланированные посты
    QGroupBox *scheduledGroup = new QGroupBox("Запланированные посты");
    QVBoxLayout *scheduledLayout = new QVBoxLayout;
    scheduledList = new QListWidget;
    scheduledList->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(scheduledList, &QListWidget::customContextMenuRequested, this, &MainWindow::showScheduledContextMenu);
    scheduledLayout->addWidget(scheduledList);
    scheduledGroup->setLayout(scheduledLayout);
    mainLayout->addWidget(scheduledGroup);

    // Уведомления
    QGroupBox *notificationGroup = new QGroupBox("Уведомления");
    QVBoxLayout *notificationLayout = new QVBoxLayout;
    notificationList = new QListWidget;
    notificationLayout->addWidget(notificationList);
    notificationGroup->setLayout(notificationLayout);
    mainLayout->addWidget(notificationGroup);

    refreshHistory();
    refreshScheduledPosts();
    refreshNotifications();
}

MainWindow::~MainWindow() {
    delete db;
    delete socialMedia;
    delete scheduler;
}

void MainWindow::loadTargets() {
    QSettings settings;
    QJsonDocument doc = QJsonDocument::fromJson(settings.value("post_targets").toByteArray());
    targets.clear();
    targetList->clear();
    for (const auto &val : doc.array()) {
        QJsonObject obj = val.toObject();
        PostTarget target;
        target.id = obj["id"].toString();
        target.platform = obj["platform"].toString();
        target.token = obj["token"].toString();
        target.target_id = obj["target_id"].toString();
        targets.append(target);

        QListWidgetItem *item = new QListWidgetItem(QString("%1: %2").arg(target.platform, target.target_id));
        item->setData(Qt::UserRole, target.id);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        targetList->addItem(item);
    }
}

PostTarget MainWindow::getTargetById(const QString &id) {
    for (const auto &target : targets) {
        if (target.id == id)
            return target;
    }
    return PostTarget();
}

void MainWindow::postToSelected() {
    QString message = messageEdit->toPlainText().trimmed();
    if (message.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Сообщение не может быть пустым!");
        return;
    }

    QStringList selectedTargets;
    for (int i = 0; i < targetList->count(); ++i) {
        if (targetList->item(i)->checkState() == Qt::Checked) {
            selectedTargets.append(targetList->item(i)->data(Qt::UserRole).toString());
        }
    }

    if (selectedTargets.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите хотя бы одну цель публикации!");
        return;
    }

    for (const auto &targetId : selectedTargets) {
        PostTarget target = getTargetById(targetId);
        QString result;
        if (target.platform == "Telegram") {
            result = socialMedia->postToTelegram(target.token, target.target_id, message);
        } else if (target.platform == "VK") {
            result = socialMedia->postToVK(target.token, target.target_id, message);
        }
        db->saveNotification(target.platform, result.contains("Успешно") ? "Отправлено" : "Ошибка", result);
        db->savePost(target.platform, message);
    }

    refreshHistory();
    refreshNotifications();
    QMessageBox::information(this, "Успех", "Пост отправлен на выбранные платформы!");
}

void MainWindow::schedulePost() {
    QString message = messageEdit->toPlainText().trimmed();
    QDateTime scheduleTime = scheduleTimeEdit->dateTime();
    if (message.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Сообщение не может быть пустым!");
        return;
    }
    if (scheduleTime <= QDateTime::currentDateTime()) {
        QMessageBox::warning(this, "Ошибка", "Время публикации должно быть в будущем!");
        return;
    }

    QStringList selectedTargets;
    for (int i = 0; i < targetList->count(); ++i) {
        if (targetList->item(i)->checkState() == Qt::Checked) {
            selectedTargets.append(targetList->item(i)->data(Qt::UserRole).toString());
        }
    }

    if (selectedTargets.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите хотя бы одну цель публикации!");
        return;
    }

    QJsonArray targetsArray;
    for (const auto &targetId : selectedTargets) {
        targetsArray.append(targetId);
    }
    db->schedulePost(message, scheduleTime, QJsonDocument(targetsArray).toJson());
    refreshScheduledPosts();
    QMessageBox::information(this, "Успех", "Пост запланирован!");
}

void MainWindow::saveTemplate() {
    QString name = templateNameEdit->text().trimmed();
    QString content = templateContentEdit->toPlainText().trimmed();
    if (name.isEmpty() || content.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Название и содержимое шаблона не могут быть пустыми!");
        return;
    }
    db->saveTemplate(name, content);
    templateCombo->addItem(name);
    QMessageBox::information(this, "Успех", "Шаблон сохранён!");
}

void MainWindow::loadTemplate() {
    QString name = templateCombo->currentText();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите шаблон!");
        return;
    }
    QString content = db->loadTemplateContent(name);
    messageEdit->setText(content);
}

void MainWindow::openSettings() {
    SettingsDialog dialog(this);
    dialog.exec();
    loadTargets();
}

void MainWindow::showScheduledContextMenu(const QPoint &pos) {
    QPoint globalPos = scheduledList->mapToGlobal(pos);
    QMenu menu(this);
    QAction *editAction = menu.addAction("Редактировать");
    QAction *deleteAction = menu.addAction("Удалить");
    QObject::connect(editAction, &QAction::triggered, this, &MainWindow::editScheduledPost);
    QObject::connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteScheduledPost);
    menu.exec(globalPos);
}

void MainWindow::editScheduledPost() {
    QListWidgetItem *item = scheduledList->currentItem();
    if (!item)
        return;
    QString id = item->data(Qt::UserRole).toString();
    // Логика редактирования (например, открыть диалог с текущими данными поста)
    QMessageBox::information(this, "Редактирование", "Функция редактирования в разработке!");
}

void MainWindow::deleteScheduledPost() {
    QListWidgetItem *item = scheduledList->currentItem();
    if (!item)
        return;
    QString id = item->data(Qt::UserRole).toString();
    db->deleteScheduledPost(id);
    refreshScheduledPosts();
    QMessageBox::information(this, "Успех", "Запланированный пост удалён!");
}

void MainWindow::refreshHistory() {
    historyList->clear();
    QVector<Post> posts = db->loadPosts();
    for (const auto &post : posts) {
        historyList->addItem(QString("[%1] %2: %3").arg(post.time, post.platform, post.content.left(50)));
    }
}

void MainWindow::refreshScheduledPosts() {
    scheduledList->clear();
    QVector<ScheduledPost> posts = db->loadScheduledPosts();
    for (const auto &post : posts) {
        QListWidgetItem *item = new QListWidgetItem(QString("[%1] %2").arg(post.schedule_time, post.content.left(50)));
        item->setData(Qt::UserRole, post.id);
        scheduledList->addItem(item);
    }
}

void MainWindow::refreshNotifications() {
    notificationList->clear();
    QVector<Notification> notifications = db->loadNotifications();
    for (const auto &notification : notifications) {
        notificationList->addItem(QString("[%1] %2: %3").arg(notification.time, notification.platform, notification.message));
    }
}
