#include "windfanconfiguration.h"
#include "ui_windfanconfiguration.h"
#include "mainwindow.h"
#include <QDebug>
#include "settings.h"

WindFanConfiguration::WindFanConfiguration(MainWindow *parent)
    : QDialog(parent)
    , ui(new Ui::WindFanConfiguration)
    , m_parent(parent)
{
    ui->setupUi(this);
    this->setFixedSize(400, 140);

    readDataFromSettings();
}

WindFanConfiguration::~WindFanConfiguration()
{
    m_parent->setEnabled(true);
    delete ui;
}

void WindFanConfiguration::readDataFromSettings()
{
    Settings *settings = Settings::getInstance();
    m_windFanIndex = settings->getWindFanIndex();
    ui->windFanIndexLabel->setText(QString::number(m_windFanIndex));
    ui->windFanIndexSlider->setValue(m_windFanIndex);
}

void WindFanConfiguration::on_buttonBox_rejected()
{
    m_parent->setEnabled(true);
    readDataFromSettings();
}

void WindFanConfiguration::on_buttonBox_accepted()
{
    m_parent->setEnabled(true);

    Settings *settings = Settings::getInstance();
    settings->setWindFanIndex(m_windFanIndex);
}

void WindFanConfiguration::on_windFanIndexSlider_valueChanged(qint32 value)
{
    ui->windFanIndexLabel->setText(QString::number(value));
    m_windFanIndex = value;
}

void WindFanConfiguration::on_WindFanConfiguration_destroyed()
{
    m_parent->setEnabled(true);
}

