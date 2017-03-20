#pragma once
#ifndef _SERVER_DATABASE_H_
#define _SERVER_DATABASE_H_

//////////////////////////////////////////////////////////////////////////
// ���ݿ���صĺ���
//////////////////////////////////////////////////////////////////////////
///
/// ͬ��ִ�����ݿ����
/// [8/13/2014] create by albert.xu
///
xgc_bool SyncDBExecute( xgc_lpcstr lpSql );

///
/// ͬ��ִ�����ݿ����
/// [12/18/2014] create by albert.xu
///
xgc_void SyncDBExecute( const std::function< XGC::DB::respcall( XGC::DB::sql_connection ) > &fnPost );

///
/// ͬ��ִ�����ݿ���������ؽ����
/// [8/13/2014] create by albert.xu
///
XGC::DB::sql_result SyncDBExecuteRc( xgc_lpcstr lpSql, XGC::DB::sql_recordset &pRs );

///
/// ת���ַ���
/// [8/13/2014] create by albert.xu
///
xgc_size SyncDBEscapeString( xgc_lpcstr lpSrc, xgc_int32 nSrcSize, xgc_lpstr lpDst, xgc_size nDstSize );

///
/// ת���ַ���
/// [8/13/2014] create by albert.xu
///
template< xgc_size S >
xgc_size SyncDBEscapeString( xgc_lpcstr lpSrc, xgc_int32 nSrcSize, xgc_char( &lpDst )[S] )
{
	return SyncDBEscapeString( lpSrc, nSrcSize, lpDst, S );
}

///
/// ��ȡ�첽���ݿ��������
/// [8/13/2014] create by albert.xu
///
xgc_uint32 SyncDBErrorCode();

///
/// ��ȡ�첽���ݿ��������
/// [8/13/2014] create by albert.xu
///
xgc_lpcstr SyncDBErrorInfo();

///
/// ��ȡӰ��ļ�¼����
/// [1/5/2015] create by albert.xu
///
xgc_size SyncDBAffectRow();

///
/// Ͷ���첽���ݿ����
/// [8/13/2014] create by albert.xu
///
xgc_void AsyncDBExecutePost( const XGC::DB::postcall & );

///
/// ִ���ѷ��ص����ݿ����
/// [8/13/2014] create by albert.xu
///
xgc_size AsyncDBExecuteResp( xgc_size );

#endif // _SERVER_DATABASE_H_