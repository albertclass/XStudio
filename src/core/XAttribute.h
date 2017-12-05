#ifndef __XATTRIBUTE_H__
#define __XATTRIBUTE_H__

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

	//////////////////////////////////////////////////////////////////////////
	// 简单变体类型类,仅包含了整型和浮点类型
	//////////////////////////////////////////////////////////////////////////
	enum xAttrType
	{
		VT_VOID = 0,
		VT_BOOL,
		VT_CHAR,
		VT_BYTE,
		VT_I16,
		VT_I32,
		VT_I64,
		VT_U16,
		VT_U32,
		VT_U64,
		VT_REAL32,
		VT_REAL64,
		VT_STRING,
		VT_BUFFER,

		VT_TYPE_COUNT,
	};

	/// 定义属性缓冲区
	typedef linear_buffer< reference_buffer > attr_buffer;

	class CORE_API XAttribute
	{
		friend CORE_API attr_buffer& operator >>( attr_buffer& stream, XAttribute& c );
		friend CORE_API attr_buffer& operator <<( attr_buffer& stream, const XAttribute& c );

	public:
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
		struct Type2Value;

		template< xAttrType T >
		struct Value2Type;

		template< >
		struct Type2Value < Void >
		{
			enum { Value = VT_VOID };
		};

		template< >
		struct Type2Value < Bool >
		{
			enum { Value = VT_BOOL };
		};

		template< >
		struct Type2Value < Char >
		{
			enum { Value = VT_CHAR };
		};

		template< >
		struct Type2Value < Byte >
		{
			enum { Value = VT_BYTE };
		};

		template< >
		struct Type2Value < Short >
		{
			enum { Value = VT_I16 };
		};

		template< >
		struct Type2Value < UShort >
		{
			enum { Value = VT_U16 };
		};

		template< >
		struct Type2Value < Integer >
		{
			enum { Value = VT_I32 };
		};

		template< >
		struct Type2Value < Unsigned >
		{
			enum { Value = VT_U32 };
		};

		template< >
		struct Type2Value < Long >
		{
			enum { Value = VT_I64 };
		};

		template< >
		struct Type2Value < ULong >
		{
			enum { Value = VT_U64 };
		};

		template< >
		struct Type2Value < Real32 >
		{
			enum { Value = VT_REAL32 };
		};

		template< >
		struct Type2Value < Real64 >
		{
			enum { Value = VT_REAL64 };
		};


		template< >
		struct Type2Value < StringPtr >
		{
			enum { Value = VT_STRING };
		};


		template< >
		struct Type2Value < BufferPtr >
		{
			enum { Value = VT_BUFFER };
		};

		//////////////////////////////////////////////////////////////////////////
		template< >
		struct Value2Type < VT_VOID >
		{
			typedef Void type;
		};

		template< >
		struct Value2Type < VT_BOOL >
		{
			typedef Bool type;
		};

		template< >
		struct Value2Type < VT_CHAR >
		{
			typedef Char type;
		};

		template< >
		struct Value2Type < VT_BYTE >
		{
			typedef Byte type;
		};

		template< >
		struct Value2Type < VT_I16 >
		{
			typedef Short type;
		};

		template< >
		struct Value2Type < VT_U16 >
		{
			typedef UShort type;
		};

		template< >
		struct Value2Type < VT_I32 >
		{
			typedef Integer type;
		};

		template< >
		struct Value2Type < VT_U32 >
		{
			typedef Unsigned type;
		};

		template< >
		struct Value2Type < VT_I64 >
		{
			typedef Long type;
		};

		template< >
		struct Value2Type < VT_U64 >
		{
			typedef ULong type;
		};

		template< >
		struct Value2Type < VT_REAL32 >
		{
			typedef Real32 type;
		};

		template< >
		struct Value2Type < VT_REAL64 >
		{
			typedef Real64 type;
		};

		template< >
		struct Value2Type < VT_STRING >
		{
			typedef StringPtr type;
		};

		template< >
		struct Value2Type < VT_BUFFER >
		{
			typedef BufferPtr type;
		};

		///
		/// 默认构造，不允许隐式调用
		/// [9/9/2014] create by albert.xu
		///
		XAttribute() = delete; 

	public:
		///
		/// [1/26/2014 albert.xu]
		/// 拷贝构造
		///
		XAttribute( xAttrType eType, xgc_lpvoid lpValue )
			: mType( eType )
			, mpVoid( lpValue )
		{

		}

		///
		/// [1/26/2014 albert.xu]
		/// 拷贝构造
		///
		XAttribute( const XAttribute& _Val )
			: mType( _Val.mType )
			, mpVoid( _Val.mpVoid )
		{

		}

		static xgc_lpcstr Type2String( xAttrType eType )
		{
			xgc_lpcstr lpStr = "VT_UNKNOWE";
			switch( eType )
			{
				case xgc::VT_VOID: lpStr = "VT_VOID"; break;
				case xgc::VT_BOOL: lpStr = "VT_BOOL"; break;
				case xgc::VT_CHAR: lpStr = "VT_CHAR"; break;
				case xgc::VT_BYTE: lpStr = "VT_BYTE"; break;
				case xgc::VT_I16: lpStr = "VT_I16"; break;
				case xgc::VT_I32: lpStr = "VT_I32"; break;
				case xgc::VT_I64: lpStr = "VT_I64"; break;
				case xgc::VT_U16: lpStr = "VT_U16"; break;
				case xgc::VT_U32: lpStr = "VT_U32"; break;
				case xgc::VT_U64: lpStr = "VT_U64"; break;
				case xgc::VT_REAL32: lpStr = "VT_REAL"; break;
				case xgc::VT_REAL64: lpStr = "VT_REAL64"; break;
				case xgc::VT_STRING: lpStr = "VT_STRING"; break;
				case xgc::VT_BUFFER: lpStr = "VT_BUFFER"; break;
			}

			return lpStr;
		}

		static xgc_size Type2Size( xAttrType eType )
		{
			xgc_size nTypeSize = 0;
			switch( eType )
			{
				case xgc::VT_BOOL: nTypeSize = sizeof Bool;      break;
				case xgc::VT_CHAR: nTypeSize = sizeof Char;      break;
				case xgc::VT_BYTE: nTypeSize = sizeof Byte;      break;
				case xgc::VT_I16: nTypeSize = sizeof Short;     break;
				case xgc::VT_I32: nTypeSize = sizeof Integer;   break;
				case xgc::VT_I64: nTypeSize = sizeof Long;      break;
				case xgc::VT_U16: nTypeSize = sizeof UShort;    break;
				case xgc::VT_U32: nTypeSize = sizeof Unsigned;  break;
				case xgc::VT_U64: nTypeSize = sizeof ULong;     break;
				case xgc::VT_REAL32: nTypeSize = sizeof Real32;    break;
				case xgc::VT_REAL64: nTypeSize = sizeof Real64;    break;
				case xgc::VT_STRING: nTypeSize = sizeof StringPtr; break;
				case xgc::VT_BUFFER: nTypeSize = sizeof BufferPtr; break;
			}

			return nTypeSize;
		}

		///
		/// 删除委托的对象，需要显示调用
		/// [9/9/2014] create by albert.xu
		///
		xgc_void Release()
		{
			if( isNumeric() )
				setValue( 0 );
			else if( isBool() )
				setValue( false );
			else if( isString() )
				free( *mpString );
			else if( isBuffer() )
				free( *mpBuffer );
			else
				XGC_DEBUG_MESSAGE( "未知的数据类型被释放" );

			mpVoid = xgc_nullptr;
		}

		///
		/// [1/9/2014 albert.xu]
		/// 获取数据原始类型
		///
		xAttrType getType()const
		{
			return mType;
		}

		///
		/// [1/9/2014 albert.xu]
		/// 是否整型
		///
		xgc_bool isInteger()const
		{
			switch( getType() )
			{
				case VT_CHAR:
				case VT_I16:
				case VT_I32:
				case VT_I64:
				return true;
			}

			return false;
		}

		///
		/// [1/9/2014 albert.xu]
		/// 是否无符号
		///
		xgc_bool isUnsigned()const
		{
			switch( getType() )
			{
				case VT_BYTE:
				case VT_U16:
				case VT_U32:
				case VT_U64:
				return true;
			}

			return false;
		}

		///
		/// [1/9/2014 albert.xu]
		/// 是否实数
		///
		xgc_bool isReal()const
		{
			return getType() == VT_REAL32 || getType() == VT_REAL64;
		}

		///
		/// [1/9/2014 albert.xu]
		/// 是否数值类型
		///
		xgc_bool isNumeric()const
		{
			return isInteger() || isUnsigned() || isReal();
		}

		///
		/// 是否是bool类型
		/// [12/24/2014] create by jianglei.kinly
		///
		xgc_bool isBool()const
		{
			return getType() == VT_BOOL;
		}

		///
		/// [1/9/2014 albert.xu]
		/// 是否字符串
		///
		xgc_bool isString()const
		{
			return getType() == VT_STRING;
		}

		///
		/// [1/9/2014 albert.xu]
		/// 是否缓冲
		///
		xgc_bool isBuffer()const
		{
			return getType() == VT_BUFFER;
		}

		///
		/// [1/9/2014 albert.xu]
		/// 是否
		///
		xgc_bool isValid()const { return getType() != VT_VOID; }

		///
		/// [1/8/2014 albert.xu]
		/// 整数赋值
		///
		template< class V, typename std::enable_if< is_numeric< V >::value, xgc_bool >::type = true >
		xgc_void setValue( V _Val )
		{
			XGC_ASSERT_RETURN( isNumeric(), xgc_void( 0 ) );
			switch( getType() )
			{
				case VT_CHAR:
				*mpChar = ( XAttribute::Char ) _Val;
				break;
				case VT_BYTE:
				*mpByte = ( XAttribute::Byte ) _Val;
				break;
				case VT_I16:
				*mpShort = ( XAttribute::Short )_Val;
				break;
				case VT_I32:
				*mpInteger = ( XAttribute::Integer )_Val;
				break;
				case VT_I64:
				*mpLong = ( XAttribute::Long )_Val;
				break;

				case VT_U16:
				*mpUShort = ( XAttribute::UShort )_Val;
				break;
				case VT_U32:
				*mpUnsigned = ( XAttribute::Unsigned )_Val;
				break;
				case VT_U64:
				*mpULong = ( XAttribute::ULong )_Val;
				break;

				case VT_REAL32:
				*mpReal32 = ( XAttribute::Real32 )_Val;
				break;
				case VT_REAL64:
				*mpReal64 = ( XAttribute::Real64 )_Val;
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
		template<>
		xgc_void setValue<xgc_bool>( xgc_bool _Val )
		{
			*mpBool = _Val;
		}

		///
		/// [1/8/2014 albert.xu]
		/// 字符串赋值
		///
		xgc_void setString( xgc_lpcstr _Val, xgc_size _Length = -1 );

		///
		/// [1/8/2014 albert.xu]
		/// 缓冲区赋值
		///
		xgc_void setBuffer( xgc_lpvoid _Val, xgc_size _Length );

		///
		/// [3/18/2014 albert.xu]
		/// 设置缓冲区长度
		///
		xgc_bool setBufferLength( xgc_size _Len, xgc_byte _Fill = 0 )const;

		///
		/// 获取Buffer长度
		/// [8/22/2014] create by albert.xu
		///
		xgc_size getBufferLength()const;

		///
		/// [1/8/2014 albert.xu]
		/// 复制赋值
		///
		xgc_void assign( const XAttribute& _Val );

		///
		/// [1/8/2014 albert.xu]
		/// 值操作
		///
		template< class V, typename std::enable_if< is_numeric< V >::value, xgc_bool >::type = true >
		xgc_void multiplies( V _Val )
		{
			switch( getType() )
			{
				case VT_CHAR:
				*mpChar = XAttribute::Char( *mpChar * _Val );
				break;
				case VT_I16:
				*mpShort = XAttribute::Short( *mpShort * _Val );
				break;
				case VT_I32:
				*mpInteger = XAttribute::Integer( *mpInteger * _Val );
				break;
				case VT_I64:
				*mpLong = XAttribute::Long( *mpLong * _Val );
				break;
				case VT_BYTE:
				*mpByte = XAttribute::Byte( *mpByte * _Val );
				break;
				case VT_U16:
				*mpUShort = XAttribute::UShort( *mpUShort * _Val );
				break;
				case VT_U32:
				*mpUnsigned = XAttribute::Unsigned( *mpUnsigned * _Val );
				break;
				case VT_U64:
				*mpULong = XAttribute::ULong( *mpULong * _Val );
				break;
				case VT_REAL32:
				*mpReal32 = XAttribute::Real32( *mpReal32 * _Val );
				break;
				case VT_REAL64:
				*mpReal64 = XAttribute::Real64( *mpReal32 * _Val );
				break;
				default:
				XGC_DEBUG_MESSAGE( "非数值类型使用了数值运算!" );
				break;
			}
		}

		template< class V, typename std::enable_if< is_numeric< V >::value, xgc_bool >::type = true >
		xgc_void division( V _Val )
		{
			switch( getType() )
			{
				case VT_CHAR:
				*mpChar = XAttribute::Char( *mpChar / _Val );
				break;
				case VT_I16:
				*mpShort = XAttribute::Short( *mpShort / _Val );
				break;
				case VT_I32:
				*mpInteger = XAttribute::Integer( *mpInteger / _Val );
				break;
				case VT_I64:
				*mpLong = XAttribute::Long( *mpLong / _Val );
				break;
				case VT_BYTE:
				*mpByte = XAttribute::Byte( *mpByte / _Val );
				break;
				case VT_U16:
				*mpUShort = XAttribute::UShort( *mpUShort / _Val );
				break;
				case VT_U32:
				*mpUnsigned = XAttribute::Unsigned( *mpUnsigned / _Val );
				break;
				case VT_U64:
				*mpULong = XAttribute::ULong( *mpULong / _Val );
				break;
				case VT_REAL32:
				*mpReal32 = XAttribute::Real32( *mpReal32 / _Val );
				break;
				case VT_REAL64:
				*mpReal64 = XAttribute::Real64( *mpReal32 / _Val );
				break;
				default:
				XGC_DEBUG_MESSAGE( "非数值类型使用了数值运算!" );
				break;
			}
		}

		template< class V, typename std::enable_if< is_numeric< V >::value, xgc_bool >::type = true >
		xgc_void plus( V _Val )
		{
			switch( getType() )
			{
				case VT_CHAR:
				*mpChar = XAttribute::Char( *mpChar + _Val );
				break;
				case VT_I16:
				*mpShort = XAttribute::Short( *mpShort + _Val );
				break;
				case VT_I32:
				*mpInteger = XAttribute::Integer( *mpInteger + _Val );
				break;
				case VT_I64:
				*mpLong = XAttribute::Long( *mpLong + _Val );
				break;
				case VT_BYTE:
				*mpByte = XAttribute::Byte( *mpByte + _Val );
				break;
				case VT_U16:
				*mpUShort = XAttribute::UShort( *mpUShort + _Val );
				break;
				case VT_U32:
				*mpUnsigned = XAttribute::Unsigned( *mpUnsigned + _Val );
				break;
				case VT_U64:
				*mpULong = XAttribute::ULong( *mpULong + _Val );
				break;
				case VT_REAL32:
				*mpReal32 = XAttribute::Real32( *mpReal32 + _Val );
				break;
				case VT_REAL64:
				*mpReal64 = XAttribute::Real64( *mpReal32 + _Val );
				break;
				default:
				XGC_DEBUG_MESSAGE( "非数值类型使用了数值运算!" );
				break;
			}
		}

		template< class V, typename std::enable_if< is_numeric< V >::value, xgc_bool >::type = true >
		xgc_void minus( V _Val )
		{
			switch( getType() )
			{
				case VT_CHAR:
				*mpChar = XAttribute::Char( *mpChar - _Val );
				break;
				case VT_I16:
				*mpShort = XAttribute::Short( *mpShort - _Val );
				break;
				case VT_I32:
				*mpInteger = XAttribute::Integer( *mpInteger - _Val );
				break;
				case VT_I64:
				*mpLong = XAttribute::Long( *mpLong - _Val );
				break;
				case VT_BYTE:
				*mpByte = XAttribute::Byte( *mpByte - _Val );
				break;
				case VT_U16:
				*mpUShort = XAttribute::UShort( *mpUShort - _Val );
				break;
				case VT_U32:
				*mpUnsigned = XAttribute::Unsigned( *mpUnsigned - _Val );
				break;
				case VT_U64:
				*mpULong = XAttribute::ULong( *mpULong - _Val );
				break;
				case VT_REAL32:
				*mpReal32 = XAttribute::Real32( *mpReal32 - _Val );
				break;
				case VT_REAL64:
				*mpReal64 = XAttribute::Real64( *mpReal32 - _Val );
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
		XGC_INLINE xgc_void multiplies( const XAttribute& _Val );

		///
		/// [1/9/2014 albert.xu]
		/// 数值 除法
		///
		XGC_INLINE xgc_void division( const XAttribute& _Val );

		///
		/// [1/9/2014 albert.xu]
		/// 数值 加法
		///
		XGC_INLINE xgc_void plus( const XAttribute& _Val );

		///
		/// [1/9/2014 albert.xu]
		/// 数值 减法
		///
		XGC_INLINE xgc_void minus( const XAttribute& _Val );

		//////////////////////////////////////////////////////////////////////////
		// Limit
		//////////////////////////////////////////////////////////////////////////
		template< class VMin, class VMax, typename std::enable_if< is_numeric< VMin >::value && is_numeric< VMax >::value, xgc_bool >::type = true >
		XGC_INLINE xgc_void normalization( VMin _Min, VMax _Max )
		{
			switch( getType() )
			{
				case VT_CHAR:
				if( *mpChar > ( XAttribute::Char )_Max )
					*mpChar = ( XAttribute::Char )_Max;
				else if( *mpChar < ( XAttribute::Char )_Min )
					*mpChar = ( XAttribute::Char )_Min;
				break;
				case VT_BYTE:
				if( *mpByte > ( XAttribute::Byte )_Max )
					*mpByte = ( XAttribute::Byte )_Max;
				else if( *mpByte < ( XAttribute::Byte )_Min )
					*mpByte = ( XAttribute::Byte )_Min;
				break;
				case VT_I16:
				if( *mpShort > ( XAttribute::Short )_Max )
					*mpShort = ( XAttribute::Short )_Max;
				else if( *mpShort < ( XAttribute::Short )_Min )
					*mpShort = ( XAttribute::Short )_Min;
				break;
				case VT_U16:
				if( *mpUShort > ( XAttribute::UShort )_Max )
					*mpUShort = ( XAttribute::UShort )_Max;
				else if( *mpUShort < ( XAttribute::UShort )_Min )
					*mpUShort = ( XAttribute::UShort )_Min;
				break;
				case VT_I32:
				if( *mpInteger > ( XAttribute::Integer )_Max )
					*mpInteger = ( XAttribute::Integer )_Max;
				else if( *mpInteger < ( XAttribute::Integer )_Min )
					*mpInteger = ( XAttribute::Integer )_Min;
				break;
				case VT_U32:
				if( *mpUnsigned > ( XAttribute::Unsigned )_Max )
					*mpUnsigned = ( XAttribute::Unsigned )_Max;
				else if( *mpUnsigned < ( XAttribute::Unsigned )_Min )
					*mpUnsigned = ( XAttribute::Unsigned )_Min;
				break;
				case VT_I64:
				if( *mpLong > ( XAttribute::Long )_Max )
					*mpLong = ( XAttribute::Long )_Max;
				else if( *mpLong < ( XAttribute::Long )_Min )
					*mpLong = ( XAttribute::Long )_Min;
				break;
				case VT_U64:
				if( *mpULong > ( XAttribute::ULong )_Max )
					*mpULong = ( XAttribute::ULong )_Max;
				else if( *mpULong < ( XAttribute::ULong )_Min )
					*mpULong = ( XAttribute::ULong )_Min;
				break;
				case VT_REAL32:
				if( *mpReal32 > ( XAttribute::Real32 )_Max )
					*mpReal32 = ( XAttribute::Real32 )_Max;
				else if( *mpReal32 < ( XAttribute::Real32 )_Min )
					*mpReal32 = ( XAttribute::Real32 )_Min;
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
		XGC_INLINE XAttribute::Integer toInteger()const;

		///
		/// [1/26/2014 albert.xu]
		/// 转为无符号整型
		///
		XGC_INLINE XAttribute::Unsigned toUnsigned()const;

		///
		/// [1/9/2014 albert.xu]
		/// 转为整型取值
		///
		XGC_INLINE XAttribute::Long toLong()const;

		///
		/// [1/26/2014 albert.xu]
		/// 转为无符号整型
		///
		XGC_INLINE XAttribute::ULong toULong()const;

		///
		/// [1/9/2014 albert.xu]
		/// 转为浮点取值
		///
		XGC_INLINE XAttribute::Real32 toReal32()const;

		///
		/// [1/9/2014 albert.xu]
		/// 转为浮点取值
		///
		XGC_INLINE XAttribute::Real64 toReal64()const;

		///
		/// 转为Bool类型
		/// [12/24/2014] create by jianglei.kinly
		///
		XGC_INLINE XAttribute::Bool toBool()const
		{
			if( getType() == VT_BOOL )
				return *mpBool;
			return false;
		}

		///
		/// 转换为数值
		/// [9/9/2014] create by albert.xu
		///
		template< class T >
		XGC_INLINE T toNumeric()const
		{
			switch( getType() )
			{
				case VT_CHAR:
				return (T)*mpChar;
				break;
				case VT_BYTE:
				return (T)*mpByte;
				break;
				case VT_I16:
				return (T)*mpShort;
				break;
				case VT_U16:
				return (T)*mpUShort;
				break;
				case VT_I32:
				return (T)*mpInteger;
				break;
				case VT_U32:
				return (T)*mpUnsigned;
				break;
				case VT_I64:
				return (T)*mpLong;
				break;
				case VT_U64:
				return (T)*mpULong;
				break;
				case VT_REAL32:
				return (T)*mpReal32;
				break;
				case VT_STRING:
				return ( *mpString ) ? str2numeric< T >( (xgc_lpcstr)( *mpString )->str, xgc_nullptr ) : 0;
				break;
				case VT_BUFFER:
				return ( *mpBuffer ) ? str2numeric< T >( (xgc_lpcstr)( *mpBuffer )->buf, xgc_nullptr ) : 0;
				break;
				default:
				XGC_DEBUG_MESSAGE( "未知的属性类型！" );
				break;
			}

			return 0;
		}

		///
		/// [1/9/2014 albert.xu]
		/// 转为字符串
		///
		xgc::string toString()const;

		///
		/// [1/9/2014 albert.xu]
		/// 获取原始字符串指针
		///
		xgc_lpcstr toRawString( xgc_lpcstr lpDefault = "" )const;

		///
		/// [3/17/2014 albert.xu]
		/// 转为原始缓冲区
		///
		xgc_lpvoid toRawBuffer()const;

		///
		/// [1/17/2014 albert.xu]
		/// 设置缓冲区位
		///
		xgc_void SetBit( xgc_size _Bit )const;

		///
		/// [1/17/2014 albert.xu]
		/// 清除缓冲区位
		///
		xgc_void ClrBit( xgc_size _Bit )const;

		///
		/// [1/17/2014 albert.xu]
		/// 获取缓冲区位
		///
		xgc_bool GetBit( xgc_size _Bit, xgc_bool _Default )const;

		///
		/// [1/17/2014 albert.xu]
		/// 获取缓冲区位
		///
		xgc_bool CmpBit( xgc_size _Bit, xgc_bool _Test )const;

		///
		/// [1/8/2014 albert.xu]
		/// 赋值操作符
		///
		XAttribute& operator = ( const XAttribute& _Val )
		{
			assign( _Val );
			return *this;
		}

		template< class V, typename std::enable_if< is_numeric< V >::value, xgc_bool >::type = true >
		XAttribute& operator = ( V _Val )
		{
			setValue( _Val );
			return *this;
		}

		XAttribute& operator = ( xgc_lpcstr _Val )
		{
			setString( _Val );
			return *this;
		}

		///
		/// 算数运算符重载
		/// [9/8/2014] create by albert.xu
		///
		template< class V, typename std::enable_if< is_numeric< V >::value || std::is_base_of< XAttribute, V >::value, xgc_bool >::type = true >
		XAttribute& operator += ( V _Val ) { plus( _Val ); return *this; }
		template< class V, typename std::enable_if< is_numeric< V >::value || std::is_base_of< XAttribute, V >::value, xgc_bool >::type = true >
		XAttribute& operator -= ( V _Val ) { minus( _Val ); return *this; }
		template< class V, typename std::enable_if< is_numeric< V >::value || std::is_base_of< XAttribute, V >::value, xgc_bool >::type = true >
		XAttribute& operator *= ( V _Val ) { multiplies( _Val ); return *this; }
		template< class V, typename std::enable_if< is_numeric< V >::value || std::is_base_of< XAttribute, V >::value, xgc_bool >::type = true >
		XAttribute& operator /= ( V _Val ) { division( _Val ); return *this; }

		XAttribute& operator ++ () { plus( 1 ); return *this; }
		XAttribute& operator -- () { minus( 1 ); return *this; }

	private:
		xAttrType mType;
		union
		{
			XAttribute::Void		*mpVoid;
			XAttribute::Bool		*mpBool;
			XAttribute::Char		*mpChar;
			XAttribute::Byte		*mpByte;
			XAttribute::Short		*mpShort;
			XAttribute::UShort		*mpUShort;
			XAttribute::Integer		*mpInteger;
			XAttribute::Unsigned	*mpUnsigned;
			XAttribute::Long		*mpLong;
			XAttribute::ULong		*mpULong;
			XAttribute::Real32		*mpReal32;
			XAttribute::Real64		*mpReal64;
			XAttribute::StringPtr	*mpString;
			XAttribute::BufferPtr	*mpBuffer;
		};
	};

	///
	/// [1/9/2014 albert.xu]
	/// 转为整型取值
	///
	XGC_INLINE XAttribute::Integer XAttribute::toInteger()const
	{
		return toNumeric< XAttribute::Integer >();
	}

	///
	/// [1/26/2014 albert.xu]
	/// 转为无符号整型
	///
	XGC_INLINE XAttribute::Unsigned XAttribute::toUnsigned()const
	{
		return toNumeric< XAttribute::Unsigned >();
	}

	///
	/// [1/9/2014 albert.xu]
	/// 转为整型取值
	///
	XGC_INLINE XAttribute::Long XAttribute::toLong()const
	{
		return toNumeric< XAttribute::Long >();
	}

	///
	/// [1/26/2014 albert.xu]
	/// 转为无符号整型
	///
	XGC_INLINE XAttribute::ULong XAttribute::toULong()const
	{
		return toNumeric< XAttribute::ULong >();
	}

	///
	/// [1/9/2014 albert.xu]
	/// 转为浮点取值
	///
	XGC_INLINE XAttribute::Real32 XAttribute::toReal32()const
	{
		return toNumeric< XAttribute::Real32 >();
	}

	///
	/// [1/9/2014 albert.xu]
	/// 转为浮点取值
	///
	XGC_INLINE XAttribute::Real64 XAttribute::toReal64()const
	{
		return toNumeric< XAttribute::Real64 >();
	}

	template< class V, typename std::enable_if< is_numeric< V >::value || std::is_base_of< XAttribute, V >::value, xgc_bool >::type = true >
	XGC_INLINE V operator + ( const XAttribute& _Src, V _Val )
	{
		return _Src.toNumeric< decltype( V ) >() + _Val;
	}

	template< class V, typename std::enable_if< is_numeric< V >::value || std::is_base_of< XAttribute, V >::value, xgc_bool >::type = true >
	XGC_INLINE V operator - ( const XAttribute& _Src, V _Val )
	{
		return _Src.toNumeric< decltype( V ) >() - _Val;
	}

	template< class V, typename std::enable_if< is_numeric< V >::value || std::is_base_of< XAttribute, V >::value, xgc_bool >::type = true >
	XGC_INLINE XAttribute operator * ( const XAttribute& _Src, V _Val )
	{
		return _Src.toNumeric< decltype( V ) >() * _Val;
	}

	template< class V, typename std::enable_if< is_numeric< V >::value || std::is_base_of< XAttribute, V >::value, xgc_bool >::type = true >
	XGC_INLINE XAttribute operator / ( const XAttribute& _Src, V _Val )
	{
		return _Src.toNumeric< decltype( V ) >() / _Val;
	}

	template< class V, typename std::enable_if< std::is_integral< V >::value || std::is_base_of< XAttribute, V >::value, xgc_bool >::type = true >
	XGC_INLINE XAttribute operator % ( const XAttribute& _Src, V _Val )
	{
		return _Src.toNumeric< decltype( V ) >() % _Val;
	}

	//////////////////////////////////////////////////////////////////
	template< class V, typename std::enable_if< is_numeric< V >::value || std::is_base_of< XAttribute, V >::value, xgc_bool >::type = true >
	XGC_INLINE V operator + ( V _Val, const XAttribute& _Src )
	{
		return _Val + _Src.toNumeric< decltype( V ) >();
	}

	template< class V, typename std::enable_if< is_numeric< V >::value || std::is_base_of< XAttribute, V >::value, xgc_bool >::type = true >
	XGC_INLINE V operator - ( V _Val, const XAttribute& _Src )
	{
		return _Val - _Src.toNumeric< decltype( V ) >();
	}

	template< class V, typename std::enable_if< is_numeric< V >::value || std::is_base_of< XAttribute, V >::value, xgc_bool >::type = true >
	XGC_INLINE XAttribute operator * ( V _Val, const XAttribute& _Src )
	{
		return _Val * _Src.toNumeric< decltype( V ) >();
	}

	template< class V, typename std::enable_if< is_numeric< V >::value || std::is_base_of< XAttribute, V >::value, xgc_bool >::type = true >
	XGC_INLINE XAttribute operator / ( V _Val, const XAttribute& _Src )
	{
		return _Val / _Src.toNumeric< decltype( V ) >();
	}

	template< class V, typename std::enable_if< std::is_integral< V >::value || std::is_base_of< XAttribute, V >::value, xgc_bool >::type = true >
	XGC_INLINE XAttribute operator % ( V _Val, const XAttribute& _Src )
	{
		return _Val % _Src.toNumeric< decltype( V ) >();
	}

	///
	/// [3/17/2014 albert.xu]
	/// 序列化读
	///
	CORE_API attr_buffer& operator >> ( attr_buffer& stream, XAttribute& c );

	///
	/// [3/17/2014 albert.xu]
	/// 序列化写
	///
	CORE_API attr_buffer& operator << ( attr_buffer& stream, const XAttribute& c );
}

#endif // __XATTRIBUTE_H__