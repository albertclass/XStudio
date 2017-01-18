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

		xgc_bool ini_reader::load( xgc_lpcstr fpath )
		{
			xgc_char path[XGC_MAX_PATH]  = {0};
			xgc_char file[XGC_MAX_FNAME] = {0};

			xgc_lpcstr slash = "\\/";
			xgc_lpcstr last_slash = xgc_nullptr;
			xgc_lpcstr find_slash = strpbrk( fpath, slash );
			while( find_slash )
			{
				last_slash = find_slash;
				find_slash = strpbrk( find_slash + 1, slash );
			}

			if( last_slash )
			{
				strncpy_s( path, fpath, last_slash - fpath );
				strncpy_s( file, last_slash + 1, _TRUNCATE );
			}
			else
			{
				strcpy_s( file, fpath );
			}

			return load( path, file );
		}

		///
		/// 加载Ini文件
		/// [8/11/2014] create by albert.xu
		///
		xgc_bool ini_reader::load( xgc_lpcstr fpath, xgc_lpcstr fname )
		{
			xgc_char path[XGC_MAX_PATH] = { 0 };
			xgc_lpcstr pathname = get_normal_path( path, sizeof( path ), "%s/%s", fpath, fname );

			// 打开指定文件
			int fd = -1;
			#if defined(_WINDOWS)
			_sopen_s( &fd, pathname, O_RDONLY | O_BINARY, SH_DENYWR, S_IREAD );
			#elif defined( __GNUC__ )
			fd = open( pathname, O_RDONLY, S_IREAD );
			#endif
			if( fd == -1 )
				return false;

			// 获取文件大小
			xgc_lpstr buffer = xgc_nullptr;
			struct _stat fst;
			_fstat( fd, &fst );

			// 分配文件缓冲区
			xgc_lpstr palloc = (xgc_lpstr) realloc( buffer, fst.st_size + 1 );
			XGC_ASSERT_RETURN( palloc, false );

			// 读取文件内容
			buffer = palloc;
			int buffer_read = _read( fd, palloc, (unsigned int) fst.st_size );
			_close( fd );

			if( buffer_read < 0 )
				return false;

			xgc_size buffer_size = (xgc_size)buffer_read;

			if( buffer_size != fst.st_size )
				return false;

			// 封闭缓冲区
			buffer[buffer_size] = 0;

			// 根据文件猜测编码方式
			encoding enc = guess_encoding( (xgc_lpvoid*)&buffer, buffer_size );

			// 对UTF-8的编码进行转换
			if( enc == encoding_utf8 )
			{
				// 计算转换后需要的内存
				auto len = utf8_to_mbs( buffer, xgc_nullptr, 0 );
				if( len == -1 )
					return false;

				// 转换编码
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
			xgc_size nBufSize = pFileInfo->file_size;
			while( pCur < pFileInfo->file_buffer + nBufSize )
			{
				// 检查缓冲是否够用，不够用则扩大缓冲空间
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

				// 处理多字节字符集
				int mbl = mblen(pCur, (pFileInfo->file_buffer + nBufSize) - pCur);
				if( mbl < 0 ) return false;
				// 单字节情况下才分析
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
							// 打开文件, 分析文件内容
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
					else if( mbl == 1 && !isprint(*pCur) )
					{
						*pCur = 0;
					}
					break;
				case eTransValue:
					{
						if( strchr(" \t\r\n", *pCur) )
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

							// 增加结束符
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

		const ini_reader::section* ini_reader::find_section( xgc_lpcstr lpSection, const section* after )const
		{
			if( after && after->next == 0 )
				return xgc_nullptr;

			const section* pSection = (const section*) (after ? (storage + after->next) : storage);
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

		ini_reader::psection ini_reader::get_section( xgc_size nSection )const
		{
			if( nSection >= section_count )
				return xgc_nullptr;

			const section* pSection = (section*) storage;
			do
			{
				if( nSection-- == 0 )
					return pSection;

				pSection = (const section*) ( storage + pSection->next );
			} while( pSection != (const section*) storage );

			return xgc_nullptr;
		}

		ini_reader::psection ini_reader::get_section( xgc_lpcstr lpSection ) const
		{
			const section* pSection = (section*) storage;
			do
			{
				if( strcmp( pSection->name, lpSection ) == 0 )
					return pSection;

				pSection = (const section*) ( storage + pSection->next );
			} while( pSection != (const section*) storage );

			return xgc_nullptr;
		}

		xgc_lpcstr ini_reader::get_section_name( const ini_reader::psection pSection )const
		{
			return pSection ? pSection->name : xgc_nullptr;
		}

		xgc_size ini_reader::get_item_count( xgc_lpcstr lpSection, xgc_lpcstr lpItemName /*= xgc_nullptr*/ )const
		{
			xgc_size nCount = 0;
			const section* pSection = find_section( lpSection );
			while( pSection )
			{
				if( lpItemName )
				{
					for( xgc_uint16 i = 0; i < pSection->pair_count; ++i )
					{
						if( strcmp( pSection->pair[i].key, lpItemName ) == 0 )
							++nCount;
					}
				}
				else
				{
					nCount += pSection->pair_count;
				}

				pSection = find_section( lpSection, pSection );
			}

			return nCount;
		}

		xgc_size ini_reader::get_item_count( ini_reader::psection lpSection, xgc_lpcstr lpItemName /*= xgc_nullptr*/ )const
		{
			XGC_ASSERT_RETURN( lpSection, 0 );

			if( lpItemName )
			{
				xgc_size nCount = 0;
				for( xgc_uint16 i = 0; i < lpSection->pair_count; ++i )
				{
					if( strcmp( lpSection->pair[i].key, lpItemName ) == 0 )
						++nCount;
				}

				return nCount;
			}
			else
			{
				return lpSection->pair_count;
			}

			return 0;
		}

		xgc_lpcstr ini_reader::get_item_name( xgc_lpcstr lpSection, xgc_size nIndex )const
		{
			const section* pSection = find_section( lpSection );
			while( pSection )
			{
				if( nIndex < pSection->pair_count )
				{
					return pSection->pair[nIndex].key;
				}

				nIndex -= pSection->pair_count;
				pSection = find_section(lpSection, pSection);
			}

			return xgc_nullptr;
		}

		xgc_lpcstr ini_reader::get_item_name( ini_reader::psection lpSection, xgc_size nItem )const
		{
			XGC_ASSERT_RETURN( lpSection, xgc_nullptr );

			if( lpSection && nItem < lpSection->pair_count )
				return lpSection->pair[nItem].key;

			return xgc_nullptr;
		}

		xgc_lpcstr ini_reader::get_item_value( xgc_lpcstr lpSection, xgc_size nIndex, xgc_lpcstr lpDefault )const
		{
			const section* pSection = find_section( lpSection );
			while( pSection )
			{
				if( nIndex < pSection->pair_count )
				{
					if( pSection->pair[nIndex].trans_val )
						return trans_buffer + pSection->pair[nIndex].trans_val;

					return pSection->pair[nIndex].val;
				}

				nIndex -= pSection->pair_count;
				pSection = find_section( lpSection, pSection );
			}

			return lpDefault;
		}

		xgc_lpcstr ini_reader::get_item_value( ini_reader::psection lpSection, xgc_size nIndex, xgc_lpcstr lpDefault )const
		{
			XGC_ASSERT_RETURN( lpSection, lpDefault );

			if( nIndex < lpSection->pair_count )
			{
				if( lpSection->pair[nIndex].trans_val )
					return trans_buffer + lpSection->pair[nIndex].trans_val;

				return lpSection->pair[nIndex].val;
			}

			return lpDefault;
		}

		xgc_lpcstr ini_reader::get_item_value( xgc_lpcstr lpSection, xgc_lpcstr lpItemName, xgc_size nIndex, xgc_lpcstr lpDefault ) const
		{
			xgc_size nCount = 0;
			const section* pSection = find_section( lpSection );
			while( pSection )
			{
				for( xgc_uint16 i = 0; i < pSection->pair_count; ++i )
				{
					if( strcmp( pSection->pair[i].key, lpItemName ) != 0 )
						continue;

					if( nCount == nIndex )
					{
						if( pSection->pair[i].trans_val )
							return trans_buffer + pSection->pair[i].trans_val;
						else
							return pSection->pair[i].val;
					}

					++nCount;
				}

				pSection = find_section( lpSection, pSection );
			}

			return lpDefault;
		}

		xgc_lpcstr ini_reader::get_item_value( ini_reader::psection lpSection, xgc_lpcstr lpItemName, xgc_size nIndex, xgc_lpcstr lpDefault )const
		{
			XGC_ASSERT_RETURN( lpSection, lpDefault );
			xgc_size nCount = 0;

			for( xgc_uint16 i = 0; i < lpSection->pair_count; ++i )
			{
				if( strcmp( lpSection->pair[i].key, lpItemName ) != 0 )
					continue;

				if( nCount == nIndex )
				{
					if( lpSection->pair[i].trans_val )
						return trans_buffer + lpSection->pair[i].trans_val;
					else
						return lpSection->pair[i].val;
				}

				++nCount;
			}

			return lpDefault;
		}

		xgc_lpcstr ini_reader::get_item_value( xgc_lpcstr lpSection, xgc_lpcstr lpItemName, xgc_lpcstr lpDefault )const
		{
			const section* pSection = find_section( lpSection );
			while( pSection )
			{
				for( xgc_size nIdx = 0; nIdx < pSection->pair_count; ++nIdx )
				{
					if( strcmp( lpItemName, pSection->pair[nIdx].key ) == 0 )
					{
						if( pSection->pair[nIdx].trans_val )
							return trans_buffer + pSection->pair[nIdx].trans_val;

						return pSection->pair[nIdx].val;
					}
				}

				pSection = find_section( lpSection, pSection );
			}

			return lpDefault;
		}

		xgc_lpcstr ini_reader::get_item_value( ini_reader::psection lpSection, xgc_lpcstr lpItemName, xgc_lpcstr lpDefault )const
		{
			XGC_ASSERT_RETURN( lpSection, lpDefault );
			for( xgc_size nIdx = 0; nIdx < lpSection->pair_count; ++nIdx )
			{
				if( strcmp( lpItemName, lpSection->pair[nIdx].key ) == 0 )
				{
					if( lpSection->pair[nIdx].trans_val )
						return trans_buffer + lpSection->pair[nIdx].trans_val;

					return lpSection->pair[nIdx].val;
				}
			}

			return lpDefault;
		}

		///
		/// \brief 获取数值类型的值
		/// [8/11/2014] create by albert.xu
		///
		xgc_bool ini_reader::get_item_value( xgc_lpcstr lpSection, xgc_lpcstr lpItem, xgc_bool bDefault ) const
		{
			xgc_lpcstr pValue = get_item_value( lpSection, lpItem, xgc_nullptr );
			if( pValue )
			{
				if( strcasecmp( "true", pValue ) == 0 )
					return true;
				else
					return atoi( pValue ) != 0;
			}

			return bDefault;
		}

		xgc_bool ini_reader::get_item_value( ini_reader::psection lpSection, xgc_lpcstr lpItem, xgc_bool bDefault ) const
		{
			xgc_lpcstr pValue = get_item_value( lpSection, lpItem, xgc_nullptr );
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
			// 缓冲结束位置
			xgc_lpstr lpCur = lpBuffer;
			xgc_lpstr lpEnd = lpBuffer + nBufferSize;

			// 记录点
			xgc_lpcstr lpMem = xgc_nullptr;
			xgc_string loMem[4];

			xgc_size nMem = 0;

			// 是否搜索中
			xgc_bool bSearch = false;
			// 开始复制
			while( *lpValue )
			{
				if( lpCur == lpEnd )
					return -1;

				if( bSearch )
				{
					// 正在搜索匹配的过程中发现，则认为已找到匹配项
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
						// 双百分号则认为是转义符
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
		/// 释放所有文件
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