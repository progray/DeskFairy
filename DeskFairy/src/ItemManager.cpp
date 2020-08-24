#include "ItemManager.h"
#include "Logger.h"
#include "Defines.h"
#include <QCoreApplication>
#include <algorithm>
#include <QMouseEvent>
#include <QPainter>
#include "TransparentWindow.h"

QImage itemImage[ITEMTYPE_COUNT];

ItemManager::ItemManager(
	int width, int height,
	double* modelX, double* modelY,
	double* modelItemSize,
	MouseTracker* mouseTracker)
	: _screenSize(width, height)
	, _worldSize(_ScreenToWorld(_screenSize))
	, _modelX(modelX), _modelY(modelY)
	, _modelItemSize(modelItemSize)
	, _mouseTracker(mouseTracker)
	, QWidget(nullptr)
{
	TransparentWindow::Setup(this, Settings::expandToStateBar);
	this->show();

	_frameTime = 1000.0 / Settings::itemFps;
	_timer = new QTimer;
	_timer->start(_frameTime);
	connect(_timer, &QTimer::timeout, this, &ItemManager::_Update);

	itemImage[ITEM_P] = QImage(QString(Def::resourcePath) + "/Item/item_p.png");
	itemImage[ITEM_B] = QImage(QString(Def::resourcePath) + "/Item/item_b.png");
	itemImage[ITEM_1UP] = QImage(QString(Def::resourcePath) + "/Item/item_1up.png");
	itemImage[ITEM_POINT] = QImage(QString(Def::resourcePath) + "/Item/item_point.png");

	connect(_mouseTracker, &MouseTracker::mouseMoveSignal, this, &ItemManager::_OnMouseMove);

	_world = new b2World(b2Vec2(0.0, Settings::itemGravity));
	_world->SetWarmStarting(true);

	b2BodyDef nullDef;
	_nullBody = _world->CreateBody(&nullDef);

	_SetupBorder();

	UpdateSettings();
}
	
ItemManager::~ItemManager()
{
	if (_draggingItem)
		_OnDragEnd();
	delete _timer;
	this->Clear();
	delete _world;
}

void ItemManager::AddItem(int type)
{
	_items.push_back(new Item);
	Item* item = _items.back();
	item->type = type;
	item->size = *_modelItemSize;
	item->time = _time;

	b2Vec2 pos(*_modelX, *_modelY);
	b2Vec2 box(*_modelItemSize * 0.5, *_modelItemSize * 0.5);
	pos = _ScreenToWorld(pos);
	box = _ScreenToWorld(box);

	b2BodyDef itemBodyDef;
	b2PolygonShape itemShape;
	b2FixtureDef itemFixtureDef;
	itemBodyDef.type = b2_dynamicBody;
	itemBodyDef.position.Set(pos.x, pos.y);
	item->body = _world->CreateBody(&itemBodyDef);
	itemShape.SetAsBox(box.x, box.y);
	itemFixtureDef.shape = &itemShape;
	itemFixtureDef.density = Def::itemDensity;
	itemFixtureDef.friction = Def::itemFriction;
	itemFixtureDef.restitution = Def::itemRestitution;
	item->body->CreateFixture(&itemFixtureDef);
}

void ItemManager::StartDraggingItemFromLive2D()
{
	_draggingItemFromLive2D = true;
	_OnDragStart(_items.back());
}

void ItemManager::EndDragItemFromLive2D()
{
	_OnDragEnd();
	_draggingItemFromLive2D = false;
}

void ItemManager::_RemoveItem(Item* item)
{
	if (_draggingItem == item)
	{
		_OnDragEnd();
	}

	auto it = std::find(_items.begin(), _items.end(), item);
	if (it != _items.end())
	{
		_world->DestroyBody((*it)->body);
		delete (*it);
		_items.erase(it);
	}
}

ItemManager::Item* ItemManager::_FindItem(b2Body* body) const
{
	for (auto it = _items.begin(); it != _items.end(); it++)
	{
		if ((*it)->body == body)
		{
			return *it;
		}
	}
	return nullptr;
}

void ItemManager::_Update()
{
	_world->Step(_frameTime * 0.001, 3, 1);
	this->update();
	_time += _frameTime;
}

ItemManager::Item* ItemManager::_GetMouseItem() const
{
	b2AABB cursor;
	cursor.lowerBound = _ScreenToWorld(b2Vec2(_mouseX, _mouseY) + b2Vec2(-0.5, -0.5));
	cursor.upperBound = _ScreenToWorld(b2Vec2(_mouseX, _mouseY) + b2Vec2(0.5, 0.5));
	ItemManager::QueryCallback q(_ScreenToWorld(b2Vec2(_mouseX, _mouseY)));
	_world->QueryAABB(&q, cursor);
	if (q.fixture)
	{
		return _FindItem(q.fixture->GetBody());
	}
	return nullptr;
}

void ItemManager::Clear()
{
	while (_items.size())
		_RemoveItem(_items.front());
}

void ItemManager::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::MouseButton::LeftButton)
	{
		_OnDragStart(_GetMouseItem());
	}
	if(e->button() == Qt::MouseButton::RightButton)
	{
		_RemoveItem(_GetMouseItem());
	}
}

void ItemManager::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() == Qt::MouseButton::LeftButton)
	{
		_OnDragEnd();
	}
}

