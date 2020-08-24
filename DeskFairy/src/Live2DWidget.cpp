#include "Live2DWidget.h"

#include <Math/CubismMatrix44.hpp>

#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QThread>
#include <QMessageBox>
#include <QDesktopServices>
#include <QContextMenuEvent>
#include <QTimer>
#include <QMenu>
#include <qhotkey.h>

#include <cmath>

#include "Defines.h"
#include "Logger.h"
#include "GLFuncs.h"
#include "TransparentWindow.h"
#include "Model.h"
#include "Allocator.h"
#include "MouseTracker.h"
#include "ScreenGrabber.h"
#include "Settings.h"
#include "ItemManager.h"
#include "ThLauncher.h"


//供Cubism使用的日志输出函数
static void CubismLogFuncion(const Csm::csmChar* s)
{
    Logger::Print("%s", s);
}



Live2DWidget::Live2DWidget(std::string modelName)
    : QOpenGLWidget(Q_NULLPTR)
{
    //初始化 窗口属性
    TransparentWindow::Setup(this, Settings::expandToStateBar);
    this->setAttribute(Qt::WA_QuitOnClose);
    _screenWidth = this->width();
    _screenHeight = this->height();
    _modelSize = Settings::size;
    this->resizeGL(_screenWidth, _screenHeight);
    _MoveModelTo(_screenWidth * 0.5, _screenHeight * 0.5);
    this->show();
  
    //初始化 更新器
    _frameTime = 1000 / Settings::mainFps;
    _timer = new QTimer(this);
    _timer->start(_frameTime);
    connect(_timer, &QTimer::timeout, this, &Live2DWidget::_Update);

    //初始化 各组件及信号槽
    _mouseTracker = new MouseTracker(this);
    connect(_mouseTracker, &MouseTracker::mouseMoveSignal, this, &Live2DWidget::MouseMoveEvent);

    _screenGrabber = new ScreenGrabber();
    connect(this, &Live2DWidget::TakeScreenshotSignal, _screenGrabber, &ScreenGrabber::Grab);
    connect(this, &Live2DWidget::EditScreenshotSignal, _screenGrabber, &ScreenGrabber::EditExistedFile);
    connect(_screenGrabber, &ScreenGrabber::Grabbed, this, &Live2DWidget::_OnScreenshotGrabbed);
    connect(_screenGrabber, &ScreenGrabber::Edited, this, &Live2DWidget::_OnScreenshotEditEnded);
    connect(_screenGrabber, &ScreenGrabber::EditStart, this, &Live2DWidget::_OnScreenshotEditStarted);

    _itemManager = new ItemManager(_screenWidth, _screenHeight, &_itemX, &_itemY, &_itemSize, _mouseTracker);
    connect(_itemManager, &ItemManager::GiveItem, this, &Live2DWidget::_ChangeItem);
    connect(this, &Live2DWidget::AddItemSignal,     _itemManager, &ItemManager::AddItem);
    connect(this, &Live2DWidget::DragItemSignal,    _itemManager, &ItemManager::StartDraggingItemFromLive2D);
    connect(this, &Live2DWidget::EndDragItemSignal, _itemManager, &ItemManager::EndDragItemFromLive2D);

    //初始化 菜单
    _menu = new QMenu(this);

    QMenu* _screenshotMenu = new QMenu(_menu);
    _screenshotMenu->setTitle("截图");

    _actionTakeScreenshot = new QAction(this);
    _actionTakeScreenshot->setText("选区截图");
    connect(_actionTakeScreenshot, &QAction::triggered, this, &Live2DWidget::_OnActionTakeScreenshot);
    _screenshotMenu->addAction(_actionTakeScreenshot);

    _actionTakeFastScreenshot = new QAction(this);
    _actionTakeFastScreenshot->setText("快速截图");
    connect(_actionTakeFastScreenshot, &QAction::triggered, this, &Live2DWidget::_OnActionTakeFastScreenshot);
    _screenshotMenu->addAction(_actionTakeFastScreenshot);

    _actionEditScreenshot = new QAction(this);
    _actionEditScreenshot->setText("裁剪已有截图");
    connect(_actionEditScreenshot, &QAction::triggered, this, &Live2DWidget::_OnActionEditScreenshot);
    _screenshotMenu->addAction(_actionEditScreenshot);

    _actionOpenScreenshotFolder = new QAction(this);
    _actionOpenScreenshotFolder->setText("打开截图文件夹");
    connect(_actionOpenScreenshotFolder, &QAction::triggered, this, &Live2DWidget::_OnActionOpenScreenshotFolder);
    _screenshotMenu->addAction(_actionOpenScreenshotFolder);

    _menu->addMenu(_screenshotMenu);

    QMenu* _actionMenu = new QMenu(_menu);
    _actionMenu->setTitle("操作");

    _actionSitOrFloat = new QAction(this);
    _actionSitOrFloat->setText("坐下");
    connect(_actionSitOrFloat, &QAction::triggered, this, &Live2DWidget::_OnActionSitOrFloat);
    _actionMenu->addAction(_actionSitOrFloat);

    _actionSleepOrWake = new QAction(this);
    _actionSleepOrWake->setText("催眠");
    connect(_actionSleepOrWake, &QAction::triggered, this, &Live2DWidget::_OnActionSleepOrWake);
    _actionMenu->addAction(_actionSleepOrWake);   

    _actionChangeItem = new QAction(this);
    _actionChangeItem->setText("更换物品");
    connect(_actionChangeItem, &QAction::triggered, this, &Live2DWidget::_OnActionChangeItem);
    _actionMenu->addAction(_actionChangeItem);

    _actionClearItem = new QAction(this);
    _actionClearItem->setText("清空物品");
    connect(_actionClearItem, &QAction::triggered, _itemManager, &ItemManager::Clear);
    _actionMenu->addAction(_actionClearItem);
    
    _menu->addMenu(_actionMenu);

    _menu->addSeparator();

    _actionOpenThLauncher = new QAction(this);
    _actionOpenThLauncher->setText("东方启动器...");
    connect(_actionOpenThLauncher, &QAction::triggered, this, &Live2DWidget::_OnActionOpenThLauncher);
    _menu->addAction(_actionOpenThLauncher);

    _menu->addSeparator();

    _actionOpenSettingsMenu = new QAction(this);
    _actionOpenSettingsMenu->setText("设置...");
    connect(_actionOpenSettingsMenu, &QAction::triggered, this, &Live2DWidget::_OnActionOpenSettingsMenu);
    _menu->addAction(_actionOpenSettingsMenu);

    _actionOpenAboutsDialog = new QAction(this);
    _actionOpenAboutsDialog->setText("关于...");
    connect(_actionOpenAboutsDialog, &QAction::triggered, this, &Live2DWidget::_OnActionOpenAboutsDialog);
    _menu->addAction(_actionOpenAboutsDialog);

    _menu->addSeparator();

    _actionClose = new QAction(this);
    _actionClose->setText("退出");
    connect(_actionClose, &QAction::triggered, this, &Live2DWidget::_OnActionClose);
    _menu->addAction(_actionClose);

    _menu->show();
    _menu->hide();

    //初始化 OpenGL (初始化前必须先show出窗口）
    this->makeCurrent();
    GLFuncs()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GLFuncs()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GLFuncs()->glEnable(GL_BLEND);
    GLFuncs()->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //初始化 cubism
    _cubismAllocator = new Allocator();
    _cubismOption = new Csm::CubismFramework::Option();
    _cubismOption->LogFunction = CubismLogFuncion;
    _cubismOption->LoggingLevel = Csm::CubismFramework::Option::LogLevel_Verbose;
    Csm::CubismFramework::StartUp(_cubismAllocator, _cubismOption);
    Csm::CubismFramework::Initialize();

    //初始化 模型
    _LoadModel(modelName);
    _SetupModelSize();

    _ChangeItem(_GetRandomItem(), false);

    //初始化其他
    _SetupHotkeys();

    if (Settings::firstTimeStart)
    {
        _OnActionOpenAboutsDialog();
        Settings::Save();
    }

    this->_Show();
}

