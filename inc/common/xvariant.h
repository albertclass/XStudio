#ifndef __VARIANT_H__
#define __VARIANT_H__

#include "xutility.h"
namespace xgc
{
#ifdef _WIN64
#	define XVARIANT_MAKE( HI, LO )	(( (HI) << 32 ) | (LO))
#	define XVARIANT_HIPART( X )	( X >> 32 )
#	define XVARIANT_LOPART( X )	( X & 0xFFFFFFFF )
#else
#	define XVARIANT_MAKE( HI, LO )	(( (HI) << 16 ) | (LO))
#	define XVARIANT_HIPART( X )	( X >> 16 )
#	define XVARIANT_LOPART( X )	( X & 0xFFFF )
#endif

	struct COMMON_API xvariant
	{
	public:
		enum value_type
		{
			vt_void = 0,
			vt_bool,
			vt_char,
			vt_byte,
			vt_i16,
			vt_i32,
			vt_i64,
			vt_u16,
			vt_u32,
			vt_u64,
			vt_real,
			vt_real64,
			vt_string,
			vt_buffer,
		};

		typedef xgc_void	Void;
		typedef xgc_bool	Bool;
		typedef xgc_int8	Char;
		typedef xgc_uint8	Byte;
		typedef xgc_int16	Short;
		typedef xgc_uint16	UShort;
		typedef xgc_int32	Integer;
		typedef xgc_uint32	Unsigned;
		typedef xgc_int64	Long;
		typedef xgc_uint64	ULong;
		typedef xgc_real32	Real32;
		typedef xgc_real64	Real64;

		typedef struct
		{
			xgc_uint16	alc;	// 容量
			xgc_uint16	len;	// 串长度
			xgc_char	str[1];	// 串首址
		}String, *StringPtr;

		typedef struct
		{
			xgc_uint16	alc;	// 容量
			xgc_uint16	len;	// 缓冲长度
			xgc_byte	buf[1];	// 缓冲首址
		}Buffer, *BufferPtr;

		template< typename T >
		struct type2value;

		template< xvariant::value_type T >
		struct value2type;

		template< >
		struct type2value < Void >
		{
			enum { Value = vt_void };
		};

		template< >
		struct type2value < Bool >
		{
			enum { Value = vt_bool };
		};

		template< >
		struct type2value < Char >
		{
			enum { Value = vt_char };
		};

		template< >
		struct type2value < Byte >
		{
			enum { Value = vt_byte };
		};

		template< >
		struct type2value < Short >
		{
			enum { Value = vt_i16 };
		};

		template< >
		struct type2value < UShort >
		{
			enum { Value = vt_u16 };
		};

		template< >
		struct type2value < Integer >
		{
			enum { Value = vt_i32 };
		};

		template< >
		struct type2value < Unsigned >
		{
			enum { Value = vt_u32 };
		};

		template< >
		struct type2value < Long >
		{
			enum { Value = vt_i64 };
		};

		template< >
		struct type2value < ULong >
		{
			enum { Value = vt_u64 };
		};

		template< >
		struct type2value < Real32 >
		{
			enum { Value = vt_real };
		};

		template< >
		struct type2value < Real64 >
		{
			enum { Value = vt_real64 };
		};


		template< >
		struct type2value < StringPtr >
		{
			enum { Value = vt_string };
		};


		template< >
		struct type2value < BufferPtr >
		{
			enum { Value = vt_buffer };
		};

		//////////////////////////////////////////////////////////////////////////
		template< >
		struct value2type < vt_void >
		{
			typedef Void type;
		};

		template< >
		struct value2type < vt_bool >
		{
			typedef Bool type;
		};

		template< >
		struct value2type < vt_char >
		{
			typedef Char type;
		};

		template< >
		struct value2type < vt_byte >
		{
			typedef Byte type;
		};

		template< >
		struct value2type < vt_i16 >
		{
			typedef Short type;
		};

		template< >
		struct value2type < vt_u16 >
		{
			typedef UShort type;
		};

		template< >
		struct value2type < vt_i32 >
		{
			typedef Integer type;
		};

		template< >
		struct value2type < vt_u32 >
		{
			typedef Unsigned type;
		};

		template< >
		struct value2type < vt_i64 >
		{
			typedef Long type;
		};

		template< >
		struct value2type < vt_u64 >
		{
			typedef ULong type;
		};

