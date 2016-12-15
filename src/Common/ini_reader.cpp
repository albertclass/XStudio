#include <sys/stat.h>
#include <fcntl.h>
#include "ini_reader.h"
#include "encoding.h"
#include "xsystem.h"

#if defined(_WINDOWS)
#	include <mbstring.h>
#endif

namespace xgc
{
	namespace common
	{
		ini_reader::ini_reader()
			: root( xgc_nullptr )
			, storage( (xgc_lpstr) malloc( 4096 ) )
			, section_ptr( xgc_nullptr )
			, keypair_ptr( xgc_nullptr )
			, trans_buffer( (xgc_lpstr) malloc( 4096 ) )
			, trans_offset( 4 )
			, section_count( 0 )
		{
			memset( storage, 0, memsize( storage ) );
			memset( trans_buffer, 0, memsize( trans_buffer ) );
		}

		ini_reader::~ini_reader()
		{
			fini( root );
			free( storage );
			free( trans_buffer );
		}

		xgc_bool ini_reader::load( xgc_lpcstr path )
		{
			return load( path, "" );
		}

		///
		/// ����Ini�ļ�
		/// [8/11/2014] create by albert.xu
		///
		xgc_bool ini_reader::load( xgc_lpcstr fpath, xgc_lpcstr fname )
		{
			xgc_char path[_MAX_PATH] = { 0 };
			xgc_lpcstr pathname = get_normal_path( path, sizeof( path ), "%s", fpath );

			// ��ָ���ļ�
			int fd = -1;
			#if defined(_WINDOWS)
			_sopen_s( &fd, pathname, O_RDONLY | O_BINARY, SH_DENYWR, S_IREAD );
			#elif defined( __GNUC__ )
			fd = _open( pathname, O_RDONLY, S_IREAD );
			#endif
			if( fd == -1 )
				return false;

			// ��ȡ�ļ���С
			xgc_lpstr buffer = xgc_nullptr;
			struct _stat fst;
			_fstat( fd, &fst );

			// �����ļ�������
			xgc_lpstr palloc = (xgc_lpstr) realloc( buffer, fst.st_size + 1 );
			XGC_ASSERT_RETURN( palloc, false );

			// ��ȡ�ļ�����
			buffer = palloc;
			int buffer_read = _read( fd, palloc, (unsigned int) fst.st_size );
			_close( fd );

			if( buffer_read < 0 )
				return false;

			xgc_size buffer_size = (xgc_size)buffer_read;

			if( buffer_size != fst.st_size )
				return false;

			// ��ջ�����
			buffer[buffer_size] = 0;

			// �����ļ��²���뷽ʽ
			encoding enc = guess_encoding( (xgc_lpvoid*)&buffer, buffer_size );

			// ��UTF-8�ı������ת��
			if( enc == encoding_utf8 )
			{
				// ����ת������Ҫ���ڴ�
				auto len = utf8_to_mbs( buffer, xgc_nullptr, 0 );
				if( len == -1 )
					return false;

				// ת������
				auto ptr = (xgc_lpstr) malloc( len + 1 );
				utf8_to_mbs( buffer, ptr, len );

				buffer = ptr;
				buffer_size = len;

				buffer[buffer_size] = 0;
				free( palloc );
			}

			file_info* info_ptr = XGC_NEW file_info;
			info_ptr->file_buffer	= buffer;
			info_ptr->file_size		= buffer_size;

			strcpy_s( info_ptr->file_path, fpath );
			strcpy_s( info_ptr->file_name, fname );

			if( root )
				root->externs.push_back( info_ptr );
			else
				root = info_ptr;

			return parse( info_ptr );
		}

		struct ini_reader::key_val
		{
			xgc_lpstr	key;
			xgc_lpstr	val;

			xgc_size	trans_val;
		};

		struct ini_reader::section
		{
			xgc_size	next;
			xgc_lpcstr	name;
			xgc_uint16	pair_count;
			key_val		pair[1];
		};

