#include "targetdialog.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

TargetDialog::TargetDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("Добавить цель публикации");
  setMinimumSize(300, 200);

  QVBoxLayout *mainLayout = new QVBoxLayout(this);

  platformCombo = new QComboBox;
  platformCombo->addItems({"Telegram", "VK"});
  mainLayout->addWidget(new QLabel("Платформа:"));
  mainLayout->addWidget(platformCombo);

  tokenEdit = new QLineEdit;
  tokenEdit->setPlaceholderText("Введите токен API...");
  mainLayout->addWidget(new QLabel("Токен API:"));
  mainLayout->addWidget(tokenEdit);

  targetIdEdit = new QLineEdit;
  targetIdEdit->setPlaceholderText("Введите ID чата или группы...");
  mainLayout->addWidget(new QLabel("ID чата/группы:"));
  mainLayout->addWidget(targetIdEdit);

  QHBoxLayout *buttonLayout = new QHBoxLayout;
  QPushButton *okBtn = new QPushButton("ОК");
  QPushButton *cancelBtn = new QPushButton("Отмена");
  connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
  connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
  buttonLayout->addWidget(okBtn);
  buttonLayout->addWidget(cancelBtn);
  mainLayout->addLayout(buttonLayout);
}

QString TargetDialog::getPlatform() const {
  return platformCombo->currentText();
}

QString TargetDialog::getToken() const { return tokenEdit->text().trimmed(); }

QString TargetDialog::getTargetId() const {
  return targetIdEdit->text().trimmed();
}
