#pragma once

#include <QDialog>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui {
    class AlertDialog;
}
QT_END_NAMESPACE

class AlertDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AlertDialog(QWidget* parent = nullptr);
    ~AlertDialog();

    //populates the dialog with alert content
    void SetAlertData(
        const QString& metric,
        const QString& message,
        const QString& valueNumber,
        const QString& valueUnit,
        const QString& severity,
        const QString& timestamp
    );

    //helper to change title text if needed later
    void SetTitle(const QString& title);

signals:
    void AcknowledgeRequested();
    void ViewAlertsRequested();

private slots:
    void OnAcknowledgeClicked();
    void OnViewAlertsClicked();
    void OnCloseClicked();

private:
    Ui::AlertDialog* ui;
};