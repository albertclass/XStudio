#ifndef _SERVER_SEQUENCE_H_
#define _SERVER_SEQUENCE_H_

namespace ServerSequence
{
	///
	/// 初始化ServerSequence
	/// [6/4/2015] create by jianglei.kinly
	///
	xgc_bool InitServerSequence();

	///
	/// 获得一个sequenceID
	/// [6/4/2015] create by jianglei.kinly
	///
	xgc_uint64 GetSID();
};

#endif // _SERVER_SEQUENCE_H_