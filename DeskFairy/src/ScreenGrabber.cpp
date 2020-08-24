#include <QApplication>
#include <QPainter>
#include <QDesktopWidget>
#include "ScreenGrabber.h"
#include "Settings.h"
#include "Logger.h"
#include "Defines.h"
#include "TransparentWindow.h"

ScreenshotWidget::ScreenshotWidget() : QWidget(nullptr)
{
    _width = QApplication::desktop()->geometry().width();
    _height = QApplication::desktop()->geometry().height();
    this->setMouseTracking(true);
    this->setWindowState(this->windowState() ^ Qt::WindowFullScreen);

    QPushButton* acceptBtn = new QPushButton();
    acceptBtn->setMaximumSize(QSize(Def::editButtonSize, Def::editButtonSize));
    acceptBtn->setMinimumSize(QSize(Def::editButtonSize, Def::editButtonSize));
 //   acceptBtn->setAttribute(Qt::WA_TranslucentBackground);
    acceptBtn->setIcon(QIcon(QString(Def::resourcePath) + "/Icon/accept.png"));
    acceptBtn->setIconSize(this->size());
    connect(acceptBtn, &QPushButton::clicked, this, &ScreenshotWidget::_Accpet);

    QPushButton* rejectBtn = new QPushButton();
    rejectBtn->setMaximumSize(QSize(Def::editButtonSize, Def::editButtonSize));
    rejectBtn->setMinimumSize(QSize(Def::editButtonSize, Def::editButtonSize));
 //   rejectBtn->setAttribute(Qt::WA_TranslucentBackground);
    rejectBtn->setIcon(QIcon(QString(Def::resourcePath) + "/Icon/reject.png"));
    rejectBtn->setIconSize(this->size());
    connect(rejectBtn, &QPushButton::clicked, this, &ScreenshotWidget::_Reject);

    QPushButton* reselectBtn = new QPushButton();
    reselectBtn->setMaximumSize(QSize(Def::editButtonSize, Def::editButtonSize));
    reselectBtn->setMinimumSize(QSize(Def::editButtonSize, Def::editButtonSize));
//    reselectBtn->setAttribute(Qt::WA_TranslucentBackground);
    reselectBtn->setIcon(QIcon(QString(Def::resourcePath) + "/Icon/reselect.png"));
    reselectBtn->setIconSize(this->size());
    connect(reselectBtn, &QPushButton::clicked, this, &ScreenshotWidget::_Reselect);

    QHBoxLayout* lay = new QHBoxLayout;
    lay->addWidget(reselectBtn);
    lay->addWidget(rejectBtn);
    lay->addWidget(acceptBtn);
    lay->setMargin(1);

    _box = new QGroupBox(this);
    _box->resize(Def::editButtonSize * 3.5, Def::editButtonSize + 4);
    _box->setLayout(lay);
    _box->setStyleSheet("#groupBox{border:0px solid;}");
    _box->hide();
    boxSize = _box->size();
}

ScreenshotWidget::~ScreenshotWidget()
{

}

void ScreenshotWidget::StartEdit(QPixmap image)
{
    this->show();
    _box->hide();
    _image = image;
    _p1 = QPoint(0, 0);
    _p2 = QPoint(_width, _height);
    _selected = false;
    _dragging = false;
    update();
}

void ScreenshotWidget::EndEdit(bool accept)
{
    if (accept)
    {
        if (_selected)
        {
            int w = std::abs(_p1.x() - _p2.x()) * (_image.width() / (double)_width);
            int h = std::abs(_p1.y() - _p2.y()) * (_image.height() / (double)_height);
            int x = std::min(_p1.x(), _p2.x()) * (_image.width() / (double)_width);
            int y = std::min(_p1.y(), _p2.y()) * (_image.height() / (double)_height);
            _image = _image.copy(QRect(x, y, w, h));
        }
    }

    _box->hide();
    this->hide();  
    emit Edited(accept ? &_image : nullptr);
    _selected = false;
    _dragging = false;
}

void ScreenshotWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    QBrush brush = painter.brush();
    QPen pen = painter.pen();

    //暗色背景（让截图变暗
    brush.setColor(QColor(15, 10, 0)); 
    brush.setStyle(Qt::BrushStyle::SolidPattern);
    pen.setWidth(0);
    painter.setBrush(brush);
    painter.setPen(pen);
    painter.setOpacity(1.0);
    painter.drawRect(this->rect());
    //半透明截图
    painter.setBrush(Qt::BrushStyle::SolidPattern);
    painter.setOpacity(0.6);
    painter.drawPixmap(this->rect(), _image);

    //鼠标定位
    painter.setOpacity(1.0);
    pen.setStyle(Qt::PenStyle::DashLine);
    pen.setColor(QColor(255, 255, 255, 127));
    pen.setWidth(1);
    painter.setPen(pen);
    painter.setBrush(Qt::BrushStyle::NoBrush);
    QPoint cursor = _draggingPoint ? _GetDragPoint(_GetMouseOnDragPoint()) : QPoint(_mouseX, _mouseY);
    painter.drawLine(QPoint(cursor.x(), 0), QPoint(cursor.x(), _height));
    painter.drawLine(QPoint(0, cursor.y()), QPoint(_width, cursor.y()));

    if (_selected || _dragging)
    {
        //不透明选择区域截图
        QRect rect = _GetSelect();
        int w = rect.width() * (_image.width() / (double)_width);
        int h = rect.height() * (_image.height() / (double)_height);
        int x = rect.x() * (_image.width() / (double)_width);
        int y = rect.y() * (_image.height() / (double)_height);
        painter.drawPixmap(_GetSelect(), _image, QRect(x, y, w, h));

        //选择框
        pen.setStyle(Qt::PenStyle::SolidLine);
        pen.setColor(QColor(200, 30, 10));
        pen.setWidth(5);
        painter.setPen(pen);
        painter.setBrush(Qt::BrushStyle::NoBrush);

        QPoint right = QPoint(rect.width() / 4, 0);
        QPoint left = QPoint(-rect.width() / 4, 0);
        QPoint up = QPoint(0, -rect.height() / 4);
        QPoint down = QPoint(0, rect.height() / 4);

        painter.drawLine(rect.bottomLeft(), rect.bottomLeft() + up);
        painter.drawLine(rect.bottomLeft(), rect.bottomLeft() + right);
        painter.drawLine(rect.topLeft(), rect.topLeft() + down);
        painter.drawLine(rect.topLeft(), rect.topLeft() + right);
        painter.drawLine(rect.bottomRight(), rect.bottomRight() + up);
        painter.drawLine(rect.bottomRight(), rect.bottomRight() + left);
        painter.drawLine(rect.topRight(), rect.topRight() + down);
        painter.drawLine(rect.topRight(), rect.topRight() + left);

        right /= 4, left /= 4, up /= 4, down /= 4;
        painter.drawLine(rect.center() + left, rect.center() + right);
        painter.drawLine(rect.center() + up, rect.center() + down);

        if (_selected && _IsMouseOnSelect(Def::draggingPointSize / 2))
        {
            //拖动点
            int nowPoint = _draggingPoint == P_CTR ? P_CTR : _GetMouseOnDragPoint();
            for (int id = DRAGPOINT_COUNT - 1; id >= 1; id--)
            {
                QPoint pos = _GetDragPoint(id);
                int s = Def::draggingPointSize;

                brush.setColor(id != nowPoint ? QColor(255, 200, 150) : QColor(255, 255, 255));
                brush.setStyle(Qt::BrushStyle::SolidPattern);
                pen.setWidth(2);
                painter.setBrush(brush);
                painter.setPen(pen);
                painter.setOpacity(1.0);
                painter.drawRect(QRect(pos.x() - s / 2, pos.y() - s / 2, s, s));
            }
        }
    }
}

