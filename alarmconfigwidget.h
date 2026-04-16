#ifndef ALARMCONFIGWIDGET_H
#define ALARMCONFIGWIDGET_H

#include <QGroupBox>
#include <QVector>

class QSpinBox;
class QCheckBox;
class QPushButton;

class AlarmConfigWidget : public QGroupBox
{
    Q_OBJECT

public:
    explicit AlarmConfigWidget(QWidget *parent = nullptr);

    int maskValue1() const;
    int maskValue2() const;

signals:
    /// Emitted when user clicks write. addr1/addr2 are register addresses to write.
    void writeRequested(int value1, int value2);

private slots:
    void onCheckBoxChanged();
    void onSpinChanged();
    void onWriteClicked();

private:
    void setupUi();
    void updateSpinsFromCheckBoxes();
    void updateCheckBoxesFromSpin(const QVector<QCheckBox *> &boxes, int value);

    QSpinBox *m_maskSpin1 = nullptr;   // 报警屏蔽1
    QSpinBox *m_maskSpin2 = nullptr;   // 报警屏蔽2

    // Row1 checkboxes: 热继(9) + PTC检测(4) + 不平衡(3) = 16
    QVector<QCheckBox *> m_row1Boxes;
    // Row2 checkboxes: 过流(9)
    QVector<QCheckBox *> m_row2Boxes;

    QPushButton *m_writeBtn = nullptr;

    bool m_updating = false; // prevent recursion
};

#endif // ALARMCONFIGWIDGET_H
