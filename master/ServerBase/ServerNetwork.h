#pragma once

#include "ServerDefines.h"

///
/// [12/30/2013 albert.xu]
/// ��ʼ������
///
xgc_bool InitializeNetwork( IniFile &ini );

///
/// [2/17/2014 albert.xu]
/// ������������
///
xgc_bool ProcessNetwork();

///
/// [12/30/2013 albert.xu]
/// ��ֹ����
///
xgc_void FinializeNetwork();

///
/// ��ʼ��Monitor����
/// [12/30/2014] create by jianglei.kinly
///
xgc_bool InitializeMTNetwork( IniFile& ini );