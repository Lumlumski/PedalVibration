#ifndef WHEELSLIPCONFIGURATION_H
#define WHEELSLIPCONFIGURATION_H

#include <QDialog>

class MainWindow;

namespace Ui {
class WheelSlipConfiguration;
}

class WheelSlipConfiguration : public QDialog
{
    Q_OBJECT

public:
    explicit WheelSlipConfiguration(MainWindow *parent = nullptr);
    ~WheelSlipConfiguration();

private Q_SLOTS:
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();

    void on_gasIndexSlider_valueChanged(qint32 value);
    void on_brakeIndexSlider_valueChanged(int value);

    void on_WheelSlipConfiguration_destroyed();

private:
    void readDataFromSettings();

    Ui::WheelSlipConfiguration *ui;
    MainWindow* m_parent;

    qint32 m_brakeIndex;
    qint32 m_gasIndex;
};

#endif // WHEELSLIPCONFIGURATION_H
