#pragma once
#ifndef _TABLE_MAKER_H_
#define _TABLE_MAKER_H_
#include "Database.h"

///
/// \brief ���������ļ��������ݿ��
///
/// \param conn ���ݿ����Ӷ���
/// \param fconfig ���ݿ������ļ�·��
/// \param environment �������� ��ʽΪ key=val;key=val;...key=val
/// \author albert.xu
/// \date 2015/12/21 18:10
///
xgc_bool make_tables( xgc::sql::connection_cfg cfg, xgc_lpcstr fconfig, xgc_lpcstr environment = xgc_nullptr );

#endif // _TABLE_MAKER_H_