		template< >
		struct value2type < vt_real >
		{
			typedef Real32 type;
		};

		template< >
		struct value2type < vt_real64 >
		{
			typedef Real64 type;
		};

		template< >
		struct value2type < vt_string >
		{
			typedef StringPtr type;
		};

		template< >
		struct value2type < vt_buffer >
		{
			typedef BufferPtr type;
		};

	private:
		value_type t;

		union
		{
			Void*		mVoid;
			Bool		mBool;
			Char		mChar;
			Byte		mByte;
			Short		mShort;
			UShort		mUShort;
			Integer		mInteger;
			Unsigned	mUnsigned;
			Long		mLong;
			ULong		mULong;
			Real32		mReal32;
			Real64		mReal64;
			StringPtr	mString;
			BufferPtr	mBuffer;
		};

	public:
		///
		/// \brief 默认构造
		/// \author albert.xu
		/// \date 2017/09/08
		///
		xvariant();

		///
		/// \brief 拷贝构造
		/// \author albert.xu
		/// \date 2017/09/08
		///
		xvariant( const xvariant& _Val );

		///
		/// \brief 右值构造
		/// \author albert.xu
		/// \date 2017/09/08
		///
		xvariant( xvariant&& _Val );

		///
		/// \brief 字符串构造
		/// \author albert.xu
		/// \date 2017/09/08
		///
		xvariant( xgc_lpcstr s )
			: xvariant()
		{
			set_string( s );
		}

		///
		/// \brief 缓冲区构造
		/// \author albert.xu
		/// \date 2017/09/08
		///
		xvariant( xgc_lpvoid d, xgc_size s )
			: xvariant()
		{
			set_buffer( d, s );
		}

		///
		/// \brief 值构造
		/// \author albert.xu
		/// \date 2017/09/08
		///
		template< class _Ty, typename std::enable_if< is_numeric< _Ty >::value, int >::type = 0 >
		xvariant( _Ty v )
		{
			set_value( v );
		}

		///
		/// 删除委托的对象，需要显示调用
		/// [9/9/2014] create by albert.xu
		///
		~xvariant();

		///
		/// \brief 获取类型字符串
		/// \author albert.xu
		/// \date 2017/09/08
		///
		static xgc_lpcstr type2string( value_type t );

		///
		/// \brief 获取类型所需尺寸
		/// \author albert.xu
		/// \date 2017/09/08
		///
		static xgc_size type2size( value_type t );

		///
		/// \brief 获取数据原始类型
		/// \author albert.xu
		/// \date 2017/09/08
		///
		value_type type()const
		{
			return t;
		}

		///
		/// \brief 获取类型所需尺寸
		/// \author albert.xu
		/// \date 2017/09/08
		///
		xgc_size type2size()const;

		///
		/// \brief 是否有符号数
		/// \author albert.xu
		/// \date 2017/09/08
		///
		xgc_bool is_integer()const;

		///
		/// \brief 是否无符号数
		/// \author albert.xu
		/// \date 2017/09/08
		///
		xgc_bool is_unsigned()const;

		///
		/// \brief 是否实数
		/// \author albert.xu
		/// \date 2017/09/08
		///
		xgc_bool is_real()const;

		///
		/// [1/9/2014 albert.xu]
		/// 是否数值类型
		///
		xgc_bool is_value()const
		{
			return is_integer() || is_unsigned() || is_real();
		}

		///
		/// 是否是bool类型
		/// [12/24/2014] create by jianglei.kinly
		///
		xgc_bool is_bool()const
		{
			return type() == vt_bool;
		}

		///
		/// [1/9/2014 albert.xu]
		/// 是否字符串
		///
		xgc_bool is_string()const
		{
			return type() == vt_string;
		}

		///
		/// [1/9/2014 albert.xu]
		/// 是否缓冲
		///
		xgc_bool is_buffer()const
		{
			return type() == vt_buffer;
		}

		///
		/// [1/9/2014 albert.xu]
		/// 是否
		///
		xgc_bool valid()const { return type() != vt_void; }

