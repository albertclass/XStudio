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
		/// Ini文件读取类
		/// [8/11/2014] create by albert.xu
		///
		class COMMON_API IniFile
		{
		private:
			struct FileInfo
			{
				/// @var 文件缓冲
				xgc_lpstr mFileBuffer;
				/// @var 文件路径
				xgc_char mFilePath[_MAX_PATH];
				/// @var 文件名
				xgc_char mFileName[_MAX_FNAME];
				/// @var 包含的文件
				xgc_list< FileInfo* > mIncludes;
			};
			
			/// @var 文件缓冲
			FileInfo* mFileRoot;
			/// @var 分析缓冲
			xgc_lpstr mSaveBuffer;

			struct Section;
			Section *pSection;
			struct KeyPair;
			KeyPair *pKeyPair;

			/// @var 翻译缓冲
			xgc_lpstr mTransBuffer;
			/// @var 写偏移
			xgc_size  mTransOffset;

			/// @var 节的个数
			xgc_size mSectionCount;

		public:
			IniFile();

			IniFile( xgc_lpcstr lpFilePath );

			~IniFile();

			///
			/// 加载Ini文件
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool Load( xgc_lpcstr lpFilePath );

			///
			/// 确认段是否存在
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool IsExistSection( xgc_lpcstr lpSection )const;

			///
			/// 确认所需的项是否存在
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool IsExistItem( xgc_lpcstr lpSection, xgc_lpcstr lpItem )const;

			///
			/// 获取节的个数
			/// [8/11/2014] create by albert.xu
			///
			xgc_size GetSectionCount()const
			{
				return mSectionCount;
			}

			///
			/// 获取属性个数
			/// [8/11/2014] create by albert.xu
			///
			xgc_size GetItemCount( xgc_lpcstr lpSection )const;

			///
			/// 获取指定的Section名
			/// [8/11/2014] create by albert.xu
			///
			xgc_lpcstr GetSectionName( xgc_size nSectionIdx )const;

			///
			/// 获取属性的名字
			/// [8/11/2014] create by albert.xu
			///
			xgc_lpcstr GetItemName( xgc_lpcstr lpSection, xgc_size nIdx )const;

			///
			/// 获取指定的数据。
			/// [8/11/2014] create by albert.xu
			/// @param lpSection 节的名称
			/// @param nIdx 属性的索引
			///
			xgc_lpcstr GetItemValue( xgc_lpcstr lpSection, xgc_size nIdx, xgc_lpcstr lpDefault )const;

			///
			/// 获取指定的数据。
			/// [8/11/2014] create by albert.xu
			/// @param lpSection 节的名称
			/// @param lpName 属性的名称
			///
			xgc_lpcstr GetItemValue( xgc_lpcstr lpSection, xgc_lpcstr lpName, xgc_lpcstr lpDefault )const;

			///
			/// 获取数值类型的值
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
			/// 对配置串做变换
			/// [12/4/2014] create by albert.xu
			///
			xgc_long TransformValue( xgc_lpstr lpBuffer, xgc_size nBufferSize, xgc_lpcstr lpValue );

			///
			/// 对配置串做变换< 模板方法 >
			/// [12/4/2014] create by albert.xu
			///
			template< xgc_size size >
			xgc_long TransformValue( xgc_char( &lpBuffer )[size], xgc_lpcstr lpValue )
			{
				return TransformValue( lpBuffer, size, lpValue );
			}
		private:
			///
			/// 释放所有文件
			/// [7/16/2015] create by albert.xu
			///
			xgc_void Free( FileInfo* &pFileInfo );

			///
			/// 加载Ini文件
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool Load( xgc_lpcstr lpFilePath, xgc_lpcstr lpFileName );

			///
			/// 文件分析
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool AnalyzeFile( FileInfo* pFileInfo );

			///
			/// 查找Section
			/// [8/11/2014] create by albert.xu
			///
			const Section* FindSection( xgc_lpcstr lpSection )const;
		};
	}
}

#endif // _INI_H_