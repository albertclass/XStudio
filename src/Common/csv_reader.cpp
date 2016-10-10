#include "csv_reader.h"
#include "encoding.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#if defined( _WINDOWS )
#include <io.h>
#elif defined( __GNUC__ )
#include <malloc.h>
#include <unistd.h>
#endif

namespace xgc
{
	namespace common
	{
		csv_reader::csv_reader()
			: buffer( xgc_nullptr )
			, rows( 0 )
			, cols( 0 )
			, fetch( 0 )
		{
			memset( chunk, 0, sizeof( chunk ) );
		}

		csv_reader::~csv_reader()
		{
			free( buffer );
			for( auto pChunk : chunk )
				free( pChunk );
		}

		xgc_lpcstr csv_reader::get_cell( xgc_size row, xgc_size col ) const throw()
		{
			if( col >= cols ) return xgc_nullptr;

			xgc_size idx = ( row >> 12 );
			if( idx < XGC_COUNTOF( chunk ) )
			{
				xgc_lpvoid chk = chunk[idx];
				if( chk )
				{
					xgc_size offset = ( row & 0x0fff );
					xgc_lpstr* cell = (xgc_lpstr*) chk;

					return cell[offset*cols + col];
				}
			}

			return xgc_nullptr;
		}

		xgc_lpcstr csv_reader::set_cell( xgc_size nRow, xgc_size nCol, xgc_lpstr pStr )
		{
			XGC_ASSERT_RETURN( nCol < cols, xgc_nullptr );

			xgc_size idx = ( nRow >> 12 ); // 右移12位 每过4096,重新分配一次内存
			XGC_ASSERT_RETURN( idx < XGC_COUNTOF( chunk ), xgc_nullptr );

			xgc_lpvoid chk = chunk[idx];
			if( chk == xgc_nullptr )
			{
				// 一次分配4096行
				chunk[idx] = chk = (xgc_lpvoid) malloc( 0x1000 * sizeof( xgc_lpvoid ) * cols );
				if( chk == xgc_nullptr )
					return xgc_nullptr;
				#if defined(__GNUC__)
				memset( chk, 0, malloc_usable_size( chk ) );
				#elif defined(_WINDOWS)
				memset( chk, 0, _msize( chk ) );
				#endif
			}

			xgc_size offset = ( nRow & 0x0fff );
			xgc_lpstr* cell = (xgc_lpstr*) chk;

			cell[offset*cols + nCol] = pStr;
			return cell[offset*cols + nCol];
		}

		xgc_int32 csv_reader::get_col( xgc_lpcstr title ) const throw()
		{
			for( xgc_size i = 0; this->title && i < cols; ++i )
			{
				xgc_lpcstr str = get_cell( 0, fetch % cols );
				if( str && strcasecmp( title, str ) == 0 )
					return fetch++ % cols;
				++fetch;
			}

			return cols;
		}

		xgc_bool csv_reader::parse( xgc_lpstr buffer, xgc_size buffer_size, xgc_char split )
		{
			xgc_bool	is_string = false;
			xgc_size&	n = rows; // 行号
			xgc_size	c = 0; // 当前是第几列
			xgc_size	i = 0; // 读取位置

			set_cell( n, c, buffer );
			while( i < buffer_size )
			{
				if( buffer[i] == split )
				{
					if( !is_string && i > 0 )
					{
						buffer[i] = 0;
						set_cell( n, ++c, buffer + i + 1 );
					}
				}
				else switch( buffer[i] )
				{
				case 0:
					return true;
				case '\r':
					buffer[i] = 0;
					break;
				case '\n':
					if( !is_string && i > 0 )
					{
						c = 0;

						buffer[i] = 0;
						set_cell( ++n, c, buffer + i + 1 );
					}
					break;
				case '"':
					if( !is_string )
						set_cell( n, c, buffer + i + 1 );

					buffer[i] = 0;
					is_string = !is_string;
					break;
				}

				if( c >= cols )
					return false;
				++i;
			}

			// 字符串未完结,则认为错误
			if( is_string )
				return false;

			if( c + 1 == cols )
				++n;

			return true;
		}

