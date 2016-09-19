#pragma once

#ifdef CORE_EXPORTS
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif	//CORE_EXPORTS
#pragma warning( disable:4251 )

#include <Windows.h>
#include "stdafx.h"
#include "serialization.h"

namespace XGC
{
    //////////////////////////////////////////////////////////////////////////
    // [1/21/2014 jianglei.kinly]
    // action ���õ�ö�ٶ���
    // �ϲ�ֻ��ָ����Ҫ���õ�ö��ֵ���ɻ�ö�Ӧ��action map(��̬��ʼ��) / action pair(��̬��ʼ��)
    //////////////////////////////////////////////////////////////////////////
    enum EN_ACTION_CONFIG_LIST
    {
        enSkillAction,  // ����action -> xml:ActionSkillConfig.xml
        
        enActionMax     // ��Чֵ
    };

    // ����EN_ACTION_CONFIG_LIST��Ӧ��action config xml path
    // TODO: �����ʵ��Ӧ�ü��ؽ����� ��û�����ô��
    static xgc_lpcstr XMLPATH_SKILLACTION = "";

    ///////////////////////////////////////////////////////////////
    // action config xml �ĸ�ʽ�ֶζ���
    static xgc_lpcstr ELEM_ACTIONS        = "Actions";
    static xgc_lpcstr ELEM_ACTION         = "Action";
    static xgc_lpcstr ATTR_NAME           = "name";
    static xgc_lpcstr ELEM_ARG            = "arg";
    static xgc_lpcstr ATTR_ARG_NAME       = "name";
    static xgc_lpcstr ATTR_ARG_TYPE       = "type";
    static xgc_lpcstr ATTR_ARG_HASDEFAULT = "hasDefault";
    static xgc_lpcstr ATTR_ARG_DEFAULT    = "default";
    ///////////////////////////////////////////////////////////////

    typedef struct _tagTypeNode
    {
        xgc_int32 theInt;
        xgc_float32 theFloat;
        xgc_string theString;
    }stTypeNode;

    typedef struct _tagActionFunction
    {
        xgc_string strName;
        xgc_string strType;
        xgc_bool hasDefault;
        stTypeNode stNode;
    }stActionFunction;

    class XActionMgr
    {
    private:
        // Action �����������map�У���ActionName��KEY
        xgc_unordered_map<xgc_string, xgc_vector<stActionFunction> > m_umapAction;

    private:
        xgc_bool StringToValueByType(xgc_lpcstr type, xgc_lpcstr _value, stTypeNode& node)
        {
            if (type == "int")
                node.theInt = atoi(_value);
            else if (type == "float")
                node.theFloat = atof(_value);
            else if (type == "string")
                node.theString = _value;
            else
                return false;
            return true;
        }
        //////////////////////////////////////////////////////////////////////////
        // [1/21/2014 jianglei.kinly]
        // enum to 
        //////////////////////////////////////////////////////////////////////////
        xgc_lpcstr enum2path(EN_ACTION_CONFIG_LIST enArg) const
        {
            switch (enArg)
            {
            case XGC::enSkillAction:
                return XGC::XMLPATH_SKILLACTION;
            case XGC::enActionMax:
                return xgc_nullptr;
            default:
                return xgc_nullptr;
            }
            return xgc_nullptr;
        }
    public:
        XActionMgr()
        {}
        ~XActionMgr()
        {}
        //////////////////////////////////////////////////////////////////////////
        // [1/20/2014 jianglei.kinly]
        // ���������ļ�
        //////////////////////////////////////////////////////////////////////////
        xgc_bool Init(const xgc_string& xmlPath);
        //////////////////////////////////////////////////////////////////////////
        // [1/21/2014 jianglei.kinly]
        // ��̬����action ���ص������ݴ��� m_pairAction
        //////////////////////////////////////////////////////////////////////////
        xgc_bool DynamicInit(const xgc_string& xmlPath, const xgc_string& action, std::vector<stActionFunction>& vec);
        //////////////////////////////////////////////////////////////////////////
        // [1/21/2014 jianglei.kinly]
        // ��̬����action�ӿ�
        //////////////////////////////////////////////////////////////////////////
        xgc_bool DynamicLoadAction(EN_ACTION_CONFIG_LIST en, const xgc_string& action, std::vector<stActionFunction>& vec)
        {
            xgc_lpcstr xmlPath = enum2path(en);
            
            XGC_ASSERT_RETURN(xmlPath == xgc_nullptr, false);

            XGC_ASSERT_RETURN(DynamicInit(xmlPath, action, vec), false);

            return true;
        }
        //////////////////////////////////////////////////////////////////////////
        // [1/20/2014 jianglei.kinly]
        // �ж�arg�ǲ�������,����ȡ�������л�����
        //////////////////////////////////////////////////////////////////////////
        xgc_bool IsArgFull(const xgc_vector<stActionFunction>& vec, const xgc_unordered_map<xgc_string, xgc_string>& mapArgs, common::serialization& res)
        {
            FUNCTION_BEGIN;
            for (auto it = vec.begin(); it != vec.end(); ++it)
            {
                stActionFunction node = *it;
                auto it2 = mapArgs.find(node.strName);
                if (it2 == mapArgs.end())
                {
                    if (node.hasDefault)
                    {
                        if (node.strType == "int")
                            res << node.stNode.theInt;
                        else if (node.strType == "float")
                            res << node.stNode.theFloat;
                        else if (node.strType == "string")
                            res << node.stNode.theString.c_str();
                    }
                    else
                        return false;
                }
                else
                {
                    if (node.strType == "int")
                        res << atoi(it2->second.c_str());
                    else if (node.strType == "float")
                        res << (float)atof(it2->second.c_str());
                    else if (node.strType == "string")
                        res << it2->second.c_str();
                }
            }

            return true;
            FUNCTION_END;
            return false;
        }
        //////////////////////////////////////////////////////////////////////////
        // [1/20/2014 jianglei.kinly]
        // �ж��Ƿ���������
        //////////////////////////////////////////////////////////////////////////
        xgc_bool IsInConfig(xgc_lpcstr actionName)
        {
            auto it = m_umapAction.find(actionName);
            if (it == m_umapAction.end())
                return false;
            return true;
        }
        //////////////////////////////////////////////////////////////////////////
        // [1/20/2014 jianglei.kinly]
        // ֱ�ӷ�������map const
        //////////////////////////////////////////////////////////////////////////
        const xgc_unordered_map<xgc_string, xgc_vector<stActionFunction> >& GetActionMap()
        {
            return m_umapAction;
        }
    };
}