void ScreenshotWidget::mouseMoveEvent(QMouseEvent* e)
{
    _mouseX = e->globalPos().x();
    _mouseY = e->globalPos().y();
    if (_dragging)
    {
        //已选择时拖动调整大小
        if (_selected)
        {
            //按到拖动点时开始调整大小
            if (_draggingPoint)
            {               
                QPoint delta = QPoint(_mouseX - _dragStartMouseX, _mouseY - _dragStartMouseY);
                switch (_draggingPoint)
                {
                //角落点：拖动点就是p2 直接调整即可
                case ScreenshotWidget::P_BR:
                case ScreenshotWidget::P_BL:
                case ScreenshotWidget::P_TR:
                case ScreenshotWidget::P_TL:
                    _p2 = _dragStartP2 + delta;
                    break;
                //左右边界点：拖动点与p2X相关
                case ScreenshotWidget::P_MR:                  
                case ScreenshotWidget::P_ML:
                    _p2.setX(_dragStartP2.x() + delta.x());
                    break;
                //上下边界点：拖动点与p2Y相关
                case ScreenshotWidget::P_MT:
                case ScreenshotWidget::P_MB:
                    _p2.setY(_dragStartP2.y() + delta.y());
                    break;
                //中心点：对p1p2进行整体位移
                case ScreenshotWidget::P_CTR:
                    _p1 = _dragStartP1 + delta;
                    _p2 = _dragStartP2 + delta;
                    {
                        QPoint size = _p2 - _p1;
                        //限制
                        if (_p1.x() < 0) _p1.setX(0), _p2.setX(size.x());
                        if (_p1.y() < 0) _p1.setY(0), _p2.setY(size.y());
                        if (_p2.x() > _width) _p2.setX(_width), _p1.setX(_width - size.x());
                        if (_p2.y() > _height) _p2.setY(_height), _p1.setY(_height - size.y());
                    }
                    break;
                default:
                    break;
                }
            }
        }
        //未选择时拖动选择
        else
        {
            _p2 = QPoint(_mouseX, _mouseY);
        }
    }
    update();
}

void ScreenshotWidget::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::MouseButton::LeftButton && !_dragging)
    {
        _dragging = true;
        //已选择时
        if (_selected)
        {
            //按到拖动点时开始调整大小
            int id = _GetMouseOnDragPoint();
            if (id)
            {     
                _box->hide();     
                _draggingPoint = id;
                QRect rect = this->_GetSelect();
                //设置p1、p2
                //p1为另外一个点 p2为拖动的点或和拖动相关的点
                switch (_draggingPoint)
                {
                case ScreenshotWidget::P_BR:
                    _p1 = rect.topLeft()        + QPoint(0, 0);
                    _p2 = rect.bottomRight()    + QPoint(1, 1);
                    break;
                case ScreenshotWidget::P_BL:
                    _p1 = rect.topRight()       + QPoint(1, 0);
                    _p2 = rect.bottomLeft()     + QPoint(0, 1);
                    break;
                case ScreenshotWidget::P_TR:
                    _p1 = rect.bottomLeft()     + QPoint(0, 1);
                    _p2 = rect.topRight()       + QPoint(1, 0);
                    break;
                case ScreenshotWidget::P_TL:
                    _p1 = rect.bottomRight()    + QPoint(1, 1);
                    _p2 = rect.topLeft()        + QPoint(0, 0);
                    break;
                case ScreenshotWidget::P_MR:
                    _p1 = rect.bottomLeft()     + QPoint(0, 1);
                    _p2 = rect.topRight()       + QPoint(1, 0);
                    break;
                case ScreenshotWidget::P_ML:
                    _p1 = rect.bottomRight()    + QPoint(1, 1);
                    _p2 = rect.topLeft()        + QPoint(0, 0);
                    break;
                case ScreenshotWidget::P_MT:
                    _p1 = rect.bottomLeft()     + QPoint(0, 1);
                    _p2 = rect.topRight()       + QPoint(1, 0);
                    break;
                case ScreenshotWidget::P_MB:
                    _p1 = rect.topLeft()        + QPoint(0, 0);
                    _p2 = rect.bottomRight()    + QPoint(1, 1);
                    break;
                case ScreenshotWidget::P_CTR:
                    _p1 = rect.topLeft()        + QPoint(0, 0);
                    _p2 = rect.bottomRight()    + QPoint(1, 1);
                    break;
                default:
                    break;
                }
                _dragStartP1 = _p1;
                _dragStartP2 = _p2;
                _dragStartMouseX = _mouseX;
                _dragStartMouseY = _mouseY;              
            }
            //没按到拖动点切不在选择框内时重新选择
            else if(!_IsMouseOnSelect(Def::draggingPointSize / 2))
            {
                _box->hide();
                _selected = false;
                _p2 = _p1 = QPoint(_mouseX, _mouseY);
            }
        }
        //未选择时开始选择
        else
        {
           _p2 = _p1 = QPoint(_mouseX, _mouseY);
        }
    }
    if (e->button() == Qt::MouseButton::RightButton)
    {
        //取消选择
        _selected = false;
    }
    update();
}

void ScreenshotWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::MouseButton::LeftButton)
    {
        _dragging = false;
        //已选择时结束调整大小
        if (_selected)
        {
            //结束拖动调整
            if (_draggingPoint)
            {
                _draggingPoint = P_NONE; 
                _UpdateBoxPos();
                _box->show();
            }
        }
        //未选择时结束选择
        else
        {
            _Normalize();
            _selected = true;
            _UpdateBoxPos();
            _box->show();
        }
    }
    update();
}

