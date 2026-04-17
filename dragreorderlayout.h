#ifndef DRAGREORDERLAYOUT_H
#define DRAGREORDERLAYOUT_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPoint>

class QFrame;

class DragReorderLayout : public QWidget
{
    Q_OBJECT

public:
    explicit DragReorderLayout(QWidget *parent = nullptr);

    void addWidget(QWidget *widget);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void startDrag(QWidget *widget, const QPoint &globalPos);
    void updateDrag(const QPoint &globalPos);
    void finishDrag();
    int dropIndexAt(const QPoint &globalPos) const;
    int indexOfWidget(QWidget *w) const;

    QVBoxLayout *m_layout = nullptr;

    // drag state
    QWidget *m_draggedWidget = nullptr;
    QWidget *m_floatingCopy = nullptr;   // semi-transparent floating overlay
    QFrame  *m_placeholder = nullptr;    // gray dashed-border placeholder
    QPoint   m_dragStartPos;
    QPoint   m_dragOffset;               // cursor offset within the widget
    int      m_originIndex = -1;
    bool     m_isDragging = false;

    static constexpr int DragThreshold = 8;
};

#endif // DRAGREORDERLAYOUT_H