		///
		/// [1/8/2014 albert.xu]
		/// 整数赋值
		///
		template< class V, typename std::enable_if< is_numeric< V >::value, int >::type = 0 >
		xgc_void set_value( V _Val )
		{
			XGC_ASSERT_RETURN( is_value(), xgc_void( 0 ) );
			switch( type() )
			{
				case vt_char:
				mChar = ( Char ) _Val;
				break;
				case vt_byte:
				mByte = ( Byte ) _Val;
				break;
				case vt_i16:
				mShort = ( Short )_Val;
				break;
				case vt_i32:
				mInteger = ( Integer )_Val;
				break;
				case vt_i64:
				mLong = ( Long )_Val;
				break;

				case vt_u16:
				mUShort = ( UShort )_Val;
				break;
				case vt_u32:
				mUnsigned = ( Unsigned )_Val;
				break;
				case vt_u64:
				mULong = ( ULong )_Val;
				break;

				case vt_real:
				mReal32 = ( Real32 )_Val;
				break;
				default:
				XGC_DEBUG_MESSAGE( "无效的类型值" );
				break;
			}
		}

		///
		/// 设置bool类型
		/// [12/24/2014] create by jianglei.kinly
		///
		xgc_void set_value( xgc_bool _Val )
		{
			mBool = _Val;
		}

		///
		/// [1/8/2014 albert.xu]
		/// 字符串赋值
		///
		xgc_void set_string( xgc_lpcstr _Val, xgc_size _Length = -1 );

		///
		/// [1/8/2014 albert.xu]
		/// 缓冲区赋值
		///
		xgc_void set_buffer( xgc_lpvoid _Val, xgc_size _Length );

		///
		/// [3/18/2014 albert.xu]
		/// 设置缓冲区长度
		///
		xgc_bool set_buffer_len( xgc_size _Len, xgc_byte _Fill = 0 );

		///
		/// 获取Buffer长度
		/// [8/22/2014] create by albert.xu
		///
		xgc_size get_buffer_len()const;

		///
		/// [1/8/2014 albert.xu]
		/// 复制赋值
		///
		xgc_void assign( const xvariant& _Val );

		///
		/// [1/8/2014 albert.xu]
		/// 值操作
		///
		template< class V, typename std::enable_if< is_numeric< V >::value, xgc_bool >::type = true >
		xgc_void multiplies( V _Val )
		{
			switch( type() )
			{
				case vt_char:
				mChar = Char( mChar * _Val );
				break;
				case vt_i16:
				mShort = Short( mShort * _Val );
				break;
				case vt_i32:
				mInteger = Integer( mInteger * _Val );
				break;
				case vt_i64:
				mLong = Long( mLong * _Val );
				break;
				case vt_byte:
				mByte = Byte( mByte * _Val );
				break;
				case vt_u16:
				mUShort = UShort( mUShort * _Val );
				break;
				case vt_u32:
				mUnsigned = Unsigned( mUnsigned * _Val );
				break;
				case vt_u64:
				mULong = ULong( mULong * _Val );
				break;
				case vt_real:
				mReal32 = Real32( mReal32 * _Val );
				break;
				case vt_real64:
				mReal64 = Real64( mReal32 * _Val );
				break;
				default:
				XGC_DEBUG_MESSAGE( "非数值类型使用了数值运算!" );
				break;
			}
		}

		template< class V, typename std::enable_if< is_numeric< V >::value, xgc_bool >::type = true >
		xgc_void division( V _Val )
		{
			switch( type() )
			{
				case vt_char:
				mChar = Char( mChar / _Val );
				break;
				case vt_i16:
				mShort = Short( mShort / _Val );
				break;
				case vt_i32:
				mInteger = Integer( mInteger / _Val );
				break;
				case vt_i64:
				mLong = Long( mLong / _Val );
				break;
				case vt_byte:
				mByte = Byte( mByte / _Val );
				break;
				case vt_u16:
				mUShort = UShort( mUShort / _Val );
				break;
				case vt_u32:
				mUnsigned = Unsigned( mUnsigned / _Val );
				break;
				case vt_u64:
				mULong = ULong( mULong / _Val );
				break;
				case vt_real:
				mReal32 = Real32( mReal32 / _Val );
				break;
				case vt_real64:
				mReal64 = Real64( mReal32 / _Val );
				break;
				default:
				XGC_DEBUG_MESSAGE( "非数值类型使用了数值运算!" );
				break;
			}
		}

