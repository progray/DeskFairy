#pragma once
#pragma execution_character_set("utf-8")

#include <CubismFramework.hpp>

#include <QOpenGLWidget>

#include <string>
#include <queue>


class QMouseEvent;
class QTimer;
class QHotkey;
class QMenu;
class QAction;

class Allocator;
class ScreenGrabber;
class SettingsMenu;
class MouseTracker;
class ItemManager;
class Model;
class ThLauncher;



//掌管Live2D模型、窗口、其他模块的更新
class Live2DWidget : public QOpenGLWidget
{
    Q_OBJECT

public:

    //通过模型名称初始化
    Live2DWidget(std::string modelName);

    //销毁
    ~Live2DWidget();



    //绘制模型
    void paintGL() override;



    //鼠标移动事件，由鼠标追踪器触发，与qt的mouseMoveEvent不同
    void MouseMoveEvent(int x, int y);

    //鼠标按下事件
    void mousePressEvent(QMouseEvent* e) override;

    //鼠标释放事件
    void mouseReleaseEvent(QMouseEvent* e) override;

    //鼠标双击事件
    void mouseDoubleClickEvent(QMouseEvent* e) override;

    //重写的本地事件，用于阻止鼠标点击使窗口获得焦点
    bool nativeEvent(const QByteArray& b, void* m, long* r) override;

    //右键菜单事件
    void contextMenuEvent(QContextMenuEvent* e) override;

signals:

    //截图信号
    void TakeScreenshotSignal(bool fastMode);

    //编辑已有截图信号
    void EditScreenshotSignal();

    //结束物品拖曳信号
    void EndDragItemSignal();

    //添加物品信号
    void AddItemSignal(int type);

    //拖曳物品信号
    void DragItemSignal();

    //退出信号
    void CloseSignal(int exitCode = 0);

private:

    //定时器和时间
    QTimer* _timer = nullptr;
    int _frameTime = 0;
    int _time = 0;

    

    //屏幕大小
    int _screenWidth = 0;
    int _screenHeight = 0;



    //cubism初始化相关
    Allocator* _cubismAllocator = nullptr;
    Csm::CubismFramework::Option* _cubismOption = nullptr;

    //东方启动器
    ThLauncher* _thLauncher = nullptr;

    //屏幕截图器
    ScreenGrabber* _screenGrabber   = nullptr;
    
    //物品管理器
    ItemManager* _itemManager = nullptr;

    //鼠标输入处理
    MouseTracker* _mouseTracker     = nullptr;

    double _mouseX = 0;
    double _mouseY = 0;
    bool _lBtnPressed = false;
    bool _rBtnPressed = false;
    bool _lBtnDoubleClicked = false;



    //更新数据，会在每帧调用一次
    void _Update();



    //模型 
    Model* _model = nullptr;

    double _modelSize = 0.0;
    double _modelX = 0.0;
    double _modelY = 0.0;
    double _modelXLastFrame = 0.0;
    double _modelYLastFrame = 0.0;
    double _modelXMax = 0.0;
    double _modelXMin = 0.0;
    double _modelYMax = 0.0;
    double _modelYMin = 0.0;
    double _modelVelocityX = 0.0;
    double _modelVelocityY = 0.0;

    //通过模型路径名加载模型
    void _LoadModel(std::string fileName);
    
    //初始化模型的大小（用于和屏幕边缘的碰撞检测）
    void _SetupModelSize();

    //释放模型
    void _ReleaseModel();

    //将模型移动一定距离
    void _MoveModelBy(double dx, double dy);

    //将模型移动到指定点
    void _MoveModelTo(double x, double y);

    //保持模型在屏幕内
    void _KeepModelInScreen();

    //对模型某个碰撞区域的检测
    bool _ModelHitTest(int x, int y, std::string hitArea = "Body") const;

    

    //设置菜单
    SettingsMenu* _settingsMenu = nullptr;

    //更新设置信息
    void _UpdateSettings();



    //热键
    std::vector<QHotkey*> _hotkeys;
    
    //初始化热键
    void _SetupHotkeys();

    //设置热键开启或关闭
    void _SetHotkeysState(bool enable);



    //计划函数的别名
    using PlanFunction = void (Live2DWidget::*)();

    //延迟一定时间进行的计划类型
    struct Plan
    {
        PlanFunction function;
        int executeTime;
        //比较大小方式相反，排序时按执行时间从小到大排序
        bool operator<(const Plan& other)const
        {
            return executeTime > other.executeTime;
        }
    };

    std::priority_queue<Plan> _plans;

    //添加计划
    void _AddPlan(PlanFunction func, int delay);

    //更新计划列表
    void _UpdatePlan();



