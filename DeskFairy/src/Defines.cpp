#include "Defines.h"

namespace Def
{
	const double pi = 3.1415926536;

	const DebugLogMode debugLogMode = DEBUG_TO_QT;
	const char* debugLogFilePath = "../logs";

	const char* resourcePath = "../resources";

	const int mouseTrackTimePerFrame = 60;

	const int settingsMenuFontSize = 13;
	const int settingsMenuLabelWidth = 150;
	const int settingsMenuWidgetWidth = 200;
	const int settingsMenuItemHeight = 25;
	const char* settingsSavePath = "../profiles";

	const int draggingPointSize = 10;
	const int editButtonSize = 40;

	const char* thLauncherSavePath = "../profiles";

	const double sightTrackingSpeed = 0.005;
	const int sightTrackingRangeHorizotalForward = 700;
	const int sightTrackingRangeHorizotalBackward = 500;
	const int sightTrackingRangeVertical = 500;
	const double headLeanAngleMax = 5.0;
	const double headLeanAngleMaxPoint = 60.0;
	const double headLeanMinDist = 50.0;
	const double headLeanMaxDist = 200.0;
	const double dragInertia = 20.0;
	const int wingsFlapingInterval = 314;
	const int eyeBlinkInterval = 4000;
	const int eyeBlinkTime = 200;
	const int floatingTime = 4000;
	const double floatingRange = 0.017;
	const int breathTime = 2000;
	const double sitSpeed = 0.005;
	const double expressionChangeSpeed = 0.02;
	const int smileTime = 3000;
	const int bigSmileTime = 1500;
	const double smileTriggerDist = 100;
	const int smileTriggerInterval = 6000;
	const double sleepFallingSpeed = 10.0;
	const int mouseStillTriggerSleepTime = 30000;
	const int sleepTriggerDist = 600;
	const int sleepTriggerInterval = 150000;
	const int sleepTimeMin = 60000;
	const int sleepTimeMax = 120000;
	const int lookAtOuterInterval = 10000;
	const int lookAtOuterTime = 3000;
	const double lookAtOuterTriggerDist = 400;
	const int cameraFlashTime = 200;

	const double itemWorldToScreenScale = 100.0;
	const double itemFriction = 0.3;
	const double itemDensity = 1.0;
	const double itemRestitution = 0.4;
}