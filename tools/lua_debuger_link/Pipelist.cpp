#include "stdafx.h"

#define FileDirectoryInformation 1
#define STATUS_NO_MORE_FILES 0x80000006L

typedef struct
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct
{
	LONG Status;
	ULONG Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef struct {
	ULONG NextEntryOffset;
	ULONG FileIndex;
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	LARGE_INTEGER EndOfFile;
	LARGE_INTEGER AllocationSize;
	ULONG FileAttributes;
	ULONG FileNameLength;
	union {
		struct {
			WCHAR FileName[1];
		} FileDirectoryInformationClass;

		struct {
			DWORD dwUknown1;
			WCHAR FileName[1];
		} FileFullDirectoryInformationClass;

		struct {
			DWORD dwUknown2;
			USHORT AltFileNameLen;
			WCHAR AltFileName[12];
			WCHAR FileName[1];
		} FileBothDirectoryInformationClass;
	};
} FILE_QUERY_DIRECTORY, *PFILE_QUERY_DIRECTORY;


// ntdll!NtQueryDirectoryFile (NT specific!)
//
// The function searches a directory for a file whose name and attributes
// match those specified in the function call.
//
// NTSYSAPI
// NTSTATUS
// NTAPI
// NtQueryDirectoryFile(
//    IN HANDLE FileHandle,                      // handle to the file
//    IN HANDLE EventHandle OPTIONAL,
//    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
//    IN PVOID ApcContext OPTIONAL,
//    OUT PIO_STATUS_BLOCK IoStatusBlock,
//    OUT PVOID Buffer,                          // pointer to the buffer to receive the result
//    IN ULONG BufferLength,                     // length of Buffer
//    IN FILE_INFORMATION_CLASS InformationClass,// information type
//    IN BOOLEAN ReturnByOne,                    // each call returns info for only one file
//    IN PUNICODE_STRING FileTemplate OPTIONAL,  // template for search
//    IN BOOLEAN Reset                           // restart search
// );
typedef LONG (WINAPI *PROCNTQDF)( HANDLE,HANDLE,PVOID,PVOID,PIO_STATUS_BLOCK,PVOID,ULONG,
								 UINT,BOOL,PUNICODE_STRING,BOOL );

PROCNTQDF NtQueryDirectoryFile;

bool pipelist( CStringList& l )
{
	LONG ntStatus;
	IO_STATUS_BLOCK IoStatus;
	HANDLE hPipe;
	BOOL bReset = TRUE;
	PFILE_QUERY_DIRECTORY DirInfo,
		TmpInfo;

	NtQueryDirectoryFile = (PROCNTQDF)GetProcAddress(
		GetModuleHandleA( _T("ntdll") ),
		_T("NtQueryDirectoryFile")
		);

	if (!NtQueryDirectoryFile)
		return TRUE;

	hPipe = CreateFile( _T("\\\\.\\Pipe\\"), GENERIC_READ,
		FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
		NULL,OPEN_EXISTING,0,NULL);

	if(hPipe == INVALID_HANDLE_VALUE)
		return TRUE;

	DirInfo = (PFILE_QUERY_DIRECTORY) new BYTE[0x1000];

	while(1)
	{
		ntStatus = NtQueryDirectoryFile(hPipe,NULL,NULL,NULL,&IoStatus,DirInfo,0x1000,
			FileDirectoryInformation,FALSE,NULL,bReset);

		if (ntStatus!=NO_ERROR)
		{
			if (ntStatus == STATUS_NO_MORE_FILES)
				break;

			return TRUE;
		}

		TmpInfo = DirInfo;
		while(1)
		{
			if( wcsnicmp( L"lua\\", TmpInfo->FileDirectoryInformationClass.FileName, 4 ) == 0 )
			{
				LPWSTR name = (wchar_t*)malloc( ( TmpInfo->FileNameLength + 1 )* sizeof(wchar_t) );
				wcsncpy( name, TmpInfo->FileDirectoryInformationClass.FileName, TmpInfo->FileNameLength );
				name[TmpInfo->FileNameLength/2] = 0;
				//LPSTR tok = _tcstok( W2T(name), _T(".") );

				//if( tok == NULL ) break;
				//int i = m_NamepipeListCtrl.InsertItem( 0, tok );

				//tok = _tcstok( NULL, _T(".") );
				//if( tok == NULL ) break;
				//m_NamepipeListCtrl.SetItemText( i, 1, tok );

				//tok = _tcstok( NULL, _T(".") );
				//if( tok == NULL ) break;
				//m_NamepipeListCtrl.SetItemText( i, 2, tok );
				char n[1024];
				wcstombs( n, name, TmpInfo->FileNameLength + 1 );
				l.push_back( n );
				free( name );
			}

			if(TmpInfo->NextEntryOffset==0)
				break;
			else
				TmpInfo = (PFILE_QUERY_DIRECTORY)((DWORD)TmpInfo+TmpInfo->NextEntryOffset);
		}

		bReset = FALSE;
	}

	delete DirInfo;
	CloseHandle(hPipe);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
