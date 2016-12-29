#pragma once

#include <string>
#import "c:\program files\common files\system\ado\msado25.tlb" no_namespace rename ("EOF", "adoEOF")

namespace xgc
{
    class CMSSQLConnection;
    
    /**
	* ��װADO��_CommandPtr������Ҫ�������CMSSQLConnectionִ�д洢����
    *
    * CMSSQLCommand��װ��ADO�е�Command���󣬲����ɶ���߳�ͬʱ����������ʵ��
    * ��Ҫ�������CMSSQLConnectionִ�д洢����
    * ����CMSSQLCommandʱ��Ҫ����CMSSQLConnection�Ķ���ָ��
    * ���״ε���AddParameterʱ��Ҫȷ�������б������
    * �ɵ��÷����𲶻�_com_error �쳣
    */
    
    class CMSSQLCommand
    {
        friend class CMSSQLConnection;
    public:
        CMSSQLCommand()
        {
            m_pCommand.CreateInstance(__uuidof(Command));
        }
        
        ~CMSSQLCommand()
        {
        }
        
        ///����Command Type
        void SetCommandType(enum CommandTypeEnum Type) { m_pCommand->CommandType = Type; }
        
        ///����Command Text
        void SetCommandText(const char* pszText) { m_pCommand->CommandText = pszText; }
        
        ///����һ������
        void AddParameter(const char* Name, enum DataTypeEnum Type, enum ParameterDirectionEnum Direction, long Size, const _variant_t& Value = vtMissing)
        {
            m_pCommand->Parameters->Append(m_pCommand->CreateParameter(Name, Type, Direction, Size, Value));
        }
        
        ///��ȡĳ�������ķ���ֵ
        _variant_t GetItemValue(const char* Name) 
        { 
            return m_pCommand->Parameters->GetItem(Name)->GetValue(); 
        }
        
        ///������в�����Ŀǰδ�ṩ
        void ClearParameters()
        {
            for (long i = m_pCommand->Parameters->Count - 1; i >= 0; i--)
                m_pCommand->Parameters->Delete(i);
        }
        
        ///ɾ��һ������
        void DeleteParameter(const char* Name)  
        { 
            m_pCommand->Parameters->Delete(Name); 
        }
        
    private:
        _CommandPtr m_pCommand;
    };
    
    /**
    @brief ��װADO��_RecordsetPtr����
    *
    *	��װADO��_RecordsetPtr���󣬲����ɶ���߳�ͬʱ����������ʵ��
    *  �ɵ��÷����𲶻�_com_error �쳣
    */
    
    class CMSSQLRecordSet
    {
        friend class CMSSQLConnection;
    public:
        CMSSQLRecordSet(){}
        
        ~CMSSQLRecordSet(){};
        
        ///�ƶ�����¼���ײ�
        void MoveFirst() { if (m_pRecordSet)  m_pRecordSet->MoveFirst(); }
        
        ///�ƶ�����¼����һ��
        void MoveNext() { if (m_pRecordSet) m_pRecordSet->MoveNext(); }
        
        ///�ƶ�����¼����һ��
        void MovePrevious() { if (m_pRecordSet)  m_pRecordSet->MovePrevious(); }
        
        ///�ƶ�����¼��β��
        void MoveLast() { if (m_pRecordSet) m_pRecordSet->MoveLast(); }
        
        ///�Ƿ�Խ����¼��ĩβ
        BOOL IsEof()
        { 
            if (m_pRecordSet) 
                return m_pRecordSet->adoEOF; 
            else 
                return false;
        }
        
        ///�Ƿ�Խ����¼���ײ�
        BOOL IsBof() 
        { 
            if (m_pRecordSet) 
                return m_pRecordSet->BOF; 
            else
                return false;
        }
        
        ///�õ���¼����
        DWORD GetRecordCount() 
        { 
	        DWORD nRows = 0;
            
	        nRows = m_pRecordSet->GetRecordCount();

	        if(nRows == -1)
	        {
		        nRows = 0;
		        if(m_pRecordSet->adoEOF != VARIANT_TRUE)
			        m_pRecordSet->MoveFirst();
            	
		        while(m_pRecordSet->adoEOF != VARIANT_TRUE)
		        {
			        nRows++;
			        m_pRecordSet->MoveNext();
		        }
		        if(nRows > 0)
			        m_pRecordSet->MoveFirst();
	        }
            
	        return nRows;
        }
        
        ///��ȡĳһ�ֶε�ֵ
        _variant_t GetFieldValue(const char* pszFieldName)
        {
            _variant_t vtResult;
            if (m_pRecordSet) 
                vtResult = m_pRecordSet->Fields->GetItem(pszFieldName)->Value;
            else
                vtResult.vt = VT_NULL;
            return vtResult;
        }
        
        ///�õ���¼�����ֶεĸ���
        DWORD GetFieldCount() 
        {
            if (m_pRecordSet) 
                return m_pRecordSet->Fields->Count; 
            else
                return 0;
        }
    private:
        _RecordsetPtr m_pRecordSet;
    };
    
    
    /**
    @brief ��װADO��_ConnectionPtr����
    *
    *  ��װADO��_ConnectionPtr���󣬲����ɶ���߳�ͬʱ����������ʵ��
    *  �ڽ���_com_error�쳣���񣬵�����falseʱGetLastErrorInfo�ɻ�ô�������
    */
    
    class CMSSQLConnection
    {
        friend class CMSSQLCommand;
        friend class CMSSQLRecordSet;
    public:
        CMSSQLConnection(void);
        ~CMSSQLConnection(void);
        
        ///���������ַ���
        void SetConnectionString(const char* pszConnectionString) { m_strConnectionString = pszConnectionString; }
        
        ///������
        BOOL Connect(const char* pszConnectionString);
        
        ///������
        BOOL Connect();
        
        ///�ر�����
        void Close();
        
        ///ִ��һ��CMSSQLCommand���󣬲����ؼ�¼��
        BOOL Execute(CMSSQLCommand& command, CMSSQLRecordSet& pSet);
        
        ///ִ��һ��CMSSQLCommand���󣬲����ؼ�¼��
        BOOL Execute(CMSSQLCommand& command);
        
        ///ִ��һ��SQL��䣬�����ؼ�¼��
        BOOL Execute(char* pszSQL, CMSSQLRecordSet& pSet);
        
        ///ִ��һ��SQL��䣬�����ؼ�¼��
        BOOL Execute(char* pszSQL);
        
        ///�Ƿ�������
        BOOL IsConnected();
        
        ///��ȡ��һ�������
        DWORD GetLastError() { return m_dwErrCode; }
        
        ///��ȡ��һ����������
        const char* GetLastErrorInfo() { return m_strErrInfo.c_str(); }
        
    protected:
        
    private:
        //driver=sql Server;PWD=sa;UID=sa;database=db;server=127.0.0.1
        std::string     m_strConnectionString;
        _ConnectionPtr  m_pDBConnection;
        DWORD           m_dwErrCode;
        std::string     m_strErrInfo;
        void SetLastError(DWORD dwErrCode) { m_dwErrCode = dwErrCode; }
        void SetLastError(_com_error& e)
        {
            m_dwErrCode = e.WCode();
            m_strErrInfo = e.Description();
        }
        void SetLastErrorInfo(const char* pszErrInfo) { m_strErrInfo = pszErrInfo; }
        inline void TESTHR(HRESULT x)  { if FAILED(x) _com_issue_error(x); }
    };
};