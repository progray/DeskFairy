#pragma once

#include <cmath>

enum ItemType
{
	ITEM_CAMERA,
	ITEM_P,
	ITEM_1UP,
	ITEM_POINT,
	ITEM_B,
	ITEMTYPE_COUNT
};

enum DebugLogMode
{
	DEBUG_NONE,
	DEBUG_TO_STANDARD,
	DEBUG_TO_FILE,
	DEBUG_TO_QT
};

namespace Def
{
	extern const double pi;

	extern const DebugLogMode debugLogMode;		//仅在debug编译下有效
	extern const char* debugLogFilePath;

	extern const char* resourcePath;

	extern const int mouseTrackTimePerFrame;

	extern const int settingsMenuFontSize;
	extern const int settingsMenuLabelWidth;
	extern const int settingsMenuWidgetWidth;
	extern const int settingsMenuItemHeight;
	extern const char* settingsSavePath;

	extern const int draggingPointSize;
	extern const int editButtonSize;

	extern const char* thLauncherSavePath;

	extern const double sightTrackingSpeed;
	extern const int sightTrackingRangeHorizotalForward;
	extern const int sightTrackingRangeHorizotalBackward;
	extern const int sightTrackingRangeVertical;
	extern const double headLeanAngleMax;
	extern const double headLeanAngleMaxPoint;
	extern const double headLeanMinDist;
	extern const double headLeanMaxDist;
	extern const double dragInertia;
	extern const int wingsFlapingInterval;
	extern const int eyeBlinkInterval;
	extern const int eyeBlinkTime;
	extern const int floatingTime;
	extern const double floatingRange;
	extern const int breathTime;
	extern const double sitSpeed;
	extern const double expressionChangeSpeed;
	extern const int smileTime;
	extern const int bigSmileTime;
	extern const int smileTriggerInterval;
	extern const double smileTriggerDist;
	extern const int mouseStillTriggerSleepTime;
	extern const double sleepFallingSpeed;
	extern const int sleepTriggerDist;
	extern const int sleepTriggerInterval;
	extern const int sleepTimeMin;
	extern const int sleepTimeMax;
	extern const int lookAtOuterInterval;
	extern const int lookAtOuterTime;
	extern const double lookAtOuterTriggerDist;
	extern const int cameraFlashTime;

	extern const double itemWorldToScreenScale;
	extern const double itemFriction;
	extern const double itemDensity;
	extern const double itemRestitution;
	
	template <class T>
	inline const T Lerp(const T& v1, const T& v2, double delta)
	{
		return v1 + (v2 - v1) * delta;
	}

	inline const double Distance(double x1, double y1, double x2, double y2)
	{
		return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
	}
}