#pragma once

#include <QWidget>

class TransparentWindow
{
public:

	static void Setup(QWidget *w, bool expandToStateBar);

	static void SetTransparentForMouse(QWidget* w, bool flag);

	static bool processNativeEvent(QWidget* w, const QByteArray& b, void* m, long* r);

};
