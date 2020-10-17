#ifndef WindFanConfiguration_56FAD1DB3B45449EB6DD9B22CE7DF09D
#define WindFanConfiguration_56FAD1DB3B45449EB6DD9B22CE7DF09D

#include <QDialog>

class MainWindow;

namespace Ui {
class WindFanConfiguration;
}

class WindFanConfiguration : public QDialog
{
    Q_OBJECT

public:
    explicit WindFanConfiguration(MainWindow *parent = nullptr);
    ~WindFanConfiguration();

private Q_SLOTS:
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();
    void on_WindFanConfiguration_destroyed();
    void on_windFanIndexSlider_valueChanged(int value);

private:
    void readDataFromSettings();

    Ui::WindFanConfiguration *ui;
    MainWindow* m_parent;

    qint32 m_windFanIndex;
};

#endif // WindFanConfiguration_56FAD1DB3B45449EB6DD9B22CE7DF09D