void ScreenshotWidget::keyPressEvent(QKeyEvent* e)
{
    if (!e->modifiers() && e->key() == Qt::Key::Key_Escape)
    {
        _Reject();
    }
    if (!e->modifiers() &&
        (e->key() == Qt::Key::Key_Enter || e->key() == Qt::Key::Key_Space || e->key() == Qt::Key::Key_Return))
    {
        _Accpet();
    }
    if (e->modifiers() == Qt::Modifier::CTRL && e->key() == Qt::Key::Key_A)
    {
        _selected = true;
        _draggingPoint = P_NONE;
        _dragging = false;
        _p1 = QPoint(0, 0);
        _p2 = QPoint(_width, _height);
        _UpdateBoxPos();
        _box->show();
    }
    if (e->modifiers() == Qt::Modifier::CTRL && e->key() == Qt::Key::Key_X)
    {
        _Reselect();
    }
    update();
}

QRect ScreenshotWidget::_GetSelect() const
{
    int w = std::abs(_p1.x() - _p2.x());
    int h = std::abs(_p1.y() - _p2.y());
    int x = std::min(_p1.x(), _p2.x());
    int y = std::min(_p1.y(), _p2.y());
    return QRect(x, y, w, h);
}

void ScreenshotWidget::_Normalize()
{
    QRect rect = _GetSelect();
    _p1 = QPoint(rect.x(), rect.y());
    _p2 = QPoint(rect.x() + rect.width(), rect.y() + rect.height());
}

bool ScreenshotWidget::_IsMouseOnSelect(int expand) const
{
    auto rect = _GetSelect();
    return _mouseX >= rect.x() - expand && _mouseX <= rect.x() + rect.width() + expand
        && _mouseY >= rect.y() - expand && _mouseY <= rect.y() + rect.height() + expand;
}

QPoint ScreenshotWidget::_GetDragPoint(int id) const
{
    QRect rect = this->_GetSelect();
    switch (id)
    {
    case ScreenshotWidget::P_BR:
        return QPoint(rect.x() + rect.width(), rect.y() + rect.height());
    case ScreenshotWidget::P_BL:
        return QPoint(rect.x(), rect.y() + rect.height());
    case ScreenshotWidget::P_TR:
        return QPoint(rect.x() + rect.width(), rect.y());
    case ScreenshotWidget::P_TL:
        return QPoint(rect.x(), rect.y());
    case ScreenshotWidget::P_MR:
        return QPoint(rect.x() + rect.width(), rect.y() + rect.height() / 2);
    case ScreenshotWidget::P_ML:
        return QPoint(rect.x(), rect.y() + rect.height() / 2);
    case ScreenshotWidget::P_MT:
        return QPoint(rect.x() + rect.width() / 2, rect.y());
    case ScreenshotWidget::P_MB:
        return QPoint(rect.x() + rect.width() / 2, rect.y() + rect.height());
    case ScreenshotWidget::P_CTR:
        return rect.center();
    default:
        break;
    }
    return QPoint(0, 0);
}

int ScreenshotWidget::_GetMouseOnDragPoint() const
{
    for (int id = 1; id < DRAGPOINT_COUNT; id++)
    {
        QPoint pos = _GetDragPoint(id);
        int s = Def::draggingPointSize / 2;
        if (_mouseX >= pos.x() - s && _mouseX <= pos.x() + s
         && _mouseY >= pos.y() - s && _mouseY <= pos.y() + s)
        {
            return id;
        }
   }
   return P_NONE;
}

void ScreenshotWidget::_Accpet()
{
    EndEdit(true);
}

void ScreenshotWidget::_Reject()
{
    EndEdit(false);
}

void ScreenshotWidget::_Reselect()
{
    _selected = false;
    _dragging = false;
    _draggingPoint = P_NONE;
    _box->hide();
    this->update();
}

