// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#ifndef _WIN32_WINNT		// ����ʹ���ض��� Windows XP ����߰汾�Ĺ��ܡ�
#define _WIN32_WINNT 0x0501	// ����ֵ����Ϊ��Ӧ��ֵ���������� Windows �������汾��
#endif						

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <tchar.h>
#include <list>
#include <string>
#include <vector>
#include <locale>

typedef std::vector< std::string >	CStringList;
// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�
