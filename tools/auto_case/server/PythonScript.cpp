///////////////////////////////////////////////////////////////
/// COPYRIGHT ����������
/// CopyRight  ? 2015 ʢ������
/// \file PythonScript.cpp
/// \brief �ļ����
/// \author xufeng04
/// \date ���� 2016
///
/// python �ű����
///
///////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "PythonScript.h"
#include "AppConfig.h"

PyScript::PyScript()
{
}

PyScript::~PyScript()
{
}

xgc_bool PyScript::Init()
{
	Py_Initialize();
	return Py_IsInitialized() != 0;
}

xgc_void PyScript::Fini()
{
	Py_Finalize();
}

xgc_bool PyScript::GetModuleVersion( const xgc_string & module_name, xgc_string & version )
{
	return GetVersion( fs::path( getConfig().GetModulePath() ) / module_name, version );
}

xgc_bool PyScript::GetLoaderVersion( xgc_string & version )
{
	return GetVersion( fs::path( getConfig().GetLoaderPath() ), version );
}

xgc_bool PyScript::GetVersion( const fs::path & p, xgc_string & version )
{
	auto version_file = ( p / "version.py" ).string();

	FILE *fp = xgc_nullptr;
	errno_t err = fopen_s( &fp, version_file.c_str(), "rb" );
	if( xgc_nullptr == fp )
		return false;

	PyObject *m = xgc_nullptr, *d = xgc_nullptr, *r = xgc_nullptr, *md = xgc_nullptr, *v = xgc_nullptr;
	xgc_bool set_file_name = false, ret = false;

	m = PyImport_AddModule("__main__");
	if (m == NULL)
		return false;
	Py_INCREF(m);
	d = PyModule_GetDict(m);
	if (PyDict_GetItemString(d, "__file__") == NULL) {
		PyObject *f;
		f = PyUnicode_DecodeFSDefault(version_file.c_str());
		if (f == NULL)
			goto done;
		if (PyDict_SetItemString(d, "__file__", f) < 0) {
			Py_DECREF(f);
			goto done;
		}
		if (PyDict_SetItemString(d, "__cached__", Py_None) < 0) {
			Py_DECREF(f);
			goto done;
		}
		set_file_name = 1;
		Py_DECREF(f);
	}

	r = PyRun_FileEx( fp, version_file.c_str(), Py_file_input, d, d, 1 );

	if( xgc_nullptr == r )
	{
		PyObject *type, *value, *traceback;
		PyErr_Fetch( &type, &value, &traceback );

		const char* perr = xgc_nullptr;
		if( type )
		{
			PyObject *str = PyObject_Str( type );
			if( str )
				perr = PyUnicode_AsUTF8( str );

			Py_XDECREF( str );
		}

		if( value )
		{
			PyObject *str = PyObject_Str( value );
			if( str )
				perr = PyUnicode_AsUTF8( str );

			Py_XDECREF( str );
		}

		if( traceback )
		{
			PyObject *str = PyObject_Str( traceback );
			if( str )
				perr = PyUnicode_AsUTF8( str );

			Py_XDECREF( str );
		}

		goto done;
	}

	Py_DECREF( r );

	v = PyDict_GetItemString( d, "version" );
	if( xgc_nullptr == v )
		goto done;

	version = PyUnicode_AsUTF8( v );
	ret = true;
done:
	if (set_file_name && PyDict_DelItemString(d, "__file__"))
		PyErr_Clear();

	Py_DECREF(m);
	return ret;
}

PyScript & getScript()
{
	// TODO: insert return statement here
	static PyScript inst;
	return inst;
}
