#include <mbstring.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include "ini.h"

namespace XGC
{
	namespace common
	{
		xgc_bool IniFile::Load( xgc_lpcstr lpFilePath )
		{
			return Load( lpFilePath, "" );
		}

		///
		/// 加载Ini文件
		/// [8/11/2014] create by albert.xu
		///
		xgc_bool IniFile::Load( xgc_lpcstr lpFilePath, xgc_lpcstr lpFileName )
		{
			xgc_char szPathName[_MAX_PATH] = { 0 };
			sprintf_s( szPathName, "%s%s", lpFilePath, lpFileName );
			xgc_lpstr lpPathName = _fullpath( xgc_nullptr, szPathName, sizeof( szPathName ) );

			int fd = -1;
			_sopen_s( &fd, lpPathName, O_BINARY | O_RDONLY, SH_DENYWR, S_IREAD );
			if( fd == -1 )
			{
				DWORD dwErr = GetLastError();
				return false;
			}

			xgc_lpstr pFileBuffer = xgc_nullptr;
			struct _stat fst;
			_fstat( fd, &fst );

			xgc_size buffer_size = fst.st_size;
			xgc_lpstr buffer = (xgc_lpstr) realloc( pFileBuffer, fst.st_size + 1 );
			XGC_ASSERT_RETURN( buffer, false );

			pFileBuffer = buffer;
			int rd = _read( fd, buffer, (unsigned int) buffer_size );
			_close( fd );

			if( rd != buffer_size )
			{
				return false;
			}
			pFileBuffer[fst.st_size] = 0;

			FileInfo* pFileInfo = XGC_NEW FileInfo;
			pFileInfo->mFileBuffer = pFileBuffer;

			xgc_char drive[_MAX_DRIVE];
			xgc_char dir[_MAX_DIR];
			xgc_char fname[_MAX_FNAME];
			xgc_char ext[_MAX_EXT];

			_splitpath_s( lpPathName, drive, dir, fname, ext );

			sprintf_s( pFileInfo->mFilePath, "%s%s", drive, dir );
			sprintf_s( pFileInfo->mFileName, "%s%s", fname, ext );

			if( mFileRoot )
				mFileRoot->mIncludes.push_back( pFileInfo );
			else
				mFileRoot = pFileInfo;

			return AnalyzeFile( pFileInfo );
		}

		struct IniFile::KeyPair
		{
			xgc_lpstr	mKey;
			xgc_lpstr	mVal;

			xgc_size	mTransVal;
		};

		struct IniFile::Section
		{
			xgc_size	mNextSecion;
			xgc_lpcstr	mName;
			xgc_uint16	mPairCount;
			KeyPair		mPair[1];
		};

