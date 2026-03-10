#include "AlertDialog.h"
#include "ui_AlertDialog.h"

AlertDialog::AlertDialog(QWidget* parent)
    : QDialog(parent),
    ui(new Ui::AlertDialog)
{
    ui->setupUi(this);

    //keep dialog fixed to my design size
    setFixedSize(size());

    //connect buttons from .ui file
    connect(ui->acknowledgeButton, &QPushButton::clicked,
        this, &AlertDialog::OnAcknowledgeClicked);

    connect(ui->viewAlertsButton, &QPushButton::clicked,
        this, &AlertDialog::OnViewAlertsClicked);

    connect(ui->closeButton, &QPushButton::clicked,
        this, &AlertDialog::OnCloseClicked);
}

AlertDialog::~AlertDialog()
{
    delete ui;
}

void AlertDialog::SetAlertData(
    const QString& metric,
    const QString& message,
    const QString& valueNumber,
    const QString& valueUnit,
    const QString& severity,
    const QString& timestamp
) {
    ui->metricValueLabel->setText(metric);
    ui->messageValueLabel->setText(message);
    ui->valueNumberLabel->setText(valueNumber);
    ui->valueUnitLabel->setText(valueUnit);
    ui->severityValueLabel->setText(severity);
    ui->severityBadge->setText(severity);
    ui->timestampLabel->setText(timestamp);

    // keep the dialog title strong and consistent
    ui->titleLabel->setText("CRITICAL ALERT");

}

void AlertDialog::SetTitle(const QString& title)
{
    ui->titleLabel->setText(title);
}

void AlertDialog::OnAcknowledgeClicked()
{
    emit AcknowledgeRequested();
    accept();
}

void AlertDialog::OnViewAlertsClicked()
{
    emit ViewAlertsRequested();
    accept();
}

void AlertDialog::OnCloseClicked()
{
    reject();
}