void ScreenshotWidget::_UpdateBoxPos()
{
    QPoint pos(0, 0);
    QRect rect = _GetSelect();
    //右下角外部
    if (rect.bottomRight().y() + 2 + boxSize.height() <= _height)
    {
        pos.setY(rect.bottomRight().y() + 2);
        //右侧朝内
        if (rect.bottomRight().x() - boxSize.width() >= 0)
            pos.setX(rect.bottomRight().x() - boxSize.width());
        //左侧朝内
        else if(rect.bottomLeft().x() + boxSize.width() <= _width)
            pos.setX(rect.bottomLeft().x());
        //右侧朝外
        else if (rect.bottomRight().x() - boxSize.width() <= _width)
            pos.setX(rect.bottomRight().x());
        //左侧朝外
        else
            pos.setX(rect.bottomLeft().x() - boxSize.width());
    }
    //右上角外部
    else if (rect.topRight().y() - 2 - boxSize.height() >= 0)
    {
        pos.setY(rect.topRight().y() - 2 - boxSize.height());
        //右侧朝内
        if (rect.bottomRight().x() - boxSize.width() >= 0)
            pos.setX(rect.bottomRight().x() - boxSize.width());
        //左侧朝内
        else if (rect.bottomLeft().x() + boxSize.width() <= _width)
            pos.setX(rect.bottomLeft().x());
        //右侧朝外
        else if (rect.bottomRight().x() - boxSize.width() <= _width)
            pos.setX(rect.bottomRight().x());
        //左侧朝外
        else
            pos.setX(rect.bottomLeft().x() - boxSize.width());
    }
    //右下角内部
    else
    {
        pos.setY(rect.bottomRight().y() - boxSize.height());
        //右侧朝内
        if (rect.bottomRight().x() - boxSize.width() >= 0)
            pos.setX(rect.bottomRight().x() - boxSize.width());
        //左侧朝内
        else if (rect.bottomLeft().x() + boxSize.width() <= _width)
            pos.setX(rect.bottomLeft().x());
        //右侧朝外
        else if (rect.bottomRight().x() - boxSize.width() <= _width)
            pos.setX(rect.bottomRight().x());
        //左侧朝外
        else
            pos.setX(rect.bottomLeft().x() - boxSize.width());
    }
    _box->setGeometry(pos.x(), pos.y(), _box->width(), _box->height());
}


FileSelector::FileSelector() : QObject(nullptr)
{
    _file = new QFileDialog;
    _file->setWindowTitle("选择图片");
    _file->setNameFilter("Images(*.png)");
    _file->setViewMode(QFileDialog::Detail);
}

FileSelector::~FileSelector()
{
    _file->deleteLater();
}

void FileSelector::Select()
{
    _file->setDirectory(Settings::screenshotPath.c_str());
    emit Selected(_file->getOpenFileName());
}



ScreenGrabber::ScreenGrabber()
    : QObject(nullptr)
{
    _thread = new QThread;

    _ui = new ScreenshotWidget;

    _selector = new FileSelector;

    connect(this, &ScreenGrabber::EditScreenshot, _ui, &ScreenshotWidget::StartEdit);
    connect(_ui, &ScreenshotWidget::Edited, this, &ScreenGrabber::OnEdited);
    connect(this, &ScreenGrabber::SelectFile, _selector, &FileSelector::Select);
    connect(_selector, &FileSelector::Selected, this, &ScreenGrabber::OnFileSelected);

    this->moveToThread(_thread);
    _thread->start();
}

ScreenGrabber::~ScreenGrabber()
{
    _ui->deleteLater();
    _selector->deleteLater();
    _thread->exit();
    _thread->deleteLater();
}

void ScreenGrabber::EditExistedFile()
{
    _editing = true;
    emit SelectFile();   
}

void ScreenGrabber::Grab(bool fast)
{
    _editing = true;
    Logger::MakeDir(Settings::screenshotPath);
    
    if (fast)
    {
        QApplication::primaryScreen()->grabWindow(0)
            .save((std::string(Settings::screenshotPath) + "/screenshot_" + Logger::GetTimeString() + ".png").c_str());
        emit Grabbed();
        emit Edited(true);
        _editing = false;
    }
    else
    {
        emit EditScreenshot(QApplication::primaryScreen()->grabWindow(0));
        emit Grabbed();
    }
}

void ScreenGrabber::OnEdited(QPixmap* image)
{
    if(image)
        image->save((std::string(Settings::screenshotPath) + "/screenshot_" + Logger::GetTimeString() + ".png").c_str());
    emit Edited(image);
    _editing = false;
}

bool ScreenGrabber::IsEditing() const
{
    return _editing;
}

void ScreenGrabber::OnFileSelected(QString path)
{
    if (path.isEmpty())
    {
        OnEdited(nullptr);
        return;
    }
    QPixmap image(path, "png");
    emit EditStart();
    emit EditScreenshot(image);
}
