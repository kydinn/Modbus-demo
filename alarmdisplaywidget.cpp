#include "alarmdisplaywidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>

static QLabel *createLight(const QString &text)
{
    auto *label = new QLabel(text);
    label->setStyleSheet("color: green;");
    return label;
}

AlarmDisplayWidget::AlarmDisplayWidget(QWidget *parent)
    : QGroupBox(QStringLiteral("报警信息"), parent)
{
    setupUi();

    connect(m_alarmSpin1, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AlarmDisplayWidget::onAlarmValue1Changed);
    connect(m_alarmSpin2, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AlarmDisplayWidget::onAlarmValue2Changed);
}

void AlarmDisplayWidget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    // --- Top row: spin boxes ---
    auto *topLayout = new QHBoxLayout;
    topLayout->addStretch();
    topLayout->addWidget(new QLabel(QStringLiteral("报警码值1")));
    m_alarmSpin1 = new QSpinBox;
    m_alarmSpin1->setRange(0, 65535);
    m_alarmSpin1->setValue(0);
    m_alarmSpin1->setMinimumWidth(120);
    topLayout->addWidget(m_alarmSpin1);
    topLayout->addSpacing(40);
    topLayout->addWidget(new QLabel(QStringLiteral("报警码值2")));
    m_alarmSpin2 = new QSpinBox;
    m_alarmSpin2->setRange(0, 65535);
    m_alarmSpin2->setValue(0);
    m_alarmSpin2->setMinimumWidth(120);
    topLayout->addWidget(m_alarmSpin2);
    topLayout->addStretch();
    mainLayout->addLayout(topLayout);

    // Helper to add a group of lights
    auto addLightGroup = [](QHBoxLayout *row, const QString &title,
                            int count, QVector<QLabel *> &lights) {
        row->addWidget(new QLabel(title));
        for (int i = 1; i <= count; ++i) {
            auto *light = createLight(QStringLiteral("○%1").arg(i));
            light->setProperty("groupIndex", i);
            lights.append(light);
            row->addWidget(light);
        }
        row->addSpacing(20);
    };

    // --- Row 1: 热继(9) PTC掉线(4) 不平衡(3) ---
    auto *row1Layout = new QHBoxLayout;
    addLightGroup(row1Layout, QStringLiteral("热　继："), 9, m_row1Lights);
    addLightGroup(row1Layout, QStringLiteral("PTC掉线："), 4, m_row1Lights);
    addLightGroup(row1Layout, QStringLiteral("不平衡："), 3, m_row1Lights);
    row1Layout->addStretch();
    mainLayout->addLayout(row1Layout);

    // --- Row 2: 过流(9) PTC超温(4) 缺相(3) ---
    auto *row2Layout = new QHBoxLayout;
    addLightGroup(row2Layout, QStringLiteral("过　流："), 9, m_row2Lights);
    addLightGroup(row2Layout, QStringLiteral("PTC超温："), 4, m_row2Lights);
    addLightGroup(row2Layout, QStringLiteral("缺　相："), 3, m_row2Lights);
    row2Layout->addStretch();
    mainLayout->addLayout(row2Layout);
}

void AlarmDisplayWidget::updateRow(const QVector<QLabel *> &lights, int value)
{
    for (int i = 0; i < lights.size(); ++i) {
        bool alarm = (value >> i) & 1;
        int idx = lights[i]->property("groupIndex").toInt();
        if (alarm) {
            lights[i]->setStyleSheet("color: red;");
            lights[i]->setText(QStringLiteral("●%1").arg(idx));
        } else {
            lights[i]->setStyleSheet("color: green;");
            lights[i]->setText(QStringLiteral("○%1").arg(idx));
        }
    }
}

void AlarmDisplayWidget::onAlarmValue1Changed(int value)
{
    updateRow(m_row1Lights, value);
}

void AlarmDisplayWidget::onAlarmValue2Changed(int value)
{
    updateRow(m_row2Lights, value);
}