		template< class V, typename std::enable_if< is_numeric< V >::value, xgc_bool >::type = true >
		xgc_void plus( V _Val )
		{
			switch( type() )
			{
				case vt_char:
				mChar = Char( mChar + _Val );
				break;
				case vt_i16:
				mShort = Short( mShort + _Val );
				break;
				case vt_i32:
				mInteger = Integer( mInteger + _Val );
				break;
				case vt_i64:
				mLong = Long( mLong + _Val );
				break;
				case vt_byte:
				mByte = Byte( mByte + _Val );
				break;
				case vt_u16:
				mUShort = UShort( mUShort + _Val );
				break;
				case vt_u32:
				mUnsigned = Unsigned( mUnsigned + _Val );
				break;
				case vt_u64:
				mULong = ULong( mULong + _Val );
				break;
				case vt_real:
				mReal32 = Real32( mReal32 + _Val );
				break;
				case vt_real64:
				mReal64 = Real64( mReal32 + _Val );
				break;
				default:
				XGC_DEBUG_MESSAGE( "非数值类型使用了数值运算!" );
				break;
			}
		}

		template< class V, typename std::enable_if< is_numeric< V >::value, xgc_bool >::type = true >
		xgc_void minus( V _Val )
		{
			switch( type() )
			{
				case vt_char:
				mChar = Char( mChar - _Val );
				break;
				case vt_i16:
				mShort = Short( mShort - _Val );
				break;
				case vt_i32:
				mInteger = Integer( mInteger - _Val );
				break;
				case vt_i64:
				mLong = Long( mLong - _Val );
				break;
				case vt_byte:
				mByte = Byte( mByte - _Val );
				break;
				case vt_u16:
				mUShort = UShort( mUShort - _Val );
				break;
				case vt_u32:
				mUnsigned = Unsigned( mUnsigned - _Val );
				break;
				case vt_u64:
				mULong = ULong( mULong - _Val );
				break;
				case vt_real:
				mReal32 = Real32( mReal32 - _Val );
				break;
				case vt_real64:
				mReal64 = Real64( mReal32 - _Val );
				break;
				default:
				XGC_DEBUG_MESSAGE( "非数值类型使用了数值运算!" );
				break;
			}
		}

		///
		/// [1/9/2014 albert.xu]
		/// 数值 乘法
		///
		xgc_void multiplies( const xvariant& _Val );

		///
		/// [1/9/2014 albert.xu]
		/// 数值 除法
		///
		xgc_void division( const xvariant& _Val );

		///
		/// [1/9/2014 albert.xu]
		/// 数值 加法
		///
		xgc_void plus( const xvariant& _Val );

		///
		/// [1/9/2014 albert.xu]
		/// 数值 减法
		///
		xgc_void minus( const xvariant& _Val );

		//////////////////////////////////////////////////////////////////////////
		// Limit
		//////////////////////////////////////////////////////////////////////////
		template< class VMin, class VMax, typename std::enable_if< is_numeric< VMin >::value && is_numeric< VMax >::value, xgc_bool >::type = true >
		XGC_INLINE xgc_void normalization( VMin _Min, VMax _Max )
		{
			switch( type() )
			{
				case vt_char:
				{
					if( mChar > (Char)_Max )
						mChar = (Char)_Max;
					else if( mChar < (Char)_Min )
						mChar = (Char)_Min;
				}
				break;
				case vt_byte:
				{
					if( mByte > ( Byte )_Max )
						mByte = (Byte)_Max;
					else if( mByte < (Byte)_Min )
						mByte = (Byte)_Min;
				}
				break;
				case vt_i16:
				{
					if( mShort > ( Short )_Max )
						mShort = (Short)_Max;
					else if( mShort < (Short)_Min )
						mShort = (Short)_Min;
				}
				break;
				case vt_u16:
				{
					if( mUShort > ( UShort )_Max )
						mUShort = (UShort)_Max;
					else if( mUShort < (UShort)_Min )
						mUShort = (UShort)_Min;
				}
				break;
				case vt_i32:
				{
					if( mInteger > ( Integer )_Max )
						mInteger = (Integer)_Max;
					else if( mInteger < (Integer)_Min )
						mInteger = (Integer)_Min;
				}
				break;
				case vt_u32:
				if( mUnsigned >( Unsigned )_Max )
					mUnsigned = ( Unsigned )_Max;
				else if( mUnsigned < ( Unsigned )_Min )
					mUnsigned = ( Unsigned )_Min;
				break;
				case vt_i64:
				if( mLong >( Long )_Max )
					mLong = ( Long )_Max;
				else if( mLong < ( Long )_Min )
					mLong = ( Long )_Min;
				break;
				case vt_u64:
				if( mULong >( ULong )_Max )
					mULong = ( ULong )_Max;
				else if( mULong < ( ULong )_Min )
					mULong = ( ULong )_Min;
				break;
				case vt_real:
				if( mReal32 >( Real32 )_Max )
					mReal32 = ( Real32 )_Max;
				else if( mReal32 < ( Real32 )_Min )
					mReal32 = ( Real32 )_Min;
				break;
				default:
				XGC_DEBUG_MESSAGE( "非数值类型使用了数值运算!" );
				break;
			}
		}

