#include "TransparentWindow.h"
#include <QCoreApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QtWidgets/QApplication>

#ifdef WIN32
#include <Windows.h>
#endif	// WIN32

void TransparentWindow::Setup(QWidget* w, bool expandToStateBar)
{
    w->setWindowFlag(Qt::WindowType::MSWindowsOwnDC, false);
    w->setWindowFlag(Qt::FramelessWindowHint);
    w->setWindowFlag(Qt::Tool);
    w->setWindowFlag(Qt::WindowStaysOnTopHint);
    w->setAttribute(Qt::WA_TranslucentBackground);
    QSize screenSize;
    if (!expandToStateBar)
        screenSize = QApplication::desktop()->availableGeometry().size();
    else
        screenSize = QApplication::desktop()->size();
    w->setGeometry(0, 0, screenSize.width(), screenSize.height());
    TransparentWindow::SetTransparentForMouse(w, true);
}

void TransparentWindow::SetTransparentForMouse(QWidget* w, bool flag)
{

#ifdef WIN32
    HWND hWnd = (HWND)(w->winId());
    if (flag)
        SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE);
    else
        SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) & (~WS_EX_TRANSPARENT));
#endif	// WIN32

}

bool TransparentWindow::processNativeEvent(QWidget* w, const QByteArray& b, void* m, long* r)
{

#ifdef WIN32
    MSG* msg = reinterpret_cast<MSG*>(m);
    if (msg->message == WM_MOUSEACTIVATE)
    {
        *r = MA_NOACTIVATE;
        return true;
    }
    if (msg->message == WM_ACTIVATE)
    {
        if (LOWORD(msg->wParam))
        {
            w->clearFocus();
            *r = 1;
            return true;
        }
    }
#endif // WIN32

    return false;
}
