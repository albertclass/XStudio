///
/// CopyRight ? 2016 X Studio
/// \file ini_reader.h
/// \date ���� 2016
///
/// \author albert.xu windxu@126.com
/// \brief ini file reader no write function
///

#ifndef _INI_H_
#define _INI_H_
#include "defines.h"
#include "exports.h"

namespace xgc
{
	namespace common
	{
		///
		/// Ini�ļ���ȡ��
		/// [8/11/2014] create by albert.xu
		///
		class COMMON_API ini_reader
		{
		private:
			struct file_info
			{
				/// �ļ�����
				xgc_lpstr file_buffer;
				/// �ļ����峤��
				xgc_size file_size;
				/// �ļ�·��
				xgc_char file_path[XGC_MAX_PATH];
				/// �ļ���
				xgc_char file_name[XGC_MAX_FNAME];
				/// �������ļ�
				xgc_list< file_info* > externs;
			};

			/// @var �ļ�����
			file_info* root;
			/// @var ��������
			xgc_lpstr storage;

			struct section;
			section *section_ptr;
			struct key_val;
			key_val *keypair_ptr;

			/// @var ���뻺��
			xgc_lpstr trans_buffer;
			/// @var дƫ��
			xgc_size  trans_offset;

			/// @var �ڵĸ���
			xgc_size  section_count;

		public:
			typedef const section* const psection;
			///
			/// \brief ���캯��
			///
			/// \author albert.xu
			/// \date 2015/12/17 15:51
			///
			ini_reader();

			///
			/// \brief ��������
			///
			/// \author albert.xu
			/// \date 2015/12/17 15:51
			///
			~ini_reader();

			///
			/// \brief ����Ini�ļ�
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool load( xgc_lpcstr path );

			///
			/// \brief ȷ�϶��Ƿ����
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool is_exist_section( xgc_lpcstr lpSection )const;

			///
			/// ȷ����������Ƿ����
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool is_exist_item( xgc_lpcstr lpSection, xgc_lpcstr lpItem )const;

			///
			/// \brief ��ȡ�ڵĸ���
			/// [8/11/2014] create by albert.xu
			///
			xgc_size get_section_count()const
			{
				return section_count;
			}

			///
			/// \brief ����Section
			/// [8/11/2014] create by albert.xu
			///
			psection get_section( xgc_size nSection )const;

			///
			/// \brief ����Section
			/// [8/11/2014] create by albert.xu
			///
			psection get_section( xgc_lpcstr lpSection )const;

			///
			/// \brief ��ȡָ����Section��
			/// [8/11/2014] create by albert.xu
			///
			xgc_lpcstr get_section_name( psection pSection )const;

			///
			/// \brief ��ȡ���Ը���
			/// [8/11/2014] create by albert.xu
			///
			xgc_size get_item_count( xgc_lpcstr lpSection, xgc_lpcstr lpItemName = xgc_nullptr )const;

			///
			/// \brief ��ȡ���Ը���
			/// [8/11/2014] create by albert.xu
			///
			xgc_size get_item_count( psection lpSection, xgc_lpcstr lpItemName = xgc_nullptr )const;

			///
			/// \brief ��ȡ���Ե�����
			/// [8/11/2014] create by albert.xu
			///
			xgc_lpcstr get_item_name( xgc_lpcstr lpSection, xgc_size nIndex )const;

			///
			/// \brief ��ȡ���Ե�����
			/// [8/11/2014] create by albert.xu
			///
			xgc_lpcstr get_item_name( psection lpSection, xgc_size nItem )const;

			///
			/// \brief ��ȡָ�������ݡ�
			/// [8/11/2014] create by albert.xu
			/// \param lpSection �ڵ�����
			/// \param nIdx ���Ե�����
			///
			xgc_lpcstr get_item_value( xgc_lpcstr lpSection, xgc_size nIndex, xgc_lpcstr lpDefault )const;

