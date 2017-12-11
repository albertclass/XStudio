#pragma once
#ifndef _SERVER_DATABASE_H_
#define _SERVER_DATABASE_H_
//////////////////////////////////////////////////////////////////////////
// 数据库相关的函数
//////////////////////////////////////////////////////////////////////////
///
/// \brief 初始化数据库 
/// \date 12/11/2017
/// \author xufeng04
/// \return 是否初始化成功
///
xgc_bool InitServerDatabase( ini_reader &ini );

///
/// \brief 清理数据库 
/// \date 12/11/2017
/// \author xufeng04
///
xgc_void FiniServerDatabase();

///
/// 同步执行数据库操作
/// [8/13/2014] create by albert.xu
///
xgc_bool SyncDBExecute( xgc_lpcstr lpSql );

///
/// 同步执行数据库操作
/// [12/18/2014] create by albert.xu
///
xgc_void SyncDBExecute( const postcall &post );

///
/// 同步执行数据库操作，返回结果集
/// [8/13/2014] create by albert.xu
///
sql_result SyncDBExecuteRc( xgc_lpcstr lpSql, sql_recordset &pRs );

///
/// 转义字符串
/// [8/13/2014] create by albert.xu
///
xgc_size SyncDBEscapeString( xgc_lpcstr lpSrc, xgc_int32 nSrcSize, xgc_lpstr lpDst, xgc_size nDstSize );

///
/// 转义字符串
/// [8/13/2014] create by albert.xu
///
template< xgc_size S >
xgc_size SyncDBEscapeString( xgc_lpcstr lpSrc, xgc_int32 nSrcSize, xgc_char( &lpDst )[S] )
{
	return SyncDBEscapeString( lpSrc, nSrcSize, lpDst, S );
}

///
/// 获取异步数据库错误描述
/// [8/13/2014] create by albert.xu
///
xgc_uint32 SyncDBErrorCode();

///
/// 获取异步数据库错误描述
/// [8/13/2014] create by albert.xu
///
xgc_lpcstr SyncDBErrorInfo();

///
/// 获取影响的记录条数
/// [1/5/2015] create by albert.xu
///
xgc_size SyncDBAffectRow();

///
/// 投递异步数据库操作
/// [8/13/2014] create by albert.xu
///
xgc_void AsyncDBExecutePost( const xgc::sql::postcall & );

///
/// 执行已返回的数据库操作
/// [8/13/2014] create by albert.xu
///
xgc_size AsyncDBExecuteResp( xgc_size );

#endif // _SERVER_DATABASE_H_