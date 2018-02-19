#pragma once
///
/// COPYRIGHT ����������
/// CopyRight  ? 2015 ʢ������
/// \file PythonScript.h
/// \brief �ļ����
/// \author xufeng04
/// \date ���� 2016
///
/// python�ű����
///
///

///
/// \brief python �ű����
///
/// \author albert.xu
/// \date 2016/04/07 13:52
///
class PyScript
{
friend PyScript& getScript();
private:
	///
	/// \brief ����
	///
	/// \author albert.xu
	/// \date 2016/04/07 13:52
	///
	PyScript();

	///
	/// \brief ����
	///
	/// \author albert.xu
	/// \date 2016/04/07 13:52
	///
	~PyScript();

public:
	///
	/// \brief ��ʼ���ű����
	///
	/// \author albert.xu
	/// \date 2016/04/07 13:53
	///
	xgc_bool Init();

	///
	/// \brief ����ű����
	///
	/// \author albert.xu
	/// \date 2016/04/07 13:53
	///
	xgc_void Fini();

	///
	/// \brief ��ȡģ��汾��
	///
	/// \author albert.xu
	/// \date 2016/04/11 11:25
	///
	xgc_bool GetModuleVersion( const xgc_string & module_name, xgc_string & version );

	///
	/// \brief ��ȡ��ܰ汾��
	///
	/// \author albert.xu
	/// \date 2016/04/11 11:26
	///
	xgc_bool GetLoaderVersion( xgc_string & version );

protected:
	///
	/// \brief ��ȡ�汾��
	///
	/// \author albert.xu
	/// \date 2016/04/11 16:37
	///
	xgc_bool GetVersion( const fs::path & p, xgc_string & version );
};

PyScript& getScript();