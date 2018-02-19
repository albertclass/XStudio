///////////////////////////////////////////////////////////////
/// COPYRIGHT ����������
/// CopyRight  ? 2015 ʢ������
/// \file Service.h
/// \brief �ļ����
/// \author xufeng04
/// \date ���� 2016
///
/// windows service ��װ
///
///////////////////////////////////////////////////////////////

//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//
#define FACILITY_SYSTEM                  0x0
#define FACILITY_STUBS                   0x3
#define FACILITY_RUNTIME                 0x2
#define FACILITY_IO_ERROR_CODE           0x4


//
// Define the severity codes
//
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


//
// MessageId: SVC_ERROR
//
// MessageText:
//
// An error has occurred (%2).
//
#define SVC_ERROR                        ((DWORD)0xC0000001L)

//
// MessageId: SVC_INFOMATION
//
// MessageText:
//
// Infomation (%2).
//
#define SVC_INFOMATION                   ((DWORD)0x40000002L)

///
/// �������״̬
/// [11/28/2014] create by albert.xu
///
xgc_void ReportServiceStatus( xgc_uint32 nState, xgc_uint32 nExitCode, xgc_uint32 nWaitHint );

///
/// ����������¼�
/// @param nEventType EVENTLOG_
/// @param nErrorCode ServerService.h defined.
/// [11/28/2014] create by albert.xu
///
xgc_void ReportServiceEvent( xgc_uint16 nEventType, xgc_uint32 nErrorCode, xgc_lpcstr lpInfomation );

///
/// �������Ƿ���ֹͣ
/// [11/28/2014] create by albert.xu
///
xgc_bool IsServiceStoped();

///
/// �������Ƿ�����ͣ
/// [11/28/2014] create by albert.xu
///
xgc_bool IsServicePaused();

///
/// �Ƿ�ͨ����������
/// [12/5/2014] create by albert.xu
///
xgc_bool IsService();
