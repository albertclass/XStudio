///
/// CopyRight ? 2016 X Studio
/// \file thread_pool.h
/// \date 八月 2016
///
/// \author albert.xu windxu@126.com
/// \brief 线程池
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

// 命名空间
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

		template< class _Ret >
		struct task_warp : public task_base
		{
		private:
			std::function< _Ret() > exec;
			std::packaged_task< _Ret() > task;

		public:
			explicit task_warp( const std::function< _Ret() > &t )
				: exec( t )
				, task( [this]()->_Ret{ return exec(); } )
			{

			}

			explicit task_warp( std::function< _Ret() > &&t )
				: exec( std::move( t ) )
				, task( [this]()->_Ret{ return exec(); } )
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
			// 线程池
			xgc::vector<std::thread> pool;
			// 任务队列
			xgc::queue<task_base*> tasks;
			// 同步
			std::mutex m_lock_queue;
			// 同步
			std::mutex m_lock_task;
			// 光栅
			std::condition_variable cv_task;
			// 是否关闭提交
			std::atomic<bool> stop;

		public:
			// 构造
			thread_pool() : stop( false )
			{

			}

			explicit thread_pool( size_t size ) : stop { false }
			{
				restart( size < 1 ? 1 : size );
			}

			// 析构
			~thread_pool()
			{
				shutdown();
			}

			// 停止任务提交
			void shutdown()
			{
				stop.store( true );
				cv_task.notify_all();    // 唤醒线程执行
				for( std::thread& thread : pool )
				{
					thread.join();        // 等待任务结束， 前提：线程一定会执行完
				}

				pool.clear();
			}

			// 重启任务提交
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

			// 提交一个任务
			template< class _Fx, class... _Types >
			auto commit( _Fx&& f, _Types&&... args ) ->std::future< decltype(function_ret_t( std::forward< _Fx >( f ) )) >
			{
				using _Ret = decltype(function_ret_t( std::forward< _Fx >( f ) ));

				if( stop.load() )
				{    // stop == true ??qqqq
					throw std::runtime_error( "task executor have closed commit." );
				}

				auto task = XGC_NEW task_warp< _Ret >( std::bind( f, std::forward< _Types >( args )... ) );

				std::future< _Ret > future = task->get_future();

				// 添加任务到队列
				std::unique_lock<std::mutex> lock { m_lock_queue };
				tasks.push( task );

				cv_task.notify_all();    // 唤醒线程执行

				return future;
			}

		private:
			// 获取一个待执行的 task
			task_base* get_one_task()
			{
				std::unique_lock<std::mutex> lock( m_lock_task );
				cv_task.wait( lock, [this](){
					auto is_stop  = stop.load();
					auto is_empty = tasks.empty();
					return  is_stop || !is_empty;
				} ); // wait 直到有 task

				if( false == stop )
				{
					std::unique_lock<std::mutex> lock( m_lock_queue );
					auto t = tasks.front(); // 取一个 task
					tasks.pop();
					return t;
				}

				return xgc_nullptr;
			}

			// 任务调度
			void schedual()
			{
				while( !stop.load() )
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