		xgc_bool IniFile::AnalyzeFile( FileInfo* pFileInfo )
		{
			xgc_lpstr pCur = pFileInfo->mFileBuffer;
			xgc_lpstr pMem = xgc_nullptr;

			enum LineType { eNewLine, eEndLine, eError, eCommit, eSection, ePairKey, ePairEqu, ePairValue, eSearchValue, eRealValue, eTransValue };
			LineType eType = eNewLine;

			xgc_size nMemSize = _msize( mSaveBuffer );
			while( pCur < pFileInfo->mFileBuffer + _msize( pFileInfo->mFileBuffer ) )
			{
				if( mSaveBuffer + nMemSize - (xgc_lpstr) pKeyPair < XGC_MAX( sizeof( Section ), sizeof( KeyPair ) ) )
				{
					xgc_lpstr pNew = (xgc_lpstr) realloc( mSaveBuffer, _msize( mSaveBuffer ) + 4096 );
					XGC_ASSERT_RETURN( pNew, false );

					memset( pNew + nMemSize, 0, _msize( pNew ) - nMemSize );
					nMemSize = _msize( pNew );

					pSection    = (Section*)( pNew + ( (xgc_lpstr) pSection - mSaveBuffer ) );
					pKeyPair    = (KeyPair*)( pNew + ( (xgc_lpstr) pKeyPair - mSaveBuffer ) );
					mSaveBuffer = pNew;
				}

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
						if( pKeyPair && ( isalpha( *pCur ) || _ismbslead( (xgc_byte*) pCur, (xgc_byte*) pCur ) ) )
						{
							pKeyPair->mKey = pCur;
							eType = ePairKey;
						}
						break;
					}
					break;
					case eSection:
					{
						if( *pCur == ']' )
						{
							*pCur = 0;
							eType = eNewLine;

							if( _strnicmp( "Extern:", pMem, 7 ) == 0 )
							{
								// 打开文件, 分析文件内容
								if( false == Load( pFileInfo->mFilePath, pMem + 7 ) )
									eType = eError;
							}
							else
							{
								if( pSection )
								{
									pSection->mNextSecion = (xgc_lpstr) pKeyPair - mSaveBuffer;
									pSection = (Section*) ( pKeyPair );
								}
								else
								{
									pSection = (Section*) mSaveBuffer;
								}

								pSection->mName = pMem;
								pKeyPair = pSection->mPair;

								++mSectionCount;
							}
						}
						else if( strchr( " \t\r\n", *pCur ) )
						{
							*pCur = 0;
							eType = eError;
							++mSectionCount;
						}
					}
					break;
					case ePairKey:
					{
						if( *pCur == '=' )
						{
							*pCur = 0;
							eType = ePairValue;
						}
						else if( strchr( " \t", *pCur ) )
						{
							*pCur = 0;
							eType = ePairEqu;
						}
						else if( strchr( "\r\n", *pCur ) )
						{
							*pCur = 0;

							pKeyPair->mVal = pCur;
							++pSection->mPairCount;
							++pKeyPair;

							eType = eNewLine;
						}
					}
					break;
					case ePairEqu:
					{
						if( *pCur == '=' )
						{
							eType = ePairValue;
						}
						else if( strchr( "\r\n", *pCur ) )
						{
							pKeyPair->mVal = pCur;
							++pSection->mPairCount;
							++pKeyPair;

							eType = eNewLine;
						}

						*pCur = 0;
					}
					break;
					case ePairValue:
					{
						if( isgraph( *pCur ) || _ismbslead( (xgc_byte*) pCur, (xgc_byte*) pCur ) )
						{
							pKeyPair->mVal = pCur;
							eType = eRealValue;
						}
						else if( strchr( "\r\n", *pCur ) )
						{
							pKeyPair->mVal = pCur;
							++pSection->mPairCount;
							++pKeyPair;

							*pCur = 0;
							eType = eNewLine;
						}
					}
					break;
					case eRealValue:
					{
						if( *pCur == '%' && *( pCur + 1 ) != '%' )
						{
							pKeyPair->mTransVal = mTransOffset;
							eType = eTransValue;
						}

						if( strchr( "\r\n", *pCur ) )
						{
							*pCur = 0;

							++pSection->mPairCount;
							++pKeyPair;

							eType = eNewLine;
						}
					}
					break;
					case eTransValue:
					{
						if( strchr( "\r\n", *pCur ) )
						{
							*pCur = 0;
							auto cpy = TransformValue( mTransBuffer + mTransOffset, _msize( mTransBuffer ) - mTransOffset, pKeyPair->mVal );
							while( cpy < 0 )
							{
								auto pNew = realloc( mTransBuffer, _msize( mTransBuffer ) + 4096 );
								if( pNew == xgc_nullptr )
									return false;
								
								mTransBuffer = (xgc_lpstr) pNew;

								cpy = TransformValue( mTransBuffer + mTransOffset, _msize( mTransBuffer ) - mTransOffset, pKeyPair->mVal );
							}

							// 增加结束符
							mTransOffset += cpy;
							XGC_ASSERT_RETURN( mTransOffset < _msize( mTransBuffer ), false );

							if( mTransOffset == _msize( mTransBuffer ) )
							{
								auto pNew = realloc( mTransBuffer, _msize( mTransBuffer ) + 4096 );
								if( pNew == xgc_nullptr )
									return false;

								mTransBuffer = (xgc_lpstr) pNew;
							}

							mTransBuffer[mTransOffset++] = 0;

							++pSection->mPairCount;
							++pKeyPair;

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
				++pCur;
			}

			return true;
		}

		const IniFile::Section* IniFile::FindSection( xgc_lpcstr lpSection )const
		{
			const Section* pSection = (Section*) mSaveBuffer;
			if( mSectionCount ) do
			{
				if( strcmp( lpSection, pSection->mName ) == 0 )
					return pSection;

				pSection = (const Section*) ( mSaveBuffer + pSection->mNextSecion );
			} while( pSection != (const Section*) mSaveBuffer );

			return xgc_nullptr;
		}

		xgc_bool IniFile::IsExistSection( xgc_lpcstr lpSection )const
		{
			return xgc_nullptr != FindSection( lpSection );
		}

		xgc_bool IniFile::IsExistItem( xgc_lpcstr lpSection, xgc_lpcstr lpItem )const
		{
			return xgc_nullptr != GetItemValue( lpSection, lpItem, xgc_nullptr );
		}

		xgc_size IniFile::GetItemCount( xgc_lpcstr lpSection )const
		{
			const Section* pSection = FindSection( lpSection );
			if( xgc_nullptr == pSection )
				return 0;

			return pSection->mPairCount;
		}

		xgc_lpcstr IniFile::GetSectionName( xgc_size nSectionIdx )const
		{
			if( nSectionIdx >= mSectionCount )
				return xgc_nullptr;

			const Section* pSection = (Section*) mSaveBuffer;
			do
			{
				if( nSectionIdx-- == 0 )
					return pSection->mName;

				pSection = (const Section*) ( mSaveBuffer + pSection->mNextSecion );
			} while( pSection != (const Section*) mSaveBuffer );

			return xgc_nullptr;
		}

		xgc_lpcstr IniFile::GetItemName( xgc_lpcstr lpSection, xgc_size nIdx )const
		{
			const Section* pSection = FindSection( lpSection );
			if( xgc_nullptr == pSection )
				return xgc_nullptr;

			if( nIdx >= pSection->mPairCount )
				return xgc_nullptr;

			return pSection->mPair[nIdx].mKey;
		}

		xgc_lpcstr IniFile::GetItemValue( xgc_lpcstr lpSection, xgc_size nIdx, xgc_lpcstr lpDefault )const
		{
			const Section* pSection = FindSection( lpSection );
			if( xgc_nullptr == pSection )
				return lpDefault;

			if( nIdx >= pSection->mPairCount )
				return lpDefault;

			if( pSection->mPair[nIdx].mTransVal )
				return mTransBuffer + pSection->mPair[nIdx].mTransVal;

			return pSection->mPair[nIdx].mVal;
		}

		xgc_lpcstr IniFile::GetItemValue( xgc_lpcstr lpSection, xgc_lpcstr lpName, xgc_lpcstr lpDefault )const
		{
			const Section* pSection = FindSection( lpSection );
			if( xgc_nullptr == pSection )
				return lpDefault;

			for( xgc_size nIdx = 0; nIdx < pSection->mPairCount; ++nIdx )
			{
				if( strcmp( lpName, pSection->mPair[nIdx].mKey ) == 0 )
				{
					if( pSection->mPair[nIdx].mTransVal )
						return mTransBuffer + pSection->mPair[nIdx].mTransVal;

					return pSection->mPair[nIdx].mVal;
				}
			}

			return lpDefault;
		}

		xgc_long IniFile::TransformValue( xgc_lpstr lpBuffer, xgc_size nBufferSize, xgc_lpcstr lpValue )
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

						xgc_lpcstr lpItem = GetItemValue( loMem[0].c_str(), loMem[1].c_str(), "" );
						int cpy = sprintf_s( lpCur, lpEnd - lpCur, "%s", lpItem );
						if( cpy > 0 )
							lpCur += cpy;

						bSearch = false;
					}
					else if( *lpValue == '.' && nMem < xgc_countof( loMem ) )
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