		xgc_bool ini_reader::parse( file_info* pFileInfo )
		{
			xgc_lpstr pCur = pFileInfo->file_buffer;
			xgc_lpstr pMem = xgc_nullptr;

			enum LineType { eNewLine, eEndLine, eError, eCommit, eSection, ePairKey, ePairEqu, ePairValue, eSearchValue, eRealValue, eTransValue };
			LineType eType = eNewLine;

			xgc_size nMemSize = memsize( storage );
			xgc_size nBufSize = memsize( pFileInfo->file_buffer );
			while( pCur < pFileInfo->file_buffer + nBufSize )
			{
				// ��黺���Ƿ��ã������������󻺳�ռ�
				if( storage + nMemSize - (xgc_lpstr) keypair_ptr < XGC_MAX( sizeof( section ), sizeof( key_val ) ) )
				{
					xgc_lpstr pNew = (xgc_lpstr) realloc( storage, memsize( storage ) + 4096 );
					XGC_ASSERT_RETURN( pNew, false );

					memset( pNew + nMemSize, 0, memsize( pNew ) - nMemSize );
					nMemSize = memsize( pNew );

					section_ptr    = (section*)( pNew + ( (xgc_lpstr) section_ptr - storage ) );
					keypair_ptr    = (key_val*)( pNew + ( (xgc_lpstr) keypair_ptr - storage ) );
					storage = pNew;
				}

				// ������ֽ��ַ���
				int mbl = mblen(pCur, (pFileInfo->file_buffer + nBufSize) - pCur);
				if( mbl < 0 ) return false;
				// ���ֽ�����²ŷ���
				switch( eType )
				{
				case eNewLine:
					switch( *pCur )
					{
					case ' ':
					case '\t':
					case '\r':
					case '\n':
						*pCur = 0;
						break;
					case '[':
						pMem = pCur + 1;
						eType = eSection;
						break;
					case ';':
					case '#':
						eType = eCommit;
						break;
					default:
						if( keypair_ptr && (mbl > 1 || isgraph(*pCur)) )
						{
							// *pCur not in ignores character set
							keypair_ptr->key = pCur;
							eType = ePairKey;
						}
						break;
					}
					break;
				case eSection:
					switch( *pCur )
					{
						case ']':
						*pCur = 0;
						eType = eNewLine;

						if( strncasecmp( "Extern:", pMem, 7 ) == 0 )
						{
							// ���ļ�, �����ļ�����
							if( false == load( pFileInfo->file_path, pMem + 7 ) )
								eType = eError;
						}
						else
						{
							if( section_ptr )
							{
								section_ptr->next = (xgc_lpstr) keypair_ptr - storage;
								section_ptr = (section*) (keypair_ptr);
							}
							else
							{
								section_ptr = (section*) storage;
							}

							section_ptr->name = pMem;
							keypair_ptr = section_ptr->pair;

							++section_count;
						}
						break;
						case ' ':
						case '\t':
						case '\r':
						case '\n':
						// *pCur in ignores character set
						*pCur = 0;
						eType = eError;
						break;
					}
					break;
				case ePairKey:
					switch( *pCur )
					{
						case '=':
						*pCur = 0;
						eType = ePairValue;
						break;
						case ' ':
						case '\t':
						*pCur = 0;
						break;
						case '\n':
						*pCur = 0;
						eType = eError;
						break;
					}
					break;
				case ePairValue:
					switch( *pCur )
					{
						case ' ':
						case '\t':
						case '\r':
						{
							*pCur = 0;
						}
						break;
						case '\n':
						{
							keypair_ptr->val = pCur;
							++section_ptr->pair_count;
							++keypair_ptr;

							*pCur = 0;
							eType = eNewLine;
						}
						break;
						default:
						if( mbl > 1 || isgraph(*pCur) )
						{
							keypair_ptr->val = pCur;
							eType = eRealValue;
						}
						break;
					}
					break;
				case eRealValue:
					if( *pCur == '%' && *(pCur + 1) != '%' )
					{
						keypair_ptr->trans_val = trans_offset;
						eType = eTransValue;
					}
					else if( '\n' == *pCur )
					{
						*pCur = 0;

						++section_ptr->pair_count;
						++keypair_ptr;

						eType = eNewLine;
					}
					else if( mbl == 1 && !isgraph(*pCur) )
					{
						*pCur = 0;
					}
					break;
				case eTransValue:
					{
						if( strchr( " \t\r\n", *pCur ) )
						{
							*pCur = 0;
							auto cpy = transform( trans_buffer + trans_offset, memsize( trans_buffer ) - trans_offset, keypair_ptr->val );
							while( cpy < 0 )
							{
								auto pNew = realloc( trans_buffer, memsize( trans_buffer ) + 4096 );
								if( pNew == xgc_nullptr )
									return false;

								trans_buffer = (xgc_lpstr) pNew;

								cpy = transform( trans_buffer + trans_offset, memsize( trans_buffer ) - trans_offset, keypair_ptr->val );
							}

							// ���ӽ�����
							trans_offset += cpy;
							XGC_ASSERT_RETURN( trans_offset < memsize( trans_buffer ), false );

							if( trans_offset == memsize( trans_buffer ) )
							{
								auto pNew = realloc( trans_buffer, memsize( trans_buffer ) + 4096 );
								if( pNew == xgc_nullptr )
									return false;

								trans_buffer = (xgc_lpstr) pNew;
							}

							trans_buffer[trans_offset++] = 0;

							++section_ptr->pair_count;
							++keypair_ptr;

							eType = eNewLine;
						}
					}
					break;
				case eError:
					return false;
				case eCommit:
				case eEndLine:
					if( *pCur == '\n' )
						eType = eNewLine;
					break;
				}
				pCur += mbl;
			}

			return true;
		}