Live2DWidget::~Live2DWidget()
{
    if (_thLauncher)
    {
        _thLauncher->close();
    }

    _mouseTracker->deleteLater();
    _screenGrabber->deleteLater();
    _itemManager->deleteLater();
    if (_settingsMenu)
        _settingsMenu->deleteLater();
    _menu->deleteLater();

    _ReleaseModel();

    Csm::CubismFramework::Dispose();
    delete _cubismAllocator;
    delete _cubismOption;

    emit CloseSignal();
}




void Live2DWidget::paintGL()
{
    if (_model && this->isVisible())
    {
        Csm::CubismMatrix44 modelView;
        modelView.Scale(_modelSize / _screenWidth, _modelSize / _screenHeight); 
        modelView.Translate(_modelX / _screenWidth * 2.0 - 1.0, -_modelY / _screenHeight * 2.0 + 1.0);
        _model->Draw(modelView);
    }
}



void Live2DWidget::MouseMoveEvent(int x, int y)
{
    _mouseX = x;
    _mouseY = y;

    TransparentWindow::SetTransparentForMouse(this, !_ModelHitTest(x, y));

    if (!_lBtnPressed)
    {
        if (_isDragging)
        {
            _EndDrag();
        }
        if (_isDraggingItem)
        {
            _EndDragItem();
        }
    }
    else if (!_isDragging && !_isDraggingItem)
    {
        if (_ModelHitTest(x, y, "Item"))
        {
            _StartDragItem();
        }
        else
        {
            _StartDrag();
        }
    }

    if (_isDragging)
    {
        _MoveModelTo(_dragStartModelX + _mouseX - _dragStartMouseX, _dragStartModelY + _mouseY - _dragStartMouseY);
    }
}

