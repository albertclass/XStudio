#pragma once
///
/// COPYRIGHT 传世工作室
/// CopyRight  ? 2015 盛大网络
/// \file PythonScript.h
/// \brief 文件简介
/// \author xufeng04
/// \date 四月 2016
///
/// python脚本组件
///
///

///
/// \brief python 脚本组件
///
/// \author albert.xu
/// \date 2016/04/07 13:52
///
class PyScript
{
friend PyScript& getScript();
private:
	///
	/// \brief 构造
	///
	/// \author albert.xu
	/// \date 2016/04/07 13:52
	///
	PyScript();

	///
	/// \brief 析构
	///
	/// \author albert.xu
	/// \date 2016/04/07 13:52
	///
	~PyScript();

public:
	///
	/// \brief 初始化脚本组件
	///
	/// \author albert.xu
	/// \date 2016/04/07 13:53
	///
	xgc_bool Init();

	///
	/// \brief 清理脚本组件
	///
	/// \author albert.xu
	/// \date 2016/04/07 13:53
	///
	xgc_void Fini();

	///
	/// \brief 获取模块版本号
	///
	/// \author albert.xu
	/// \date 2016/04/11 11:25
	///
	xgc_bool GetModuleVersion( const xgc_string & module_name, xgc_string & version );

	///
	/// \brief 获取框架版本号
	///
	/// \author albert.xu
	/// \date 2016/04/11 11:26
	///
	xgc_bool GetLoaderVersion( xgc_string & version );

protected:
	///
	/// \brief 获取版本号
	///
	/// \author albert.xu
	/// \date 2016/04/11 16:37
	///
	xgc_bool GetVersion( const fs::path & p, xgc_string & version );
};

PyScript& getScript();