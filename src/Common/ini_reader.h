///
/// CopyRight ? 2016 X Studio
/// \file ini_reader.h
/// \date 八月 2016
///
/// \author albert.xu windxu@126.com
/// \brief ini file reader no write function
/// 

#ifndef _INI_H_
#define _INI_H_
#include "exports.h"
#include "defines.h"
#include <vector>

namespace xgc 
{
	namespace common
	{
		///
		/// Ini文件读取类
		/// [8/11/2014] create by albert.xu
		///
		class COMMON_API ini_reader
		{
		private:
			struct file_info
			{
				/// 文件缓冲
				xgc_lpstr file_buffer;
				/// 文件缓冲长度
				xgc_size file_size;
				/// 文件路径
				xgc_char file_path[_MAX_PATH];
				/// 文件名
				xgc_char file_name[_MAX_FNAME];
				/// 包含的文件
				xgc_list< file_info* > externs;
			};
			
			/// @var 文件缓冲
			file_info* root;
			/// @var 分析缓冲
			xgc_lpstr storage;

			struct section;
			section *section_ptr;
			struct key_val;
			key_val *keypair_ptr;

			/// @var 翻译缓冲
			xgc_lpstr trans_buffer;
			/// @var 写偏移
			xgc_size  trans_offset;

			/// @var 节的个数
			xgc_size  section_count;

		public:
			///
			/// \brief 构造函数
			///
			/// \author albert.xu
			/// \date 2015/12/17 15:51
			///
			ini_reader();

			///
			/// \brief 析构函数
			///
			/// \author albert.xu
			/// \date 2015/12/17 15:51
			///
			~ini_reader();

			///
			/// \brief 加载Ini文件
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool load( xgc_lpcstr path );

			///
			/// \brief 确认段是否存在
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool is_exist_section( xgc_lpcstr lpSection )const;

			///
			/// 确认所需的项是否存在
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool is_exist_item( xgc_lpcstr lpSection, xgc_lpcstr lpItem )const;

			///
			/// \brief 获取节的个数
			/// [8/11/2014] create by albert.xu
			///
			xgc_size get_section_count()const
			{
				return section_count;
			}

			///
			/// \brief 获取属性个数
			/// [8/11/2014] create by albert.xu
			///
			xgc_size get_item_count( xgc_lpcstr lpSection, xgc_lpcstr lpItemName = xgc_nullptr )const;

			///
			/// \brief 获取指定的Section名
			/// [8/11/2014] create by albert.xu
			///
			xgc_lpcstr get_section_name( xgc_size nSectionIdx )const;

			///
			/// \brief 获取属性的名字
			/// [8/11/2014] create by albert.xu
			///
			xgc_lpcstr get_item_name( xgc_lpcstr lpSection, xgc_size nIndex )const;

			///
			/// \brief 获取指定的数据。
			/// [8/11/2014] create by albert.xu
			/// \param lpSection 节的名称
			/// \param nIdx 属性的索引
			///
			xgc_lpcstr get_item_value( xgc_lpcstr lpSection, xgc_size nIndex, xgc_lpcstr lpDefault )const;

			///
			/// \brief 获取指定的数据
			///
			/// \author albert.xu
			/// \date 2015/12/18 17:08
			///
			xgc_lpcstr get_item_value( xgc_lpcstr lpSection, xgc_lpcstr lpItemName, xgc_size nIndex, xgc_lpcstr lpDefault )const;

			///
			/// \brief 获取指定的数据。
			/// [8/11/2014] create by albert.xu
			/// \param lpSection 节的名称
			/// \param lpName 属性的名称
			///
			xgc_lpcstr get_item_value( xgc_lpcstr lpSection, xgc_lpcstr lpName, xgc_lpcstr lpDefault )const;

			///
			/// \brief 获取数值类型的值
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool get_item_value( xgc_lpcstr lpSection, xgc_lpcstr lpszTitle, xgc_bool bDefault )const
			{
				xgc_lpcstr pValue = get_item_value( lpSection, lpszTitle, xgc_nullptr );
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
			T get_item_value( xgc_lpcstr lpSection, xgc_lpcstr lpszTitle, T Default )const
			{
				xgc_lpcstr pValue = get_item_value( lpSection, lpszTitle, xgc_nullptr );

				return pValue ? (T) strtol( pValue, xgc_nullptr, 10 ) : Default;
			}

			template< class T, typename std::enable_if< std::is_integral< T >::value && std::is_unsigned< T >::value != false, xgc_bool >::type = true >
			T get_item_value( xgc_lpcstr lpSection, xgc_lpcstr lpszTitle, T Default )const
			{
				xgc_lpcstr pValue = get_item_value( lpSection, lpszTitle, xgc_nullptr );

				return pValue ? (T) strtoul( pValue, xgc_nullptr, 10 ) : Default;
			}

			template< class T, typename std::enable_if< std::is_floating_point< T >::value, xgc_bool >::type = true >
			T get_item_value( xgc_lpcstr lpSection, xgc_lpcstr lpszTitle, T Default )const
			{
				xgc_lpcstr pValue = get_item_value( lpSection, lpszTitle, xgc_nullptr );

				return pValue ? (T) strtod( pValue, xgc_nullptr ) : Default;
			}

		private:
			///
			/// \brief 释放所有文件
			/// [7/16/2015] create by albert.xu
			///
			xgc_void fini( file_info* &pFileInfo );

			///
			/// \brief 加载Ini文件
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool load( xgc_lpcstr lpFilePath, xgc_lpcstr lpFileName );

			///
			/// \brief 文件分析
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool parse( file_info* pFileInfo );

			///
			/// \brief 查找Section
			/// [8/11/2014] create by albert.xu
			///
			const section* find_section( xgc_lpcstr lpSection )const;

			///
			/// 对配置串做变换
			/// [12/4/2014] create by albert.xu
			///
			xgc_long transform( xgc_lpstr lpBuffer, xgc_size nBufferSize, xgc_lpcstr lpValue );
		};
	}
}

#endif // _INI_H_