			///
			/// \brief ��ȡָ�������ݡ�
			/// [8/11/2014] create by albert.xu
			/// \param lpSection �ڵ�����
			/// \param nIdx ���Ե�����
			///
			xgc_lpcstr get_item_value( psection lpSection, xgc_size nIndex, xgc_lpcstr lpDefault )const;

			///
			/// \brief ��ȡָ��������
			///
			/// \author albert.xu
			/// \date 2015/12/18 17:08
			///
			xgc_lpcstr get_item_value( xgc_lpcstr lpSection, xgc_lpcstr lpItemName, xgc_size nIndex, xgc_lpcstr lpDefault )const;

			///
			/// \brief ��ȡָ��������
			///
			/// \author albert.xu
			/// \date 2015/12/18 17:08
			///
			xgc_lpcstr get_item_value( psection lpSection, xgc_lpcstr lpItemName, xgc_size nIndex, xgc_lpcstr lpDefault )const;

			///
			/// \brief ��ȡָ�������ݡ�
			/// [8/11/2014] create by albert.xu
			/// \param lpSection �ڵ�����
			/// \param lpName ���Ե�����
			///
			xgc_lpcstr get_item_value( xgc_lpcstr lpSection, xgc_lpcstr lpItemName, xgc_lpcstr lpDefault )const;

			///
			/// \brief ��ȡָ�������ݡ�
			/// [8/11/2014] create by albert.xu
			/// \param lpSection �ڵ�����
			/// \param lpName ���Ե�����
			///
			xgc_lpcstr get_item_value( psection lpSection, xgc_lpcstr lpItemName, xgc_lpcstr lpDefault )const;

			///
			/// \brief ��ȡ��ֵ���͵�ֵ
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool get_item_value( xgc_lpcstr lpSection, xgc_lpcstr lpItemName, xgc_bool bDefault )const;

			///
			/// \brief ��ȡ��ֵ���͵�ֵ
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool get_item_value( psection lpSection, xgc_lpcstr lpItemName, xgc_bool bDefault )const;

			///
			/// \brief ��ȡ��ֵ���͵�ֵ
			/// [8/11/2014] create by albert.xu
			///
			template< class T, class = typename std::enable_if< is_numeric<T>::value >::type >
			T get_item_value( xgc_lpcstr lpSection, xgc_lpcstr lpItemName, T Default )const
			{
				xgc_lpcstr pValue = get_item_value( lpSection, lpItemName, xgc_nullptr );

				return pValue ? str2numeric<T>( pValue, xgc_nullptr ) : Default;
			}

			///
			/// \brief ��ȡ��ֵ���͵�ֵ
			/// [8/11/2014] create by albert.xu
			///
			template< class T, class = typename std::enable_if< is_numeric<T>::value >::type >
			T get_item_value( psection lpSection, xgc_lpcstr lpItemName, T Default )const
			{
				xgc_lpcstr pValue = get_item_value( lpSection, lpItemName, xgc_nullptr );

				return pValue ? str2numeric<T>( pValue, xgc_nullptr ) : Default;
			}

		private:
			///
			/// \brief �ͷ������ļ�
			/// [7/16/2015] create by albert.xu
			///
			xgc_void fini( file_info* &pFileInfo );

			///
			/// \brief ����Ini�ļ�
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool load( xgc_lpcstr lpFilePath, xgc_lpcstr lpFileName );

			///
			/// \brief �ļ�����
			/// [8/11/2014] create by albert.xu
			///
			xgc_bool parse( file_info* pFileInfo );

			///
			/// \brief ����Section
			/// [8/11/2014] create by albert.xu
			///
			const section* find_section( xgc_lpcstr lpSection, const section* after = xgc_nullptr )const;

			///
			/// �����ô����任
			/// [12/4/2014] create by albert.xu
			///
			xgc_long transform( xgc_lpstr lpBuffer, xgc_size nBufferSize, xgc_lpcstr lpValue );
		};
	}
}

#endif // _INI_H_