		xgc_bool csv_reader::load( xgc_lpcstr pathname, xgc_char split, xgc_bool has_title )
		{
			// 重入清理
			cols = 0;
			rows = 0;

			title = has_title;

			free( buffer );
			buffer = xgc_nullptr;

			for( auto i = 0; i < XGC_COUNTOF( chunk ); ++i )
			{
				free( chunk[i] );
				chunk[i] = xgc_nullptr;
			}

			// 读取文件数据
			int fd = -1;
			#if defined(_WINDOWS)
			_sopen_s( &fd, pathname, O_RDONLY | O_BINARY, SH_DENYWR, S_IREAD );
			#elif defined( __GNUC__ )
			fd = _open( pathname, O_RDONLY, S_IREAD );
			#endif

			XGC_ASSERT_RETURN( fd != -1, false );

			struct _stat fst;
			_fstat( fd, &fst );

			xgc_size buffer_size = fst.st_size;
			xgc_lpstr palloc = (xgc_lpstr) realloc( buffer, fst.st_size + 1 );
			XGC_ASSERT_RETURN( palloc, false );

			buffer = palloc;
			int rd = _read( fd, buffer, (unsigned int)buffer_size );
			_close( fd );

			// 文件读取字节数不正确则返回错误
			if( rd != buffer_size )
				return false;

			// 将最后一个字节封闭
			buffer[fst.st_size] = 0;

			// 根据文件猜测编码方式
			encoding enc = guess_encoding( (xgc_lpvoid*)&buffer, buffer_size );

			// 对UTF-8的编码进行转换
			if( enc == encoding_utf8 )
			{
				// 计算转换后需要的内存
				auto len = utf8tombs( buffer, xgc_nullptr, 0 );
				if( len == -1 )
					return false;

				// 转换编码
				auto ptr = (xgc_lpstr) malloc( len + 1 );
				utf8tombs( buffer, ptr, len );

				buffer = ptr;
				buffer_size = len;

				buffer[buffer_size] = 0;
				free( palloc );
			}

			// 根据第一行计算列数
			for( xgc_size i = 0; i < buffer_size; ++i )
			{
				if( buffer[i] == split )
				{
					++cols;
				}

				if( buffer[i] == '\n' )
				{
					++cols;
					break;
				}
			}

			// 解析
			parse( buffer, buffer_size, split );
			return true;
		}

		///
		/// \brief 读取单元格数据
		/// \param row 行号
		/// \param col 列号
		///

		inline xgc_lpcstr csv_reader::get_value( xgc_size row, xgc_size col, xgc_lpcstr value ) const throw()
		{
			xgc_lpcstr val = get_cell( title ? row + 1 : row, col );
			if( val )
				return val;

			return value;
		}

		///
		/// 读取单元格数据
		/// \param row 行号
		/// \param title 表头
		///

		inline xgc_lpcstr csv_reader::get_value( xgc_size row, xgc_lpcstr title, xgc_lpcstr value ) const throw()
		{
			XGC_ASSERT_RETURN( title, xgc_nullptr );
			return get_value( row, get_col( title ), value );
		}


		xgc_bool csv_reader::get_value( xgc_size row, xgc_size col, xgc_bool value ) const throw()
		{
			xgc_lpcstr val = get_cell( title ? row + 1 : row, col );
			if( xgc_nullptr == val )
				return value;

			if( strcasecmp( val, "0" ) == 0 )
				return false;
			else if( strcasecmp( val, "1" ) == 0 )
				return true;
			else if( strcasecmp( val, "false" ) == 0 )
				return false;
			else if( strcasecmp( val, "true" ) == 0 )
				return true;
			else
				return value;
		}

	}
}

