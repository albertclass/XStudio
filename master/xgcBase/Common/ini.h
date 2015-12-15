/*******************************************************************/
//! \file ini.h
//! \brief Ini file reader class
//! 2014/08/11 by Albert.xu
/*******************************************************************/
#ifndef _INI_H_
#define _INI_H_
#include "exports.h"
#include "defines.h"
#include <vector>

namespace XGC 
{
	namespace common
	{
		///
		/// Ini�ļ���ȡ��
		/// [8/11/2014] create by albert.xu
		///
		class COMMON_API IniFile
		{
		private:
			struct FileInfo
			{
				/// @var �ļ�����
				xgc_lpstr mFileBuffer;
				/// @var �ļ�·��
				xgc_char mFilePath[_MAX_PATH];
				/// @var �ļ���
				xgc_char mFileName[_MAX_FNAME];
				/// @var �������ļ�
				xgc_list< FileInfo* > mIncludes;
			};
			
			/// @var �ļ�����
			FileInfo* mFileRoot;
			/// @var ��������
			xgc_lpstr mSaveBuffer;

			struct Section;
			Section *pSection;
			struct KeyPair;
			KeyPair *pKeyPair;

			/// @var ���뻺��
			xgc_lpstr mTransBuffer;
			/// @var дƫ��
			xgc_size  mTransOffset;

			/// @var �ڵĸ���
			xgc_size mSectionCount;

		public:
			IniFile();

			IniFile( xgc_lpcstr lpFilePath );

			~IniFile();

			///
			/// ����Ini�ļ�
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool Load( xgc_lpcstr lpFilePath );

			///
			/// ȷ�϶��Ƿ����
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool IsExistSection( xgc_lpcstr lpSection )const;

			///
			/// ȷ����������Ƿ����
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool IsExistItem( xgc_lpcstr lpSection, xgc_lpcstr lpItem )const;

			///
			/// ��ȡ�ڵĸ���
			/// [8/11/2014] create by albert.xu
			///
			xgc_size GetSectionCount()const
			{
				return mSectionCount;
			}

			///
			/// ��ȡ���Ը���
			/// [8/11/2014] create by albert.xu
			///
			xgc_size GetItemCount( xgc_lpcstr lpSection )const;

			///
			/// ��ȡָ����Section��
			/// [8/11/2014] create by albert.xu
			///
			xgc_lpcstr GetSectionName( xgc_size nSectionIdx )const;

			///
			/// ��ȡ���Ե�����
			/// [8/11/2014] create by albert.xu
			///
			xgc_lpcstr GetItemName( xgc_lpcstr lpSection, xgc_size nIdx )const;

			///
			/// ��ȡָ�������ݡ�
			/// [8/11/2014] create by albert.xu
			/// @param lpSection �ڵ�����
			/// @param nIdx ���Ե�����
			///
			xgc_lpcstr GetItemValue( xgc_lpcstr lpSection, xgc_size nIdx, xgc_lpcstr lpDefault )const;

			///
			/// ��ȡָ�������ݡ�
			/// [8/11/2014] create by albert.xu
			/// @param lpSection �ڵ�����
			/// @param lpName ���Ե�����
			///
			xgc_lpcstr GetItemValue( xgc_lpcstr lpSection, xgc_lpcstr lpName, xgc_lpcstr lpDefault )const;

			///
			/// ��ȡ��ֵ���͵�ֵ
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool GetItemValue( xgc_lpcstr lpSection, xgc_lpcstr lpszTitle, xgc_bool bDefault )const
			{
				xgc_lpcstr pValue = GetItemValue( lpSection, lpszTitle, xgc_nullptr );
				if( pValue )
				{
					if( _stricmp( "true", pValue ) == 0 )
						return true;
					else
						return atoi( pValue ) != 0;
				}

				return bDefault;
			}

			template< class T, typename std::enable_if< std::is_integral< T >::value && std::is_unsigned< T >::value == false, xgc_bool >::type = true >
			T GetItemValue( xgc_lpcstr lpSection, xgc_lpcstr lpszTitle, T Default )const
			{
				xgc_lpcstr pValue = GetItemValue( lpSection, lpszTitle, xgc_nullptr );

				return pValue ? (T) strtol( pValue, xgc_nullptr, 10 ) : Default;
			}

			template< class T, typename std::enable_if< std::is_integral< T >::value && std::is_unsigned< T >::value != false, xgc_bool >::type = true >
			T GetItemValue( xgc_lpcstr lpSection, xgc_lpcstr lpszTitle, T Default )const
			{
				xgc_lpcstr pValue = GetItemValue( lpSection, lpszTitle, xgc_nullptr );

				return pValue ? (T) strtoul( pValue, xgc_nullptr, 10 ) : Default;
			}

			template< class T, typename std::enable_if< std::is_floating_point< T >::value, xgc_bool >::type = true >
			T GetItemValue( xgc_lpcstr lpSection, xgc_lpcstr lpszTitle, T Default )const
			{
				xgc_lpcstr pValue = GetItemValue( lpSection, lpszTitle, xgc_nullptr );

				return pValue ? (T) strtod( pValue, xgc_nullptr ) : Default;
			}

			///
			/// �����ô����任
			/// [12/4/2014] create by albert.xu
			///
			xgc_long TransformValue( xgc_lpstr lpBuffer, xgc_size nBufferSize, xgc_lpcstr lpValue );

			///
			/// �����ô����任< ģ�巽�� >
			/// [12/4/2014] create by albert.xu
			///
			template< xgc_size size >
			xgc_long TransformValue( xgc_char( &lpBuffer )[size], xgc_lpcstr lpValue )
			{
				return TransformValue( lpBuffer, size, lpValue );
			}
		private:
			///
			/// �ͷ������ļ�
			/// [7/16/2015] create by albert.xu
			///
			xgc_void Free( FileInfo* &pFileInfo );

			///
			/// ����Ini�ļ�
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool Load( xgc_lpcstr lpFilePath, xgc_lpcstr lpFileName );

			///
			/// �ļ�����
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool AnalyzeFile( FileInfo* pFileInfo );

			///
			/// ����Section
			/// [8/11/2014] create by albert.xu
			///
			const Section* FindSection( xgc_lpcstr lpSection )const;
		};
	}
}

#endif // _INI_H_