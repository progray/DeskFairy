#include "GLFuncs.h"

namespace
{
	QOpenGLFunctions* funcs = nullptr;
}

QOpenGLFunctions* const GLFuncs()
{
	if (!funcs)
	{
		funcs = QOpenGLContext::currentContext()->functions();
	}
	return funcs;
}