		const ini_reader::section* ini_reader::find_section( xgc_lpcstr lpSection )const
		{
			const section* pSection = (section*) storage;
			if( section_count ) do
			{
				if( strcmp( lpSection, pSection->name ) == 0 )
					return pSection;

				pSection = (const section*) ( storage + pSection->next );
			} while( pSection != (const section*) storage );

			return xgc_nullptr;
		}

		xgc_bool ini_reader::is_exist_section( xgc_lpcstr lpSection )const
		{
			return xgc_nullptr != find_section( lpSection );
		}

		xgc_bool ini_reader::is_exist_item( xgc_lpcstr lpSection, xgc_lpcstr lpItem )const
		{
			return xgc_nullptr != get_item_value( lpSection, lpItem, xgc_nullptr );
		}

		xgc_size ini_reader::get_item_count( xgc_lpcstr lpSection, xgc_lpcstr lpItemName /*= xgc_nullptr*/ )const
		{
			const section* pSection = find_section( lpSection );
			if( xgc_nullptr == pSection )
				return 0;

			if( lpItemName )
			{
				xgc_size nCount = 0;
				for( xgc_uint16 i = 0; i < pSection->pair_count; ++i )
				{
					if( strcmp( pSection->pair[i].key, lpItemName ) == 0 )
						++nCount;
				}
				return nCount;
			}
			else
			{
				return pSection->pair_count;
			}

		}

		xgc_lpcstr ini_reader::get_section_name( xgc_size nSectionIdx )const
		{
			if( nSectionIdx >= section_count )
				return xgc_nullptr;

			const section* pSection = (section*) storage;
			do
			{
				if( nSectionIdx-- == 0 )
					return pSection->name;

				pSection = (const section*) ( storage + pSection->next );
			} while( pSection != (const section*) storage );

			return xgc_nullptr;
		}

		xgc_lpcstr ini_reader::get_item_name( xgc_lpcstr lpSection, xgc_size nIndex )const
		{
			const section* pSection = find_section( lpSection );
			if( xgc_nullptr == pSection )
				return xgc_nullptr;

			if( nIndex >= pSection->pair_count )
				return xgc_nullptr;

			return pSection->pair[nIndex].key;
		}

		xgc_lpcstr ini_reader::get_item_value( xgc_lpcstr lpSection, xgc_size nIndex, xgc_lpcstr lpDefault )const
		{
			const section* pSection = find_section( lpSection );
			if( xgc_nullptr == pSection )
				return lpDefault;

			if( nIndex >= pSection->pair_count )
				return lpDefault;

			if( pSection->pair[nIndex].trans_val )
				return trans_buffer + pSection->pair[nIndex].trans_val;

			return pSection->pair[nIndex].val;
		}

