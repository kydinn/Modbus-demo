#include "alarmconfigwidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QSizePolicy>

AlarmConfigWidget::AlarmConfigWidget(QWidget *parent)
    : QGroupBox(QStringLiteral("报警配置"), parent)
{
    setupUi();

    // Checkbox → update spin values
    auto connectBoxes = [this](const QVector<QCheckBox *> &boxes) {
        for (auto *cb : boxes) {
            connect(cb, &QCheckBox::toggled, this, &AlarmConfigWidget::onCheckBoxChanged);
        }
    };
    connectBoxes(m_row1Boxes);
    connectBoxes(m_row2Boxes);

    // Spin → update checkboxes
    connect(m_maskSpin1, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AlarmConfigWidget::onSpinChanged);
    connect(m_maskSpin2, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AlarmConfigWidget::onSpinChanged);

    connect(m_writeBtn, &QPushButton::clicked,
            this, &AlarmConfigWidget::onWriteClicked);
}

void AlarmConfigWidget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    // --- Top: spin boxes ---
    auto *topLayout = new QHBoxLayout;
    topLayout->addWidget(new QLabel(QStringLiteral("报警屏蔽1")));
    m_maskSpin1 = new QSpinBox;
    m_maskSpin1->setRange(0, 65535);
    m_maskSpin1->setMinimumWidth(120);
    topLayout->addWidget(m_maskSpin1);
    topLayout->addSpacing(40);
    topLayout->addWidget(new QLabel(QStringLiteral("报警屏蔽2")));
    m_maskSpin2 = new QSpinBox;
    m_maskSpin2->setRange(0, 65535);
    m_maskSpin2->setMinimumWidth(120);
    topLayout->addWidget(m_maskSpin2);
    topLayout->addStretch();
    mainLayout->addLayout(topLayout);

    // --- Grid: header + checkbox rows ---
    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(4);
    grid->setVerticalSpacing(4);

    // Header row
    auto *headerLabel = new QLabel(QStringLiteral("通道屏蔽"));
    grid->addWidget(headerLabel, 0, 0);
    for (int i = 1; i <= 9; ++i) {
        auto *lbl = new QLabel(QString::number(i));
        lbl->setAlignment(Qt::AlignCenter);
        grid->addWidget(lbl, 0, i, Qt::AlignCenter);
    }

    // Helper: add a row of checkboxes into the grid
    int gridRow = 1;
    QVector<QLabel *> col0Labels;
    col0Labels.append(headerLabel);
    auto addCheckRow = [&](const QString &title, int count, QVector<QCheckBox *> &boxes) {
        auto *label = new QLabel(title);
        col0Labels.append(label);
        grid->addWidget(label, gridRow, 0);
        for (int i = 0; i < count; ++i) {
            auto *cb = new QCheckBox;
            boxes.append(cb);
            grid->addWidget(cb, gridRow, i + 1, Qt::AlignCenter);
        }
        ++gridRow;
    };

    // Row 1 groups: 热继(9) + PTC检测(4) + 不平衡(3)
    addCheckRow(QStringLiteral("热　继:"), 9, m_row1Boxes);
    addCheckRow(QStringLiteral("PTC检测:"), 4, m_row1Boxes);
    addCheckRow(QStringLiteral("不平衡:"), 3, m_row1Boxes);

    // Row 2 groups: 过流(9)
    addCheckRow(QStringLiteral("过　流:"), 9, m_row2Boxes);

    // Justify all column-0 labels to the same width using letter spacing
    {
        int maxWidth = 0;
        for (auto *lbl : col0Labels) {
            int w = lbl->fontMetrics().horizontalAdvance(lbl->text());
            maxWidth = qMax(maxWidth, w);
        }
        for (auto *lbl : col0Labels) {
            const QString text = lbl->text();
            int naturalWidth = lbl->fontMetrics().horizontalAdvance(text);
            if (text.length() > 1 && naturalWidth < maxWidth) {
                qreal spacing = qreal(maxWidth - naturalWidth) / (text.length() - 1);
                QFont font = lbl->font();
                font.setLetterSpacing(QFont::AbsoluteSpacing, spacing);
                lbl->setFont(font);
            }
        }
    }

    mainLayout->addLayout(grid);

    // --- Write button ---
    m_writeBtn = new QPushButton(QStringLiteral("写入屏蔽配置"));
    m_writeBtn->setMinimumHeight(32);
    mainLayout->addWidget(m_writeBtn);
}

int AlarmConfigWidget::maskValue1() const
{
    return m_maskSpin1->value();
}

int AlarmConfigWidget::maskValue2() const
{
    return m_maskSpin2->value();
}

void AlarmConfigWidget::onCheckBoxChanged()
{
    if (m_updating) return;
    m_updating = true;
    updateSpinsFromCheckBoxes();
    m_updating = false;
}

void AlarmConfigWidget::onSpinChanged()
{
    if (m_updating) return;
    m_updating = true;
    updateCheckBoxesFromSpin(m_row1Boxes, m_maskSpin1->value());
    updateCheckBoxesFromSpin(m_row2Boxes, m_maskSpin2->value());
    m_updating = false;
}

void AlarmConfigWidget::updateSpinsFromCheckBoxes()
{
    int val1 = 0;
    for (int i = 0; i < m_row1Boxes.size(); ++i) {
        if (m_row1Boxes[i]->isChecked())
            val1 |= (1 << i);
    }
    m_maskSpin1->setValue(val1);

    int val2 = 0;
    for (int i = 0; i < m_row2Boxes.size(); ++i) {
        if (m_row2Boxes[i]->isChecked())
            val2 |= (1 << i);
    }
    m_maskSpin2->setValue(val2);
}

void AlarmConfigWidget::updateCheckBoxesFromSpin(const QVector<QCheckBox *> &boxes, int value)
{
    for (int i = 0; i < boxes.size(); ++i) {
        boxes[i]->setChecked((value >> i) & 1);
    }
}

void AlarmConfigWidget::onWriteClicked()
{
    emit writeRequested(m_maskSpin1->value(), m_maskSpin2->value());
}