void Live2DWidget::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        _lBtnPressed = true;
    }
    if (e->button() == Qt::RightButton)
    {
        _rBtnPressed = true;
    }
}

void Live2DWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        _lBtnPressed = false;
    }
    if (e->button() == Qt::RightButton)
    {
        _rBtnPressed = false;
    }
}

void Live2DWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton && !_ModelHitTest(_mouseX, _mouseY, "Item"))
    {
        _OnActionOpenThLauncher();
        _lBtnDoubleClicked = true;
    }
}

bool Live2DWidget::nativeEvent(const QByteArray& b, void* m, long* r)
{
    if (TransparentWindow::processNativeEvent(this, b, m, r))
        return true;
    return QOpenGLWidget::nativeEvent(b, m, r);
}

void Live2DWidget::contextMenuEvent(QContextMenuEvent* e)
{
    _menu->move(e->globalX(), e->globalY());

    _actionSitOrFloat->setText(_isFloating ? "坐下" : "起身");
    _actionSleepOrWake->setText(_isSleeping ? "唤醒" : "催眠");

    _menu->show();
}



void Live2DWidget::_Update()
{
    if (_model)
    {
        //开始更新：鼠标状态
        double mouseDist = Def::Distance(_modelX, _modelY, _mouseX, _mouseY);      
        if (_mouseX != _lastFrameMouseX || _mouseY != _lastFrameMouseY || _lBtnPressed || _rBtnPressed)
        {   
            //更新鼠标静止状态，鼠标位移、点击都可以使鼠标不静止
            _mouseStillStartTime = _time;
        }
        _lastFrameMouseX = _mouseX;
        _lastFrameMouseY = _mouseY;

        //开始更新：live2d模型的参数数据
        {

            //模型中道具位置（供道具管理器使用）
            {
                _itemX = _modelX - _modelSize * 0.015;
                _itemY = _modelY + _modelSize * 0.1;
                _itemSize = 0.07 * _modelSize;
            }

            //坐下和起身
            {
                //拖动时保持浮动
                if (_isDragging)
                {
                    _Float();
                }

                if (_isFloating)
                {
                    _model->LerpParamValue("SIT", 0.0, _frameTime * Def::sitSpeed);
                }
                else
                {
                    _model->LerpParamValue("SIT", 30.0, _frameTime * Def::sitSpeed);
                }
            }       

            //闪光灯
            {
                if (_isCameraFlashing)
                {
                    _model->SetParamValue("FLASHLIGHT",
                        (1.0 - double(_time - _cameraFlashStartTime) / Def::cameraFlashTime) * 30.0);
                }
            }

            //运动惯性
            {
                double velocityX = (double)_modelX - _modelXLastFrame;
                double velocityY = (double)_modelY - _modelYLastFrame;
                _modelXLastFrame = _modelX;
                _modelYLastFrame = _modelY;
                _model->SetParamValue("VELOCITY_X",   velocityX * Def::dragInertia);
                _model->SetParamValue("VECLOCITY_Y", -velocityY * Def::dragInertia);
            }

            //睡眠
            {
                //触发睡觉
                if (Settings::sleep && !_isSleeping)
                {
                    //触发条件：鼠标远离 或 鼠标长时间不动
                    if (mouseDist > Def::sleepTriggerDist || _time - _mouseStillStartTime > Def::mouseStillTriggerSleepTime)
                    {
                        if (_time - _lastTriggeredSleepTime > Def::sleepTriggerInterval)
                        {                           
                            _Sleep();
                            _lastTriggeredSleepTime = _time;
                        }
                    }
                    //两条件均不满足时重置触发cd（下次开始满足时才开始冷却)
                    else if (mouseDist <= Def::sleepTriggerDist && _time == _mouseStillStartTime)
                    {
                        _lastTriggeredSleepTime = _time;
                    }
                }

                //睡觉时的下落
                if (Settings::fallWhenSleep && _isSleeping && _isFloating)
                {
                    this->_MoveModelBy(0, Def::sleepFallingSpeed / 600.0 * double(Settings::size) * _frameTime * 0.001);
                    //触底坐下
                    if (_modelY >= _screenHeight - _modelYMax)
                        _Sit();
                }
            }

            //微笑
            {
                //鼠标近距离移动触发微笑
                if (!_isDragging && !_isSleeping && _mouseStillStartTime == _time && mouseDist <= Def::smileTriggerDist)
                {
                    if (_time - _lastTriggeredSmileTime > Def::smileTriggerInterval)
                    {
                        _lastTriggeredSmileTime = _time;
                        //随机选择眯眼或不眯眼
                        _StartSmile(qrand() & 1);
                    }
                }
            }

            //更新表情（包括睡眠、微笑)
            {
                //更新眼睛和嘴巴参数
                if (!_isSmiling)
                {
                    if (!_isSleeping)
                    {
                        _eyeState = Def::Lerp(_eyeState, 0.0, _frameTime * Def::expressionChangeSpeed);
                        _mouthState = Def::Lerp(_mouthState, 30.0, _frameTime * Def::expressionChangeSpeed);
                    }
                    else
                    {
                        _eyeState = Def::Lerp(_eyeState, 60.0, _frameTime * Def::expressionChangeSpeed);
                        _mouthState = Def::Lerp(_mouthState, 30.0, _frameTime * Def::expressionChangeSpeed);
                    }
                }
                else
                {
                    if (_isEyeSquinting)
                    {
                        _eyeState = Def::Lerp(_eyeState, -60.0, _frameTime * Def::expressionChangeSpeed);
                        _mouthState = Def::Lerp(_mouthState, -30.0, _frameTime * Def::expressionChangeSpeed);
                    }
                    else
                    {
                        _eyeState = Def::Lerp(_eyeState, 0.0, _frameTime * Def::expressionChangeSpeed);
                        _mouthState = Def::Lerp(_mouthState, -30.0, _frameTime * Def::expressionChangeSpeed);
                    }
                }

                _model->SetParamValue("L_EYE_OPEN_CLOSE", _eyeState);
                _model->SetParamValue("R_EYE_OPEN_CLOSE", _eyeState);
                _model->SetParamValue("MOUTH_OPEN_CLOSE", _mouthState);
            }

            //视线追踪
            {
                //苏醒且睁眼时进行追踪
                if (!_isSleeping && !(_isSmiling && _isEyeSquinting))
                {
                    //鼠标远离触发看向屏幕外
                    if (mouseDist > Def::lookAtOuterTriggerDist)
                    {
                        if (_time - _lastTriggeredLookAtOuterTime > Def::lookAtOuterInterval)
                        {
                            _lastTriggeredLookAtOuterTime = _time;
                            _StartLookAtOuter();
                        }
                    }
                    else
                    {
                        _lastTriggeredLookAtOuterTime = _time;
                    }

                    //更新看向的位置
                    if (_isLookingAtOuter)
                    {
                        //看向屏幕外（模型中心)
                        _lookAtX = Def::Lerp(_lookAtX, _modelX, _frameTime * Def::sightTrackingSpeed);
                        _lookAtY = Def::Lerp(_lookAtY, _modelY, _frameTime * Def::sightTrackingSpeed);
                    }
                    else
                    {
                        //跟踪鼠标
                        _lookAtX = Def::Lerp(_lookAtX, _mouseX, _frameTime * Def::sightTrackingSpeed);
                        _lookAtY = Def::Lerp(_lookAtY, _mouseY, _frameTime * Def::sightTrackingSpeed);
                    }
                }
                else if (_isSleeping)
                {
                    //睡觉时的低头
                    double distX = abs(_lookAtX - _modelX);
                    _lookAtY = Def::Lerp(_lookAtY, _modelY + distX * 2, _frameTime * Def::sightTrackingSpeed * 0.1);
                }

                double dx = _lookAtX - _modelX;
                double dy = _modelY - _lookAtY;

                //更新头倾斜角度
                {
                    double lookAtAngle = 0.0;              
                    if (abs(dx) < 0.0001)
                    {
                        //防止除0异常
                        lookAtAngle = dy > 0 ? 90.0 : -90.0;
                    }
                    else
                    {
                        lookAtAngle = std::atan(-dy / double(dx)) / Def::pi * 180.0;
                    }

                    //从0°到headLeanAngleMaxPoint再到90° 倾斜角度先增后减
                    double leanRate = 0.0;
                    if (abs(lookAtAngle) <= Def::headLeanAngleMaxPoint)
                    {
                        leanRate = abs(lookAtAngle) / Def::headLeanAngleMaxPoint;
                    }
                    else
                    {
                        leanRate = (90.0 - abs(lookAtAngle)) / (90.0 - Def::headLeanAngleMaxPoint);
                    }

                    leanRate *= lookAtAngle > 0 ? 1.0 : -1.0;
                    leanRate *= std::max(std::min((Def::Distance(dx, dy, 0.0, 0.0) - Def::headLeanMinDist)
                        / (Def::headLeanMaxDist - Def::headLeanMinDist), 1.0), 0.0);

                    _headLeanAngle = Def::Lerp(_headLeanAngle, leanRate * Def::headLeanAngleMax,              
                        _frameTime * Def::sightTrackingSpeed);
                    _model->SetParamValue("HEAD_Z", _headLeanAngle * 3.0);
                }

                //头和眼睛跟踪
                {
                    //前后方追踪范围不同
                    if (dx < 0)
                    {
                        dx = dx / (double)Def::sightTrackingRangeHorizotalForward * 30.0;
                    }
                    else
                    {
                        dx = dx / (double)Def::sightTrackingRangeHorizotalBackward * 30.0;
                    }
                    dy = dy / (double)Def::sightTrackingRangeVertical * 30.0;

                    _model->SetParamValue("HEAD_X", dx * 1.0);
                    _model->SetParamValue("HEAD_Y", dy * 1.0);
                    _model->SetParamValue("EYEBALLS_X", dx * 2.0);
                    _model->SetParamValue("EYEBALLS_Y", dy * 2.0);
                }
            }

            //翅膀扇动
            {
                if (_isFloating)
                {
                    double interval = Def::wingsFlapingInterval * (_isSleeping ? 4.0 : 1.0);
                    _model->LerpParamValue("WINGS_FLAP", abs(sin(_time / interval * Def::pi))
                        * 60.0 * (_isDragging ? 0.3 : 1.0), _frameTime * 0.03);
                }
                else
                {
                    _model->LerpParamValue("WINGS_FLAP", 0.0, _frameTime * 0.005);
                }
            }

            //眨眼
            {
                //开始眨眼
                if (!_isSleeping && !(_isSmiling && _isEyeSquinting))
                {
                    if (_time - _lastTriggeredEyeBlinkTime > Def::eyeBlinkInterval)
                    {
                        _lastTriggeredEyeBlinkTime = _time;
                        _StartEyeBlink();
                    }
                }
                else
                {
                    _lastTriggeredEyeBlinkTime = _time;
                }

                //更新眨眼眼睛参数（会覆盖之前的表情眼睛参数)
                if (_isEyeBlinking)
                {
                    double ratio = 1.0 - abs(cos(double(_time - _eyeBlinkStartTime) / Def::eyeBlinkTime * Def::pi));
                    _eyeState = std::min(ratio * 70.0, 60.0);
                    _model->SetParamValue("L_EYE_OPEN_CLOSE", _eyeState);
                    _model->SetParamValue("R_EYE_OPEN_CLOSE", _eyeState);
                }
            }

            //漂浮
            {
                if (_isFloating && !_isDragging)
                {
                    double lastY = std::sin(double(_time - _frameTime) / Def::floatingTime * Def::pi * 2.0)
                        * Def::floatingRange * Settings::size;
                    double nowY = std::sin(double(_time) / Def::floatingTime * Def::pi * 2.0)
                        * Def::floatingRange * Settings::size;
                    _MoveModelBy(0.0, nowY - lastY);
                }
            }      

            //呼吸
            {
                _model->SetParamValue("BREATH", sin(_time / double(Def::breathTime) * 2.0 * Def::pi) * 15.0 + 15.0);
            }
        }

        _model->Update(_frameTime * 0.001f);
    }

    _UpdatePlan();

    this->update();

    _time += _frameTime;
}