		///
		/// [1/9/2014 albert.xu]
		/// 转为整型取值
		///
		XGC_INLINE Integer to_integer()const;

		///
		/// [1/26/2014 albert.xu]
		/// 转为无符号整型
		///
		XGC_INLINE Unsigned to_unsigned()const;

		///
		/// [1/9/2014 albert.xu]
		/// 转为整型取值
		///
		XGC_INLINE Long to_integer64()const;

		///
		/// [1/26/2014 albert.xu]
		/// 转为无符号整型
		///
		XGC_INLINE ULong to_unsigned64()const;

		///
		/// [1/9/2014 albert.xu]
		/// 转为浮点取值
		///
		XGC_INLINE Real32 to_real32()const;

		///
		/// [1/9/2014 albert.xu]
		/// 转为浮点取值
		///
		XGC_INLINE Real64 to_real64()const;

		///
		/// 转为Bool类型
		/// [12/24/2014] create by jianglei.kinly
		///
		XGC_INLINE Bool toBool()const
		{
			if( type() == vt_bool )
				return mBool;
			return false;
		}

		///
		/// 转换为数值
		/// [9/9/2014] create by albert.xu
		///
		template< class T >
		XGC_INLINE T to_numeric()const
		{
			switch( type() )
			{
				case vt_char:
				return (T)mChar;
				break;
				case vt_byte:
				return (T)mByte;
				break;
				case vt_i16:
				return (T)mShort;
				break;
				case vt_u16:
				return (T)mUShort;
				break;
				case vt_i32:
				return (T)mInteger;
				break;
				case vt_u32:
				return (T)mUnsigned;
				break;
				case vt_i64:
				return (T)mLong;
				break;
				case vt_u64:
				return (T)mULong;
				break;
				case vt_real:
				return (T)mReal32;
				break;
				case vt_string:
				return mString ? str2numeric< T >( (xgc_lpcstr)mString->str, xgc_nullptr ) : 0;
				break;
				case vt_buffer:
				return mBuffer ? str2numeric< T >( (xgc_lpcstr)mBuffer->buf, xgc_nullptr ) : 0;
				break;
				default:
				XGC_DEBUG_MESSAGE( "未知的属性类型！" );
				break;
			}

			return 0;
		}

		///
		/// \brief 转为字符串
		/// \author albert.xu
		/// \date 1/9/2014
		///
		xgc_string to_string()const;

		///
		/// \brief 转为字符串
		/// \author albert.xu
		/// \date 2017/09/08
		///
		xgc_long to_string( xgc_lpvoid buf, xgc_size len )const;

		///
		/// \brief 获取原始的字符串
		/// \author albert.xu
		/// \date 2017/09/08
		///
		xgc_lpcstr get_string( size_t *len = xgc_nullptr )const;

		///
		/// \brief 获取原始的缓冲区
		/// \author albert.xu
		/// \date 2017/09/08
		///
		xgc_lpvoid get_buffer( size_t *len = xgc_nullptr )const;

		///
		/// [1/17/2014 albert.xu]
		/// 设置缓冲区位
		///
		xgc_void set_bit( xgc_size idx );

		///
		/// [1/17/2014 albert.xu]
		/// 清除缓冲区位
		///
		xgc_void clr_bit( xgc_size idx );

		///
		/// [1/17/2014 albert.xu]
		/// 获取缓冲区位
		///
		xgc_bool get_bit( xgc_size idx, xgc_bool v )const;

		///
		/// [1/17/2014 albert.xu]
		/// 获取缓冲区位
		///
		xgc_bool cmp_bit( xgc_size idx, xgc_bool v )const;

		///
		/// [1/8/2014 albert.xu]
		/// 赋值操作符
		///
		xvariant& operator = ( const xvariant& _Val )
		{
			assign( _Val );
			return *this;
		}

