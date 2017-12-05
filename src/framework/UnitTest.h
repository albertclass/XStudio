#ifndef _UNIT_TEST_H_
#define _UNIT_TEST_H_

#define REGIST_UNITTEST( TEST ) xgc_bool __unittest_##TEST##__ = getUnitTestMgr().Regist< TEST >( #TEST );
///
/// 单元测试接口
/// [5/12/2015] create by albert.xu
///
struct UnitTest
{
	///
	/// 单元测试初始化
	/// [5/12/2015] create by albert.xu
	///
	virtual xgc_bool Init( ini_reader &ini ) = 0;

	///
	/// 单元测试入口
	/// [5/12/2015] create by albert.xu
	///
	virtual xgc_bool Main( int argc, char* argv[] ) = 0;
};

///
/// 测试注册帮助
/// [5/12/2015] create by albert.xu
///
struct UnitTestManager : public xgc::vector< xgc::tuple< xgc::string, UnitTest * > >
{
	friend UnitTestManager& getUnitTestMgr();
private:
	UnitTestManager()
	{

	}

	~UnitTestManager()
	{
		for( auto it = begin(); it != end(); ++it )
		{
			SAFE_DELETE( std::get< 1 >( *it ) );
		}

		clear();
	}

public:
	template< class _Ty, typename std::enable_if< std::is_base_of< UnitTest, _Ty >::value, xgc_bool >::type = true >
	xgc_bool Regist( xgc_lpcstr lpClassName, UnitTest *pTest = XGC_NEW _Ty() )
	{
		push_back( std::make_tuple( lpClassName, pTest ) );
		return true;
	}
};

UnitTestManager& getUnitTestMgr();

///
/// 单元测试初始化的接口转发函数
/// [5/12/2015] create by albert.xu
///
xgc_bool UTestInit( ini_reader &ini, xgc_lpvoid params );

///
/// 测试入口函数
/// [5/12/2015] create by albert.xu
///
xgc_bool UTestMain( int argc, char* argv[] );

#endif // _UNIT_TEST_H_