void Live2DWidget::_LoadModel(std::string fileName)
{
    std::string dir = std::string(Def::resourcePath) + "/" + fileName + '/';
    std::string jsonName = fileName + ".model3.json";

    if (_model)
    {
        delete _model;
    }
    _model = new Model();
    _model->LoadAssets(dir.c_str(), jsonName.c_str());
}

void Live2DWidget::_ReleaseModel()
{
    if (_model)
    {
        delete _model;
    }
    _model = nullptr;
}

bool Live2DWidget::_ModelHitTest(int x, int y, std::string hitArea) const
{
    if (!_model)
    {
        return false;
    }

    //获得模型在opengl中的坐标(-1.0~1.0)
    double modelX = (x - _modelX) / _modelSize * 2.0;
    double modelY = (_modelY - y) / _modelSize * 2.0;
    if (modelX < -1.0 || modelY < -1.0 || modelX > 1.0 || modelY > 1.0)
    {
        return false;
    }
    return _model->HitTest(hitArea.c_str(), modelX, modelY);
}

void Live2DWidget::_SetupModelSize()
{
    if (!_model)
        return;
    double halfSize = _modelSize * 0.5;
    _modelXMax = -halfSize, _modelXMin = halfSize;
    _modelYMax = -halfSize, _modelYMin = halfSize;
    for (double x = -halfSize; x < halfSize; x += 1.0)
    {
        for (double y = -halfSize; y < halfSize; y += 1.0)
        {
            if (_ModelHitTest(_modelX + x, _modelY + y))
            {
                _modelXMax = std::max(_modelXMax, x);
                _modelXMin = std::min(_modelXMin, x);
                _modelYMax = std::max(_modelYMax, y);
                _modelYMin = std::min(_modelYMin, y);
            }
        }
    }
}