		///
		/// \brief 右值构造
		/// \author albert.xu
		/// \date 2017/09/08
		///
		xvariant& operator=( xvariant&& _Val )
		{
			std::swap( t, _Val.t );
			std::swap( mVoid, _Val.mVoid );

			return *this;
		}

		template< class V, typename std::enable_if< is_numeric< V >::value, xgc_bool >::type = true >
		xvariant& operator = ( V _Val )
		{
			set_value( _Val );
			return *this;
		}

		xvariant& operator = ( xgc_lpcstr _Val )
		{
			set_string( _Val );
			return *this;
		}

		///
		/// 算数运算符重载
		/// [9/8/2014] create by albert.xu
		///
		template< class V, typename std::enable_if< is_numeric< V >::value || std::is_base_of< xvariant, V >::value, xgc_bool >::type = true >
		xvariant& operator += ( V _Val ) { plus( _Val ); return *this; }
		template< class V, typename std::enable_if< is_numeric< V >::value || std::is_base_of< xvariant, V >::value, xgc_bool >::type = true >
		xvariant& operator -= ( V _Val ) { minus( _Val ); return *this; }
		template< class V, typename std::enable_if< is_numeric< V >::value || std::is_base_of< xvariant, V >::value, xgc_bool >::type = true >
		xvariant& operator *= ( V _Val ) { multiplies( _Val ); return *this; }
		template< class V, typename std::enable_if< is_numeric< V >::value || std::is_base_of< xvariant, V >::value, xgc_bool >::type = true >
		xvariant& operator /= ( V _Val ) { division( _Val ); return *this; }
	};

	///
	/// [1/9/2014 albert.xu]
	/// 转为整型取值
	///
	XGC_INLINE xvariant::Integer xvariant::to_integer()const
	{
		return to_numeric< xvariant::Integer >();
	}

	///
	/// [1/26/2014 albert.xu]
	/// 转为无符号整型
	///
	XGC_INLINE xvariant::Unsigned xvariant::to_unsigned()const
	{
		return to_numeric< xvariant::Unsigned >();
	}

	///
	/// [1/9/2014 albert.xu]
	/// 转为整型取值
	///
	XGC_INLINE xvariant::Long xvariant::to_integer64()const
	{
		return to_numeric< xvariant::Long >();
	}

	///
	/// [1/26/2014 albert.xu]
	/// 转为无符号整型
	///
	XGC_INLINE xvariant::ULong xvariant::to_unsigned64()const
	{
		return to_numeric< xvariant::ULong >();
	}

	///
	/// [1/9/2014 albert.xu]
	/// 转为浮点取值
	///
	XGC_INLINE xvariant::Real32 xvariant::to_real32()const
	{
		return to_numeric< xvariant::Real32 >();
	}

	///
	/// [1/9/2014 albert.xu]
	/// 转为浮点取值
	///
	XGC_INLINE xvariant::Real64 xvariant::to_real64()const
	{
		return to_numeric< xvariant::Real64 >();
	}

	template< class V, typename std::enable_if< is_numeric< V >::value || std::is_base_of< xvariant, V >::value, xgc_bool >::type = true >
	XGC_INLINE xvariant operator + ( const xvariant& _Src, V _Val )
	{
		xvariant _RetValue( _Src );
		_RetValue.plus( _Val );
		return _RetValue;
	}

	template< class V, typename std::enable_if< is_numeric< V >::value || std::is_base_of< xvariant, V >::value, xgc_bool >::type = true >
	XGC_INLINE xvariant operator - ( const xvariant& _Src, V _Val )
	{
		xvariant _RetValue( _Src );
		_RetValue.minus( _Val );
		return _RetValue;
	}

	template< class V, typename std::enable_if< is_numeric< V >::value || std::is_base_of< xvariant, V >::value, xgc_bool >::type = true >
	XGC_INLINE xvariant operator * ( const xvariant& _Src, V _Val )
	{
		xvariant _RetValue( _Src );
		_RetValue.multiplies( _Val );
		return _RetValue;
	}

	template< class V, typename std::enable_if< is_numeric< V >::value || std::is_base_of< xvariant, V >::value, xgc_bool >::type = true >
	XGC_INLINE xvariant operator / ( const xvariant& _Src, V _Val )
	{
		xvariant _RetValue( _Src );
		_RetValue.division( _Val );
		return _RetValue;
	}
}
#endif // __VARIANT_H__