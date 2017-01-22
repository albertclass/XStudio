///
/// CopyRight ? 2016 X Studio
/// \file thread_pool.h
/// \date ���� 2016
///
/// \author albert.xu windxu@126.com
/// \brief �̳߳�
/// 

#pragma once
#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_
#include "defines.h"

#include <thread>
#include <random>
#include <condition_variable>
#include <future>
#include <atomic>

// �����ռ�
namespace xgc
{
	namespace common
	{
		template< class _Fx >
		struct function_ret;

		template< class _Ret, class... _Types >
		struct function_ret< _Ret( _Types... ) >
		{
			typedef _Ret type;
		};

		template< class _Cls, class _Ret, class... _Types >
		struct function_ret< _Ret( _Cls::* )(_Types...) >
		{
			typedef _Ret type;
		};

		template< class _Fx >
		typename function_ret< _Fx >::type function_ret_t( _Fx );

		template< class _Fx >
		typename function_ret< _Fx >::type function_ret_t( _Fx* );

		struct task_base
		{
			virtual void execute() = 0;
		};

		template< class _Func, class... _Types >
		struct task_warp : public task_base
		{
			typedef decltype( _Func ) _Ret;
		private:
			std::packaged_task< _Func( _Types... ) > task;

		public:
			explicit task_warp( std::packaged_task< _Func( _Types... ) > &t )
				: task( t )
			{

			}

			explicit task_warp( std::packaged_task< _Func( _Types... ) > &&t )
				: task( _STD move( t ) )
			{

			}

			virtual void execute() override
			{
				task();
			}

			std::future< _Ret > get_future()
			{
				return task.get_future();
			}
		};

		class thread_pool
		{
		private:
			// �̳߳�
			xgc_vector<std::thread> pool;
			// �������
			xgc_queue<task_base*> tasks;
			// ͬ��
			std::mutex m_task;
			// ��դ
			std::condition_variable cv_task;
			// �Ƿ�ر��ύ
			std::atomic<bool> stop;

		public:
			// ����
			thread_pool( size_t size = 4 ) : stop { false }
			{
				restart( size < 1 ? 1 : size );
			}

			// ����
			~thread_pool()
			{
				shutdown();
			}

			// ֹͣ�����ύ
			void shutdown()
			{
				stop.store( true );
				for( std::thread& thread : pool )
				{
					thread.join();        // �ȴ���������� ǰ�᣺�߳�һ����ִ����
				}

				pool.clear();
			}

			// ���������ύ
			void restart( size_t size = 4 )
			{
				shutdown();

				stop.store( false );
				size = size < 1 ? 1 : size;
				for( size_t i = 0; i < size; ++i )
				{
					pool.emplace_back( &thread_pool::schedual, this );    // push_back(std::thread{...})
				}
			}

			// �ύһ������
			template< class _Fx, class... _Types >
			auto commit( _Fx&& f, _Types&&... args ) ->_STD future< decltype(function_ret_t( std::forward< _Fx >( f ) )) >
			{
				using _Ret = decltype(function_ret_t( std::forward< _Fx >( f ) ));

				if( stop.load() )
				{    // stop == true ??
					throw std::runtime_error( "task executor have closed commit." );
				}

				auto task = XGC_NEW task_warp< _Fx, _Types... >( std::packaged_task<_Ret()>( std::bind( f, std::forward< _Types... >( args... ) );

				std::future< _Ret > future = task->get_future();

				// ������񵽶���
				std::unique_lock<std::mutex> lock { m_task };
				tasks.push( task );

				cv_task.notify_all();    // �����߳�ִ��

				return future;
			}

		private:
			// ��ȡһ����ִ�е� task
			task_base* get_one_task()
			{
				std::unique_lock<std::mutex> lock { m_task };
				cv_task.wait( lock, [this](){ return !tasks.empty(); } ); // wait ֱ���� task
				auto t = tasks.front(); // ȡһ�� task
				tasks.pop();
				return t;
			}

			// �������
			void schedual()
			{
				while( stop.load() )
				{
					task_base* t = get_one_task();
					if( t )
						t->execute();
				}
			}
		};
	} // end namespace common
} // end namespace xgc

#endif // _THREAD_POOL_H_