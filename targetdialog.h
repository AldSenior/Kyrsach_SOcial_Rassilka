#ifndef TARGETDIALOG_H
#define TARGETDIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>

class TargetDialog : public QDialog {
  Q_OBJECT
public:
  TargetDialog(QWidget *parent = nullptr);
  QString getPlatform() const;
  QString getToken() const;
  QString getTargetId() const;

private:
  QComboBox *platformCombo;
  QLineEdit *tokenEdit;
  QLineEdit *targetIdEdit;
};

#endif
