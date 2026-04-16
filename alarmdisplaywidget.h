#ifndef ALARMDISPLAYWIDGET_H
#define ALARMDISPLAYWIDGET_H

#include <QGroupBox>
#include <QVector>

class QSpinBox;
class QLabel;

class AlarmDisplayWidget : public QGroupBox
{
    Q_OBJECT

public:
    explicit AlarmDisplayWidget(QWidget *parent = nullptr);

    QSpinBox *alarmSpin1() const { return m_alarmSpin1; }
    QSpinBox *alarmSpin2() const { return m_alarmSpin2; }

private slots:
    void onAlarmValue1Changed(int value);
    void onAlarmValue2Changed(int value);

private:
    void setupUi();
    void updateRow(const QVector<QLabel *> &lights, int value);

    QSpinBox *m_alarmSpin1 = nullptr;
    QSpinBox *m_alarmSpin2 = nullptr;

    // Row 1: 热继(9) + PTC掉线(4) + 不平衡(3) = 16
    QVector<QLabel *> m_row1Lights;
    // Row 2: 过流(9) + PTC超温(4) + 缺相(3) = 16
    QVector<QLabel *> m_row2Lights;
};

#endif // ALARMDISPLAYWIDGET_H