void ItemManager::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (e->button() == Qt::MouseButton::LeftButton)
	{
		Item* item = _GetMouseItem();
		if(item && _time - item->time > 1000.0)
			_RemoveItem(item);
	}
}

void ItemManager::paintEvent(QPaintEvent* e)
{
	QPainter painter(this);
	for (auto item : _items)
	{
		b2Vec2 pos = _WorldToScreen(item->body->GetPosition());
		QRect itemRect(-item->size / 2, -item->size / 2, item->size, item->size);
		QImage& image = itemImage[item->type];
		painter.setOpacity((item == _mouseOnItem) ? 1.0 : 0.7);
		painter.resetMatrix();
		painter.translate(pos.x, pos.y);
		painter.rotate(item->body->GetAngle() / Def::pi * 180.0);
		
		painter.drawImage(itemRect, image);
	}
}

size_t ItemManager::ItemCount() const
{
	return _items.size();
}

bool ItemManager::IsDraggingItemFromLive2D() const
{
	return _draggingItemFromLive2D;
}

void ItemManager::UpdateSettings()
{
	TransparentWindow::Setup(this, Settings::expandToStateBar);
	_screenSize = b2Vec2(this->width(), this->height());
	_worldSize = _ScreenToWorld(_screenSize);
	_SetupBorder();

	_timer->stop();
	_frameTime = 1000.0 / Settings::itemFps;
	_timer->start(_frameTime);
	_world->SetGravity(b2Vec2(0.0, Settings::itemGravity));
	for (auto item : _items)
	{
		item->body->SetAwake(true);
	}
}

void ItemManager::_OnMouseMove(int x, int y)
{
	_mouseX = x;
	_mouseY = y;
	_mouseOnItem = _GetMouseItem();
	TransparentWindow::SetTransparentForMouse(this, !_mouseOnItem);
	if (_mouseJoint)
	{
		_mouseJoint->SetTarget(_ScreenToWorld(b2Vec2(x, y)));
	}
}

void ItemManager::_OnDragStart(Item* item)
{
	if (!item)
		return;
	_draggingItem = item;

	if (_mouseJoint)
	{
		_world->DestroyJoint(_mouseJoint);
	}
	b2MouseJointDef mouseJointDef;
	mouseJointDef.bodyA = _nullBody;
	mouseJointDef.bodyB = item->body;
	mouseJointDef.target = _ScreenToWorld(b2Vec2(_mouseX, _mouseY));
	mouseJointDef.collideConnected = false;
	mouseJointDef.maxForce = 10000000.0f * item->body->GetMass();
	_mouseJoint = (b2MouseJoint*)_world->CreateJoint(&mouseJointDef);
	item->body->SetAwake(true);
}

void ItemManager::_OnDragEnd()
{
	Item* toRemoveItem = nullptr;
	if (_draggingItem && _time - _draggingItem->time > 1000.0)
	{
		b2Vec2 dist = _draggingItem->body->GetPosition() - _ScreenToWorld(b2Vec2(*_modelX, *_modelY));
		b2Vec2 tol = _ScreenToWorld(b2Vec2(*_modelItemSize * 0.4, *_modelItemSize * 0.4));
		if (abs(dist.x) < tol.x && abs(dist.y) < tol.y)
		{
			emit GiveItem(_draggingItem->type, Settings::allowItemSpawn);
			toRemoveItem = _draggingItem;
		}		
	}

	_draggingItem = nullptr;
	if (_mouseJoint)
	{
		_world->DestroyJoint(_mouseJoint);
	}
	_mouseJoint = nullptr;

	if(toRemoveItem)
		_RemoveItem(toRemoveItem);
}

b2Vec2 ItemManager::_WorldToScreen(b2Vec2 pos) const
{
	return b2Vec2(pos.x * Def::itemWorldToScreenScale, pos.y * Def::itemWorldToScreenScale);
}

b2Vec2 ItemManager::_ScreenToWorld(b2Vec2 pos) const
{
	return b2Vec2(pos.x / Def::itemWorldToScreenScale, pos.y / Def::itemWorldToScreenScale);
}

void ItemManager::_SetupBorder()
{
	if (_border)
	{
		_world->DestroyBody(_border);
		_border = nullptr;
	}

	b2BodyDef bodyDef;
	b2FixtureDef fixDef;
	b2EdgeShape edgeShape;
	bodyDef.position.Set(0.0, 0.0);
	_border = _world->CreateBody(&bodyDef);
	fixDef.shape = &edgeShape;
	fixDef.density = 0.0f;
	fixDef.restitution = Def::itemRestitution;
	//bottom
	edgeShape.SetTwoSided(b2Vec2(0.0, 0.0), b2Vec2(_worldSize.x, 0.0));
	_border->CreateFixture(&fixDef);
	//top
	edgeShape.SetTwoSided(b2Vec2(0.0, _worldSize.y), b2Vec2(_worldSize.x, _worldSize.y));
	_border->CreateFixture(&fixDef);
	//left
	edgeShape.SetTwoSided(b2Vec2(0.0, 0.0), b2Vec2(0.0, _worldSize.y));
	_border->CreateFixture(&fixDef);
	//right
	edgeShape.SetTwoSided(b2Vec2(_worldSize.x, 0.0), b2Vec2(_worldSize.x, _worldSize.y));
	_border->CreateFixture(&fixDef);
}
