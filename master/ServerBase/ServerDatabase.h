#pragma once
#ifndef _SERVER_DATABASE_H_
#define _SERVER_DATABASE_H_

//////////////////////////////////////////////////////////////////////////
// 数据库相关的函数
//////////////////////////////////////////////////////////////////////////
///
/// 同步执行数据库操作
/// [8/13/2014] create by albert.xu
///
xgc_bool SyncDBExecute( xgc_lpcstr lpSql );

///
/// 同步执行数据库操作
/// [12/18/2014] create by albert.xu
///
xgc_void SyncDBExecute( const std::function< XGC::DB::respcall( XGC::DB::sql_connection ) > &fnPost );

///
/// 同步执行数据库操作，返回结果集
/// [8/13/2014] create by albert.xu
///
XGC::DB::sql_result SyncDBExecuteRc( xgc_lpcstr lpSql, XGC::DB::sql_recordset &pRs );

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
xgc_void AsyncDBExecutePost( const XGC::DB::postcall & );

///
/// 执行已返回的数据库操作
/// [8/13/2014] create by albert.xu
///
xgc_size AsyncDBExecuteResp( xgc_size );

#endif // _SERVER_DATABASE_H_