void Live2DWidget::_MoveModelBy(double dx, double dy)
{
    _MoveModelTo(_modelX + dx, _modelY + dy);
}

void Live2DWidget::_MoveModelTo(double x, double y)
{
    _modelX = x;
    _modelY = y;
    _KeepModelInScreen();
}

void Live2DWidget::_KeepModelInScreen()
{
    if (_modelX < -_modelXMin)
    {
        _modelX = -_modelXMin;
    }
    if (_modelX > _screenWidth - _modelXMax)
    {
        _modelX = _screenWidth - _modelXMax;
    }
    if (_modelY < -_modelYMin)
    {
        _modelY = -_modelYMin;
    }
    if (_modelY > _screenHeight - _modelYMax)
    {
        _modelY = _screenHeight - _modelYMax;
    }
}



void Live2DWidget::_UpdateSettings()
{
    //更新刷新率
    _timer->stop();
    _frameTime = 1000 / Settings::mainFps;
    _timer->start(_frameTime);

    _itemManager->UpdateSettings();

    //更新活动范围
    TransparentWindow::Setup(this, Settings::expandToStateBar);
    _screenHeight = this->height();
    _screenWidth = this->width();

    //更新模型尺寸
    int lastSize = _modelSize;
    _modelSize = Settings::size;
    _modelXMax *= _modelSize / lastSize;
    _modelYMax *= _modelSize / lastSize;
    _modelXMin *= _modelSize / lastSize;
    _modelYMin *= _modelSize / lastSize;

    //更新快捷键
    _SetupHotkeys();
}



