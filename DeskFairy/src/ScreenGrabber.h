#pragma once

#include <QObject>
#include <QScreen>
#include <QWidget>
#include <QThread>
#include <QGroupBox>
#include <QFileDialog>

class ScreenshotWidget : public QWidget
{
	Q_OBJECT
public:
	ScreenshotWidget();
	~ScreenshotWidget();

	void StartEdit(QPixmap image);

	void EndEdit(bool accept);

	void paintEvent(QPaintEvent* e) override;

	void mouseMoveEvent(QMouseEvent* e) override;

	void mousePressEvent(QMouseEvent* e) override;

	void mouseReleaseEvent(QMouseEvent* e) override;

	void keyPressEvent(QKeyEvent* e) override;

signals:
	void Edited(QPixmap* image);

private:
	enum DragPointId
	{
		P_NONE,
		P_BR, P_BL, P_TR, P_TL, 
		P_MR, P_ML, P_MT, P_MB,
		P_CTR, DRAGPOINT_COUNT
	};

	int _mouseX = 0;
	int _mouseY = 0;
	int _width = 0;
	int _height = 0;
	bool _selected = false;
	bool _dragging = false;
	int _draggingPoint = P_NONE;
	int _dragStartMouseX = 0;
	int _dragStartMouseY = 0;
	QPoint _p1;
	QPoint _p2;
	QPoint _dragStartP1;
	QPoint _dragStartP2;

	QGroupBox* _box = nullptr;
	QSize boxSize;

	QPixmap _image;

	QRect _GetSelect() const;

	void _Normalize();

	bool _IsMouseOnSelect(int expand) const;

	QPoint _GetDragPoint(int id) const;

	int _GetMouseOnDragPoint() const;

	void _Accpet();

	void _Reject();

	void _Reselect();

	void _UpdateBoxPos();
};


class FileSelector : public QObject
{
	Q_OBJECT
public:
	FileSelector();
	~FileSelector();
	void Select();

signals:
	void Selected(QString path);

private:
	QFileDialog* _file = nullptr;
};


class ScreenGrabber : public QObject
{
	Q_OBJECT
public:
	ScreenGrabber();

	~ScreenGrabber();

	void EditExistedFile();

	void Grab(bool fast);

	void OnEdited(QPixmap* image);

	bool IsEditing() const;

	void OnFileSelected(QString path);

signals:
	void Grabbed();

	void Edited(bool accept);

	void EditScreenshot(QPixmap image);

	void SelectFile();

	void EditStart();

private:

	QThread* _thread = nullptr;

	ScreenshotWidget* _ui = nullptr;

	FileSelector* _selector = nullptr;

	bool _editing = false;
};
