#include "wheelslipconfiguration.h"
#include "ui_wheelslipconfiguration.h"
#include "mainwindow.h"
#include <QDebug>
#include "settings.h"

WheelSlipConfiguration::WheelSlipConfiguration(MainWindow *parent)
    : QDialog(parent)
    , ui(new Ui::WheelSlipConfiguration)
    , m_parent(parent)
{
    ui->setupUi(this);
    this->setFixedSize(400, 300);

    readDataFromSettings();
}

WheelSlipConfiguration::~WheelSlipConfiguration()
{
    m_parent->setEnabled(true);
    delete ui;
}

void WheelSlipConfiguration::readDataFromSettings()
{
    Settings *settings = Settings::getInstance();
    m_gasIndex = settings->getGasIndex();
    m_brakeIndex = settings->getBrakeIndex();
    m_bumpingIndex = settings->getBumpingIndex();
    ui->gasIndexPercLabel->setText(QString::number(m_gasIndex) + " %");
    ui->brakeIndexPercLabel->setText(QString::number(m_brakeIndex) + " %");
    ui->bumpingIndexPercLabel->setText(QString::number(m_bumpingIndex));
    ui->gasIndexSlider->setValue(m_gasIndex);
    ui->brakeIndexSlider->setValue(m_brakeIndex);
    ui->bumpingIndexSlider->setValue(m_bumpingIndex);
}

void WheelSlipConfiguration::on_buttonBox_rejected()
{
    m_parent->setEnabled(true);
    readDataFromSettings();
}

void WheelSlipConfiguration::on_buttonBox_accepted()
{
    m_parent->setEnabled(true);

    Settings *settings = Settings::getInstance();
    settings->setGasIndex(m_gasIndex);
    settings->setBrakeIndex(m_brakeIndex);
    settings->setBumpingIndex(m_bumpingIndex);
}

void WheelSlipConfiguration::on_gasIndexSlider_valueChanged(qint32 value)
{
    ui->gasIndexPercLabel->setText(QString::number(value) + " %");
    m_gasIndex = value;
}

void WheelSlipConfiguration::on_brakeIndexSlider_valueChanged(qint32 value)
{
    ui->brakeIndexPercLabel->setText(QString::number(value) + " %");
    m_brakeIndex = value;
}

void WheelSlipConfiguration::on_bumpingIndexSlider_valueChanged(int value)
{
    ui->bumpingIndexPercLabel->setText(QString::number(value));
    m_bumpingIndex = value;
}

void WheelSlipConfiguration::on_WheelSlipConfiguration_destroyed()
{
    m_parent->setEnabled(true);
}