void Live2DWidget::_SetupHotkeys()
{
    for (auto hotkey : _hotkeys)
    {
        delete hotkey;
    }
    _hotkeys.clear();

    _hotkeys.push_back(new QHotkey(Settings::screenshotHotKey, true, this));
    connect(_hotkeys.back(), &QHotkey::activated, this, &Live2DWidget::_OnActionTakeScreenshot);

    _hotkeys.push_back(new QHotkey(Settings::screenshotFastHotKey, true, this));
    connect(_hotkeys.back(), &QHotkey::activated, this, &Live2DWidget::_OnActionTakeFastScreenshot);

    _hotkeys.push_back(new QHotkey(Settings::exitHotKey, true, this));
    connect(_hotkeys.back(), &QHotkey::activated, this, &Live2DWidget::_OnActionClose);

    _hotkeys.push_back(new QHotkey(Settings::hideHotKey, true, this));
    connect(_hotkeys.back(), &QHotkey::activated, this, &Live2DWidget::_OnActionHideOrShow);

    _hotkeys.push_back(new QHotkey(Settings::clearItemHotKey, true, this));
    connect(_hotkeys.back(), &QHotkey::activated, _itemManager, &ItemManager::Clear);

    _hotkeys.push_back(new QHotkey(Settings::editScreenshotHotkey, true, this));
    connect(_hotkeys.back(), &QHotkey::activated, this, &Live2DWidget::_OnActionEditScreenshot);

    _hotkeys.push_back(new QHotkey(Settings::openScreenshotHotkey, true, this));
    connect(_hotkeys.back(), &QHotkey::activated, this, &Live2DWidget::_OnActionOpenScreenshotFolder);

    _hotkeys.push_back(new QHotkey(Settings::thLauncherHotkey, true, this));
    connect(_hotkeys.back(), &QHotkey::activated, this, &Live2DWidget::_OnActionOpenThLauncher);
}

