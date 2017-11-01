#pragma once

#ifndef __XHEADER_H__
#define __XHEADER_H__

#include <stdio.h>
#include <stdlib.h>
#include <type_traits>
#include <functional>

#include "defines.h"
#include "logger.h"
#include "timer.h"
#include "auto_handle.h"
#include "debuger.h"
#include "exception.h"
#include "serialization.h"
#include "xbuffer.h"
#include "xsystem.h"
#include "xutility.h"
#include "destructor.h"

#include "json/json.h"

using namespace xgc;
using namespace xgc::common;

using namespace std::placeholders;

// 基本数据类型，及辅助计算库
#include "XCoreDefine.h"

#include "XMath.h"
#include "XVector2.h"
#include "XVector3.h"
#include "XGeometry.h"

// 服务器对象基础类
#include "XClassInfo.h"
#include "XObject.h"
#include "XObjectNode.h"

namespace xgc
{
	///
	/// [10/31/2012 Albert.xu]
	/// 获取定时器
	///
	extern CORE_API timer& getTimer();

	///
	/// [10/31/2012 Albert.xu]
	/// 获取计时器
	///
	extern CORE_API timer& getClock();
}
#endif // __XHEADER_H__