		xgc_lpcstr ini_reader::get_item_value( xgc_lpcstr lpSection, xgc_lpcstr lpItemName, xgc_size nIndex, xgc_lpcstr lpDefault ) const
		{
			const section* pSection = find_section( lpSection );
			if( xgc_nullptr == pSection )
				return lpDefault;

			if( nIndex >= pSection->pair_count )
				return lpDefault;

			xgc_size nCount = 0;
			for( xgc_uint16 i = 0; i < pSection->pair_count; ++i )
			{
				if( strcmp( pSection->pair[i].key, lpItemName ) != 0 )
					continue;

				if( nCount == nIndex )
				{
					if( pSection->pair[nIndex].trans_val )
						return trans_buffer + pSection->pair[nIndex].trans_val;
					else
						return pSection->pair[nIndex].val;
				}

				++nCount;
			}

			return lpDefault;
		}

		xgc_lpcstr ini_reader::get_item_value( xgc_lpcstr lpSection, xgc_lpcstr lpName, xgc_lpcstr lpDefault )const
		{
			const section* pSection = find_section( lpSection );
			if( xgc_nullptr == pSection )
				return lpDefault;

			for( xgc_size nIdx = 0; nIdx < pSection->pair_count; ++nIdx )
			{
				if( strcmp( lpName, pSection->pair[nIdx].key ) == 0 )
				{
					if( pSection->pair[nIdx].trans_val )
						return trans_buffer + pSection->pair[nIdx].trans_val;

					return pSection->pair[nIdx].val;
				}
			}

			return lpDefault;
		}

		///
		/// \brief ��ȡ��ֵ���͵�ֵ
		/// [8/11/2014] create by albert.xu
		///
		xgc_bool ini_reader::get_item_value( xgc_lpcstr lpSection, xgc_lpcstr lpszTitle, xgc_bool bDefault ) const
		{
			xgc_lpcstr pValue = get_item_value( lpSection, lpszTitle, xgc_nullptr );
			if( pValue )
			{
				if( strcasecmp( "true", pValue ) == 0 )
					return true;
				else
					return atoi( pValue ) != 0;
			}

			return bDefault;
		}

		xgc_long ini_reader::transform( xgc_lpstr lpBuffer, xgc_size nBufferSize, xgc_lpcstr lpValue )
		{
			// �������λ��
			xgc_lpstr lpCur = lpBuffer;
			xgc_lpstr lpEnd = lpBuffer + nBufferSize;

			// ��¼��
			xgc_lpcstr lpMem = xgc_nullptr;
			xgc_string loMem[4];

			xgc_size nMem = 0;

			// �Ƿ�������
			xgc_bool bSearch = false;
			// ��ʼ����
			while( *lpValue )
			{
				if( lpCur == lpEnd )
					return -1;

				if( bSearch )
				{
					// ��������ƥ��Ĺ����з��֣�����Ϊ���ҵ�ƥ����
					if( *lpValue == '%' && *( lpValue + 1 ) != '%' )
					{
						loMem[nMem].assign( lpMem, lpValue );
						lpMem = xgc_nullptr; nMem = 0;

						xgc_lpcstr lpItem = get_item_value( loMem[0].c_str(), loMem[1].c_str(), "" );
						int cpy = sprintf_s( lpCur, lpEnd - lpCur, "%s", lpItem );
						if( cpy > 0 )
							lpCur += cpy;

						bSearch = false;
					}
					else if( *lpValue == '.' && nMem < XGC_COUNTOF( loMem ) )
					{
						loMem[nMem++].assign( lpMem, lpValue );
						lpMem = lpValue + 1;
					}
					
					++lpValue;
				}
				else
				{
					if( *lpValue == '%' && *( lpValue + 1 ) == '%' )
					{
						// ˫�ٷֺ�����Ϊ��ת���
						++lpValue;
					}

					if( *lpValue == '%' )
					{
						lpMem = ++lpValue;
						bSearch = true;
						continue;
					}

					*lpCur = *lpValue;
					++lpValue;
					++lpCur;
				}
			}

			if( lpCur < lpEnd )
				*lpCur = 0;

			return xgc_long( lpCur - lpBuffer );
		}

		///
		/// �ͷ������ļ�
		/// [7/16/2015] create by albert.xu
		///
		xgc_void ini_reader::fini( file_info* &pFileInfo )
		{
			if( pFileInfo == xgc_nullptr )
				return;

			free( pFileInfo->file_buffer );

			for( auto it = pFileInfo->externs.begin(); it != pFileInfo->externs.end(); ++it )
			{
				fini( (*it) );
			}

			SAFE_DELETE( pFileInfo );
		}

	}
}