		IniFile::IniFile()
			: mFileRoot( xgc_nullptr )
			, mSaveBuffer( (xgc_lpstr) malloc( 4096 ) )
			, pSection( xgc_nullptr )
			, pKeyPair( xgc_nullptr )
			, mTransBuffer( (xgc_lpstr) malloc( 4096 ) )
			, mTransOffset( 4 )
			, mSectionCount( 0 )
		{
			memset( mSaveBuffer, 0, _msize( mSaveBuffer ) );
			memset( mTransBuffer, 0, _msize( mTransBuffer ) );
		}

		IniFile::IniFile( xgc_lpcstr lpFilePath ) 
			: IniFile()
		{
			Load( lpFilePath );
		}

		IniFile::~IniFile()
		{
			Free( mFileRoot );
			free( mSaveBuffer );
			free( mTransBuffer );
		}

		///
		/// 释放所有文件
		/// [7/16/2015] create by albert.xu
		///
		xgc_void IniFile::Free( FileInfo* &pFileInfo )
		{
			if( pFileInfo == xgc_nullptr )
				return;

			for( auto it = pFileInfo->mIncludes.begin(); it != pFileInfo->mIncludes.end(); ++it )
			{
				Free( (*it) );
			}

			pFileInfo = xgc_nullptr;
		}

	}
}