    //各种动作及它们的状态
    bool _isDragging        = false;
    bool _isDraggingItem    = false;
    bool _isFloating        = true;
    bool _isSleeping        = false;
    bool _isSmiling         = false;
    bool _isEyeSquinting    = false;
    bool _isEyeBlinking     = false;
    bool _isHotkeyDisabled  = false;
    bool _isHidingByHotkey  = false;
    bool _isLookingAtOuter  = false;
    bool _isCameraFlashing  = false;

    int _item = 0;
    double _itemX = 0.0;
    double _itemY = 0.0;
    double _itemSize = 0.0;

    double _dragStartMouseX = 0.0;
    double _dragStartMouseY = 0.0;
    double _dragStartModelX = 0.0;
    double _dragStartModelY = 0.0;

    double _lookAtX         = 0.0;
    double _lookAtY         = 0.0;
    double _headLeanAngle   = 0.0;
    double _mouthState      = 0.0;
    double _eyeState        = 0.0;

    double _lastFrameMouseX = 0.0;  
    double _lastFrameMouseY = 0.0;
    int _lastTriggeredSleepTime         = 0;
    int _lastTriggeredSmileTime         = 0;
    int _lastTriggeredEyeBlinkTime      = 0;
    int _lastTriggeredLookAtOuterTime   = 0;

    int _eyeBlinkStartTime      = 0;
    int _cameraFlashStartTime   = 0;
    int _mouseStillStartTime    = 0;  

    //开始拖动
    void _StartDrag();

    //结束拖动
    void _EndDrag();

    //开始拖动物品
    void _StartDragItem();

    //结束拖动物品
    void _EndDragItem();

    //更换物品
    void _ChangeItem(int item, bool dropItem);

    //获取一项随机物品（不会获取相机或当前持有的物品)
    int _GetRandomItem() const;

    //唤醒
    void _Wake();

    //催眠
    void _Sleep();

    //坐下
    void _Sit();

    //漂浮
    void _Float();

    //开始不眯眼或眯眼的微笑
    void _StartSmile(bool withEyeSquinted);

    //结束微笑
    void _EndSmile();

    //眨眼
    void _StartEyeBlink();

    //结束眨眼
    void _EndEyeBlink();

    //看向屏幕外
    void _StartLookAtOuter();

    //重新追踪鼠标
    void _EndLookAtOuter();

    //相机闪烁
    void _StartCameraFlash();

    //结束相机闪烁
    void _EndCameraFlash();

    //禁用或不禁用热键的隐藏画面
    void _Hide(bool withHotkeyDisabled);

    //显示画面，保证热键打开
    void _Show();


    //屏幕截图完成回调
    void _OnScreenshotGrabbed();

    //屏幕截图开始编辑回调
    void _OnScreenshotEditStarted();

    //屏幕截图结束编辑回调
    void _OnScreenshotEditEnded(bool accept);

    //启动器启动回调
    void _OnThLauncherLaunched();


    //菜单及其事件
    QMenu* _menu = nullptr;
    QAction* _actionClose                   = nullptr;
    QAction* _actionSitOrFloat              = nullptr;
    QAction* _actionSleepOrWake             = nullptr;
    QAction* _actionTakeScreenshot          = nullptr;
    QAction* _actionTakeFastScreenshot      = nullptr;
    QAction* _actionEditScreenshot          = nullptr;
    QAction* _actionOpenScreenshotFolder    = nullptr;
    QAction* _actionOpenSettingsMenu        = nullptr;
    QAction* _actionChangeItem              = nullptr;
    QAction* _actionClearItem               = nullptr;
    QAction* _actionOpenAboutsDialog        = nullptr;
    QAction* _actionHideOrShow              = nullptr;
    QAction* _actionOpenThLauncher          = nullptr;

    //菜单:关闭动作
    void _OnActionClose();

    //菜单:截图动作
    void _OnActionTakeScreenshot();

    //菜单:快速截图动作
    void _OnActionTakeFastScreenshot();

    //菜单:打开截图文件夹动作
    void _OnActionOpenScreenshotFolder();

    //菜单:编辑截图动作
    void _OnActionEditScreenshot();

    //菜单:坐下或起身动作
    void _OnActionSitOrFloat();

    //菜单:催眠或唤醒动作
    void _OnActionSleepOrWake();

    //菜单:打开设置菜单动作
    void _OnActionOpenSettingsMenu();

    //菜单:显示或隐藏动作
    void _OnActionHideOrShow();

    //菜单:打开关于界面动作
    void _OnActionOpenAboutsDialog();

    //菜单:切换物品动作
    void _OnActionChangeItem();

    //菜单:打开东方启动器
    void _OnActionOpenThLauncher();
};
