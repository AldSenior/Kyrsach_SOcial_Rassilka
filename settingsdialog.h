#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "targetdialog.h"
#include <QDialog>
#include <QListWidget>
#include <QPushButton>

class SettingsDialog : public QDialog {
  Q_OBJECT
public:
  SettingsDialog(QWidget *parent = nullptr);

private slots:
  void addTarget();
  void editTarget();
  void removeTarget();
  void exportSettings();
  void importSettings();

private:
  QListWidget *targetList;
};

#endif
