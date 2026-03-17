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

    //keep the dialog title strong and consistent
    if (severity.compare("Critical", Qt::CaseInsensitive) == 0)
    {
        ui->titleLabel->setText("CRITICAL ALERT");
    }
    else if (severity.compare("Warning", Qt::CaseInsensitive) == 0)
    {
        ui->titleLabel->setText("WARNING ALERT");
    }
    else
    {
        ui->titleLabel->setText("SYSTEM ALERT");
    }

    ApplySeverityStyle(severity);

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

void AlertDialog::ApplySeverityStyle(const QString& severity)
{
    QString badgeStyle;
    QString severityTextStyle;

    if (severity.compare("Critical", Qt::CaseInsensitive) == 0)
    {
        badgeStyle =
            "QLabel {"
            " color: #FF5C5C;"
            " background-color: rgba(140, 24, 24, 0.35);"
            " border: 1px solid #FF5C5C;"
            " border-radius: 10px;"
            " padding: 3px 10px;"
            " font-weight: 700;"
            "}";

        severityTextStyle =
            "QLabel {"
            " color: #FF5C5C;"
            " font-weight: 700;"
            "}";
    }
    else if (severity.compare("Warning", Qt::CaseInsensitive) == 0)
    {
        badgeStyle =
            "QLabel {"
            " color: #FFB14A;"
            " background-color: rgba(120, 72, 8, 0.35);"
            " border: 1px solid #FFB14A;"
            " border-radius: 10px;"
            " padding: 3px 10px;"
            " font-weight: 700;"
            "}";

        severityTextStyle =
            "QLabel {"
            " color: #FFB14A;"
            " font-weight: 700;"
            "}";
    }
    else
    {
        badgeStyle =
            "QLabel {"
            " color: #8FA7C4;"
            " background-color: rgba(80, 95, 120, 0.25);"
            " border: 1px solid #8FA7C4;"
            " border-radius: 10px;"
            " padding: 3px 10px;"
            " font-weight: 700;"
            "}";

        severityTextStyle =
            "QLabel {"
            " color: #8FA7C4;"
            " font-weight: 700;"
            "}";
    }

    ui->severityBadge->setStyleSheet(badgeStyle);
    ui->severityValueLabel->setStyleSheet(severityTextStyle);
}