void Live2DWidget::_SetHotkeysState(bool enable)
{
    if (_isHotkeyDisabled == !enable)
    {
        return;
    }

    for (auto hotkey : _hotkeys)
    {
        if (hotkey)
        {
            hotkey->blockSignals(!enable);
        }
    }
    _isHotkeyDisabled = !enable;
}



void Live2DWidget::_StartDrag()
{
    _EndSmile();
    _Wake();
    _isDragging = true;
    _dragStartMouseX = _mouseX;
    _dragStartMouseY = _mouseY;
    _dragStartModelX = _modelX;
    _dragStartModelY = _modelY;
}

void Live2DWidget::_EndDrag()
{
    _isDragging = false;
}

void Live2DWidget::_StartDragItem()
{
    _isDraggingItem = true;
    _ChangeItem(_GetRandomItem(), Settings::allowItemSpawn);
    emit DragItemSignal();
}

void Live2DWidget::_EndDragItem()
{
    emit EndDragItemSignal();
    _isDraggingItem = false;
}

void Live2DWidget::_ChangeItem(int item, bool dropItem)
{
    if (_model)
    {
        _model->SetParamValue("ITEM_P", (item == ITEM_P) * 30.0);
        _model->SetParamValue("ITEM_B", (item == ITEM_B) * 30.0);
        _model->SetParamValue("ITEM_1UP", (item == ITEM_1UP) * 30.0);
        _model->SetParamValue("ITEM_POINT", (item == ITEM_POINT) * 30.0);
        _model->SetParamValue("ITEM_CAMERA", (item == ITEM_CAMERA) * 30.0);

        if (dropItem && _item != ITEM_CAMERA)
        {
            emit AddItemSignal(_item);
        }

        _item = item;
    }
}

int Live2DWidget::_GetRandomItem() const
{
    int i = 0;
    if (_item == ITEM_CAMERA)
        i = (qrand() % (ITEMTYPE_COUNT - 1)) + 1;
    else
    {
        i = (qrand() % (ITEMTYPE_COUNT - 2)) + 1;
        if (_item <= i)
            i++;
    }
    return i;
}

void Live2DWidget::_Wake()
{
    _isSleeping = false;
}

void Live2DWidget::_Sleep()
{
    _EndSmile();
    _EndEyeBlink();

    _isSleeping = true;
    //设置醒来的时间
    _AddPlan(&Live2DWidget::_Wake, qrand() % (Def::sleepTimeMax - Def::sleepTimeMin) + Def::sleepTimeMin);
}

void Live2DWidget::_Sit()
{
    _isFloating = false;
}

void Live2DWidget::_Float()
{
    _isFloating = true;
}

void Live2DWidget::_StartSmile(bool withEyeSquinted)
{
    if (_isSleeping)
    {
        return;
    }

    _isEyeSquinting = withEyeSquinted;
    if (withEyeSquinted)
    {
        _EndEyeBlink();
    }
    _isSmiling = true;
    //设置结束微笑时间
    _AddPlan(&Live2DWidget::_EndSmile, withEyeSquinted ? Def::bigSmileTime : Def::smileTime);
}

void Live2DWidget::_EndSmile()
{
    _isSmiling = false;
}

void Live2DWidget::_StartEyeBlink()
{
    if (_isSleeping || (_isSmiling && _isEyeSquinting))
    {
        return;
    }
    _eyeBlinkStartTime = _time;
    _isEyeBlinking = true;
    _AddPlan(&Live2DWidget::_EndEyeBlink, Def::eyeBlinkTime);
}

void Live2DWidget::_EndEyeBlink()
{
    _isEyeBlinking = false;
}

void Live2DWidget::_StartLookAtOuter()
{
    _isLookingAtOuter = true;
    _AddPlan(&Live2DWidget::_EndLookAtOuter, Def::lookAtOuterTime);
}

void Live2DWidget::_EndLookAtOuter()
{
    _isLookingAtOuter = false;
}

void Live2DWidget::_StartCameraFlash()
{
    if (_item != ITEM_CAMERA)
    {
        return;
    }

    _isCameraFlashing = true;
    _cameraFlashStartTime = _time;
    _AddPlan(&Live2DWidget::_EndCameraFlash, Def::cameraFlashTime);
}

void Live2DWidget::_EndCameraFlash()
{
    _isCameraFlashing = false;
}

void Live2DWidget::_Hide(bool withHotkeyDisabled)
{
    _itemManager->hide();
    this->hide();
    if (withHotkeyDisabled)
    {
        _SetHotkeysState(false);
    }
    _timer->stop();
}

