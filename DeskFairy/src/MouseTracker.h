#pragma once

#include <QObject>
#include <QTimer>

class MouseTracker : public QObject
{
    Q_OBJECT

public:
    MouseTracker(QObject*parent = nullptr);

    ~MouseTracker();

signals:
    void mouseMoveSignal(int x, int y);

private:
    QTimer* _timer = nullptr;

    void _trackMouse();
};

