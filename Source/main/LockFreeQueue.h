#pragma once


#include <boost/lockfree/queue.hpp>


////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class	LockFreeQueue
///
/// @brief	Lock-Free 큐 처리를 위한 클래스
////////////////////////////////////////////////////////////////////////////////////////////////////
template< typename super, typename T >
class LockFreeQueueBase
	:
	public super
{
private:
	std::atomic_int m_size; ///< 크기

public:
	/// 생성자
	LockFreeQueueBase()
		:
		m_size( 0 )
	{
	}

	/// 생성자
	LockFreeQueueBase( int size )
		:
		super( size ),
		m_size( 0 )
	{
	}

	/// 소멸자
	~LockFreeQueueBase()
	{
		reset();
	}

	/// 이 객체를 처음 상태로 되돌린다.
	void reset()
	{
		consume_all( [this] ( T const& t )
					 {
						 _delete_if_possible( t );
					 } );
	}

	/// 크기를 반환한다.
	int size() const
	{
		return m_size;
	}

	bool push( T const& t )
	{
		m_size++;
		if ( !super::push( t ) )
		{
			m_size--;
			return false;
		}

		return true;
	}

	bool pop( T& ret )
	{
		if ( !super::pop( ret ) ) return false;

		if ( --m_size < 0 )
		{
			printf( "[LockFreeQueue::pop ] invalid size. [size: %d]", m_size.load() );
			m_size = 0;
		}

		return true;
	}

	template< typename Functor >
	size_t consume_all( Functor& f )
	{
		size_t result = super::consume_all( f );
		if ( ( m_size -= result ) < 0 )
		{
			printf( "[ LockFreeQueue::consume_all ]invalid size. [size: %d]", m_size.load() );
			m_size = 0;
		}

		return result;
	}

	template< typename Functor >
	size_t consume_all( Functor const& f )
	{
		size_t result = super::consume_all( f );
		if ( ( m_size -= result ) < 0 )
		{
			printf( "[[ LockFreeQueue::consume_all ]]invalid size. [size: %d]", m_size.load() );
			m_size = 0;
		}

		return result;
	}

private:
	/// 가능하면 삭제한다.
	template< typename Type >
	void _delete_if_possible( Type* t )
	{
		if ( t )
		{
			delete t;
			t = nullptr;
		}
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// @class	LockFreeQueue
///
/// @brief	Lock-Free 큐 처리를 위한 클래스
////////////////////////////////////////////////////////////////////////////////////////////////////
template< typename T, size_t size = 0 >
class LockFreeQueue
	:
	public LockFreeQueueBase< boost::lockfree::queue< T, boost::lockfree::capacity< size > >, T >
{
};