void Live2DWidget::_Show()
{
    TransparentWindow::Setup(this, Settings::expandToStateBar);
    _timer->start(_frameTime);
    this->show();
    _itemManager->show();
    _SetHotkeysState(true);
}



void Live2DWidget::_OnScreenshotGrabbed()
{
    //如果未在截图时隐藏，那就在此时（开始编辑）隐藏
    if (!Settings::hideWhenScreenshot)
    {
        _StartCameraFlash();
        _Hide(true);
    }
    _Wake();
}

void Live2DWidget::_OnScreenshotEditStarted()
{
    _Hide(true);
}

void Live2DWidget::_OnScreenshotEditEnded(bool accept)
{
    //如果之前用快捷键隐藏，那就保持隐藏
    if (!_isHidingByHotkey)
    {
        _Show();
    }
    _SetHotkeysState(true);
    _Wake();
    if (accept)
    {
        _StartCameraFlash();
    }
}

void Live2DWidget::_OnThLauncherLaunched()
{
    _thLauncher->deleteLater();
    _thLauncher = nullptr;
}



void Live2DWidget::_OnActionClose()
{
    this->close();
    this->deleteLater();
}

void Live2DWidget::_OnActionTakeFastScreenshot()
{
    if (_screenGrabber->IsEditing())
        return;

    _Wake();
    _ChangeItem(ITEM_CAMERA, false);
    if (Settings::hideWhenScreenshot)
    {
        _Hide(true);
    }
    emit TakeScreenshotSignal(true);
}

void Live2DWidget::_OnActionTakeScreenshot()
{
    if (_screenGrabber->IsEditing())
    {
        return;
    }

    _Wake();
    _ChangeItem(ITEM_CAMERA, false);
    if (Settings::hideWhenScreenshot)
    {
        _Hide(true);
    }
    emit TakeScreenshotSignal(false);
}

void Live2DWidget::_OnActionEditScreenshot()
{
    if (_screenGrabber->IsEditing())
    {
        return;
    }
    emit EditScreenshotSignal();
}

void Live2DWidget::_OnActionOpenScreenshotFolder()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(Settings::screenshotPath.c_str()));
}

void Live2DWidget::_OnActionSitOrFloat()
{
    if (!_isFloating)
    {
        _Float();
    }
    else
    {
        _Sit();
    }
}

void Live2DWidget::_OnActionOpenSettingsMenu()
{
    _Hide(true);

    _settingsMenu = new SettingsMenu();  
    if (_settingsMenu->exec())
    {
        _UpdateSettings();
        repaint();
    }
    delete _settingsMenu;
    _settingsMenu = nullptr;
    
    _Show();
}

void Live2DWidget::_OnActionHideOrShow()
{
    if (!_isHidingByHotkey)
    {
        _Hide(false);
        _isHidingByHotkey = true;
    }
    else
    {
        _Show();
        _isHidingByHotkey = false;
    }
}

void Live2DWidget::_OnActionOpenAboutsDialog()
{
    //逐行读入readme
    std::string text, tmp;
    std::ifstream in("../readme.txt");
    while (in)
    {
        std::getline(in, tmp);
        text += tmp + "\n";
    }

    if (text.empty())
    {
        return;
    }

    _Hide(true);

    QMessageBox* box = new QMessageBox;
    box->setContentsMargins(5, 5, 5, 5);
    box->setWindowTitle("DeskFairy->关于");
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->setText(text.c_str());
    box->exec();

    _Show();
}

void Live2DWidget::_OnActionSleepOrWake()
{
    if (_isSleeping)
    {
        _Wake();
    }
    else
    {
        _Sleep();
    }
}

void Live2DWidget::_OnActionChangeItem()
{
    _ChangeItem(_GetRandomItem(), Settings::allowItemSpawn);
}

void Live2DWidget::_OnActionOpenThLauncher()
{
    if (!_thLauncher)
    {
        _thLauncher = new ThLauncher;
        connect(_thLauncher, &ThLauncher::LaunchedSignal, this, &Live2DWidget::_OnThLauncherLaunched);
    }
}



void Live2DWidget::_AddPlan(PlanFunction func, int delay)
{
    Plan plan;
    plan.function = func;
    plan.executeTime = _time + delay;
    _plans.push(plan);
}

void Live2DWidget::_UpdatePlan()
{
    while (_plans.size() && _time > _plans.top().executeTime)
    {
        (this->*_plans.top().function)();
        _plans.pop();
    }
}
