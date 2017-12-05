#ifndef _UNIT_TEST_H_
#define _UNIT_TEST_H_

#define REGIST_UNITTEST( TEST ) xgc_bool __unittest_##TEST##__ = getUnitTestMgr().Regist< TEST >( #TEST );
///
/// ��Ԫ���Խӿ�
/// [5/12/2015] create by albert.xu
///
struct UnitTest
{
	///
	/// ��Ԫ���Գ�ʼ��
	/// [5/12/2015] create by albert.xu
	///
	virtual xgc_bool Init( ini_reader &ini ) = 0;

	///
	/// ��Ԫ�������
	/// [5/12/2015] create by albert.xu
	///
	virtual xgc_bool Main( int argc, char* argv[] ) = 0;
};

///
/// ����ע�����
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
/// ��Ԫ���Գ�ʼ���Ľӿ�ת������
/// [5/12/2015] create by albert.xu
///
xgc_bool UTestInit( ini_reader &ini, xgc_lpvoid params );

///
/// ������ں���
/// [5/12/2015] create by albert.xu
///
xgc_bool UTestMain( int argc, char* argv[] );

#endif // _UNIT_TEST_H_