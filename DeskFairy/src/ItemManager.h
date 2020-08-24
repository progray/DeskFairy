#pragma once

#include "MouseTracker.h"
#include "Settings.h"
#include <box2d/box2d.h>
#include <QList>
#include <QThread>
#include <QWidget>
#include <list>

class ItemManager : public QWidget
{
	Q_OBJECT
public:
	ItemManager(
		int width, int height,
		double* modelX, double*modelY,
		double* modelItemSize,
		MouseTracker* mouseTracker);

	~ItemManager();

	void AddItem(int type);

	void StartDraggingItemFromLive2D();

	void EndDragItemFromLive2D();

	void Clear();

	void mousePressEvent(QMouseEvent* e) override;

	void mouseReleaseEvent(QMouseEvent* e) override;

	void mouseDoubleClickEvent(QMouseEvent* e) override;

	void paintEvent(QPaintEvent* e) override;

	size_t ItemCount() const;

	bool IsDraggingItemFromLive2D() const;

	void UpdateSettings();

signals:
	void GiveItem(int type, bool drop);

private:
	struct Item
	{
		int type;
		int size;
		int time;
		b2Body* body = nullptr;
	};

	class QueryCallback : public b2QueryCallback
	{
	public:
		QueryCallback(const b2Vec2& point)
		{
			this->point = point;
			fixture = nullptr;
		}

		bool ReportFixture(b2Fixture* fixture) override
		{
			b2Body* body = fixture->GetBody();
			if (body->GetType() == b2_dynamicBody)
			{
				bool inside = fixture->TestPoint(point);
				if (inside)
				{
					this->fixture = fixture;
					return false;
				}
			}
			return true;
		}

		b2Vec2 point;
		b2Fixture* fixture;
	};

	bool _visible = true;

	QTimer* _timer = nullptr;
	QThread* _thread = nullptr;
	double _frameTime = 0;
	double _time = 0;

	b2World* _world = nullptr;
	b2Body* _nullBody = nullptr;
	b2Body* _border = nullptr;
	b2MouseJoint* _mouseJoint = nullptr;

	std::list<Item*> _items;
	Item* _draggingItem = nullptr;
	Item* _mouseOnItem = nullptr;
	bool _draggingItemFromLive2D = false;

	MouseTracker* _mouseTracker = nullptr;
	int _mouseX = 0;
	int _mouseY = 0;

	b2Vec2 _screenSize;
	b2Vec2 _worldSize;

	double* _modelX = nullptr;
	double* _modelY = nullptr;
	double* _modelItemSize = nullptr;

	void _OnMouseMove(int x, int y);

	void _OnDragStart(Item* item);

	void _OnDragEnd();

	void _RemoveItem(Item* item);

	Item* _FindItem(b2Body* body) const;

	void _Update();

	Item* _GetMouseItem() const;

	b2Vec2 _WorldToScreen(b2Vec2 pos) const;

	b2Vec2 _ScreenToWorld(b2Vec2 pos) const;

	void _SetupBorder();
};
