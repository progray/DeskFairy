#include "MouseTracker.h"
#include <QMouseEvent>
#include "Logger.h"
#include "Defines.h"

#ifdef WIN32
#include <Windows.h>
#endif	// WIN32

MouseTracker::MouseTracker(QObject* parent)
	: QObject(parent)
{
	_timer = new QTimer(this);
	_timer->start(1000 / Def::mouseTrackTimePerFrame);
	connect(_timer, &QTimer::timeout, this, &MouseTracker::_trackMouse);
}

MouseTracker::~MouseTracker()
{
	
}


void MouseTracker::_trackMouse()
{
	int mouseX = 0;
	int mouseY = 0;

#ifdef WIN32
	POINT p;
	GetCursorPos(&p);
	mouseX = p.x;
	mouseY = p.y;
#endif	// WIN32

	mouseMoveSignal(mouseX, mouseY);
}