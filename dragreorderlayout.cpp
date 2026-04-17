#include "dragreorderlayout.h"

#include <QMouseEvent>
#include <QApplication>
#include <QFrame>
#include <QScreen>
#include <QPainter>
#include <QPixmap>
#include <QLabel>

DragReorderLayout::DragReorderLayout(QWidget *parent)
    : QWidget(parent)
    , m_layout(new QVBoxLayout(this))
{
}

void DragReorderLayout::addWidget(QWidget *widget)
{
    m_layout->addWidget(widget);
    widget->installEventFilter(this);
}

bool DragReorderLayout::eventFilter(QObject *obj, QEvent *event)
{
    auto *widget = qobject_cast<QWidget *>(obj);
    if (!widget)
        return QWidget::eventFilter(obj, event);

    // Only handle top-level children managed by us (or the floating copy).
    if (!m_isDragging && indexOfWidget(widget) < 0)
        return QWidget::eventFilter(obj, event);

    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        auto *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton && !m_isDragging) {
            m_draggedWidget = widget;
            m_dragStartPos = me->globalPos();
            m_dragOffset = me->pos();
        }
        break;
    }
    case QEvent::MouseMove: {
        auto *me = static_cast<QMouseEvent *>(event);
        if (!(me->buttons() & Qt::LeftButton) || !m_draggedWidget)
            break;

        if (!m_isDragging) {
            if ((me->globalPos() - m_dragStartPos).manhattanLength() < DragThreshold)
                break;
            startDrag(m_draggedWidget, me->globalPos());
        }

        if (m_isDragging)
            updateDrag(me->globalPos());

        return true; // consume while dragging
    }
    case QEvent::MouseButtonRelease: {
        if (m_isDragging) {
            finishDrag();
            return true;
        }
        m_draggedWidget = nullptr;
        break;
    }
    default:
        break;
    }

    return QWidget::eventFilter(obj, event);
}

// ---- private helpers -------------------------------------------------------

void DragReorderLayout::startDrag(QWidget *widget, const QPoint &globalPos)
{
    m_isDragging = true;
    m_originIndex = indexOfWidget(widget);

    // Create a semi-transparent floating snapshot of the widget
    QPixmap pixmap(widget->size() * widget->devicePixelRatioF());
    pixmap.setDevicePixelRatio(widget->devicePixelRatioF());
    widget->render(&pixmap);

    m_floatingCopy = new QLabel(window());
    static_cast<QLabel *>(m_floatingCopy)->setPixmap(pixmap);
    m_floatingCopy->setFixedSize(widget->size());
    m_floatingCopy->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_floatingCopy->setWindowFlags(Qt::Window | Qt::FramelessWindowHint
                                   | Qt::WindowStaysOnTopHint
                                   | Qt::X11BypassWindowManagerHint);
    m_floatingCopy->setAttribute(Qt::WA_ShowWithoutActivating);
    m_floatingCopy->setWindowOpacity(0.75);
    m_floatingCopy->move(globalPos - m_dragOffset);
    m_floatingCopy->show();

    // Keep mouse grab on the original widget so dragging is not interrupted
    widget->grabMouse();

    // Insert placeholder where the widget was
    m_placeholder = new QFrame(this);
    m_placeholder->setFixedHeight(widget->height());
    m_placeholder->setStyleSheet(
        QStringLiteral("QFrame { border: 2px dashed #aaaaaa; "
                       "background: #f0f0f0; border-radius: 4px; }"));

    // Hide original and put placeholder in its slot
    m_layout->insertWidget(m_originIndex, m_placeholder);
    widget->hide();
}

void DragReorderLayout::updateDrag(const QPoint &globalPos)
{
    if (m_floatingCopy)
        m_floatingCopy->move(globalPos - m_dragOffset);

    int newIdx = dropIndexAt(globalPos);
    int phIdx = indexOfWidget(m_placeholder);
    if (newIdx != phIdx && newIdx >= 0) {
        m_layout->removeWidget(m_placeholder);
        m_layout->insertWidget(newIdx, m_placeholder);
    }
}

void DragReorderLayout::finishDrag()
{
    int dropIdx = indexOfWidget(m_placeholder);

    // Remove placeholder
    m_layout->removeWidget(m_placeholder);
    delete m_placeholder;
    m_placeholder = nullptr;

    // Delete floating copy
    delete m_floatingCopy;
    m_floatingCopy = nullptr;

    // Move the real widget to its new position
    if (m_draggedWidget) {
        m_draggedWidget->releaseMouse();
        int origIdx = indexOfWidget(m_draggedWidget);
        m_layout->removeWidget(m_draggedWidget);

        // Adjust dropIdx: placeholder was already removed;
        // if the dragged widget was before the drop position, shift down by 1.
        if (origIdx < dropIdx)
            dropIdx--;

        // Clamp to valid range
        int count = m_layout->count();
        if (dropIdx > count)
            dropIdx = count;

        m_layout->insertWidget(dropIdx, m_draggedWidget);
        m_draggedWidget->show();
    }

    m_draggedWidget = nullptr;
    m_isDragging = false;
    m_originIndex = -1;
}

int DragReorderLayout::dropIndexAt(const QPoint &globalPos) const
{
    QPoint localPos = mapFromGlobal(globalPos);
    int count = m_layout->count();

    for (int i = 0; i < count; ++i) {
        QLayoutItem *item = m_layout->itemAt(i);
        if (!item || !item->widget())
            continue;
        QRect geo = item->widget()->geometry();
        if (localPos.y() < geo.center().y())
            return i;
    }
    return qMax(0, count - 1);
}

int DragReorderLayout::indexOfWidget(QWidget *w) const
{
    for (int i = 0; i < m_layout->count(); ++i) {
        if (m_layout->itemAt(i)->widget() == w)
            return i;
    }
    return -1;
}
