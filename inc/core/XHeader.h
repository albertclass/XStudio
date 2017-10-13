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
// �����������ͣ������������
#include "XCoreDefine.h"

#include "XMath.h"
#include "XVector2.h"
#include "XVector3.h"
#include "XGeometry.h"

// ���������������
#include "XClassInfo.h"
#include "XObject.h"

///
/// [10/31/2012 Albert.xu]
/// ��ȡ��ʱ��
///
extern timer& getTimer();

///
/// [10/31/2012 Albert.xu]
/// ��ȡ��ʱ��
///
extern timer& getClock();

#endif // __XHEADER_H__