/*
 *用途：网络重复数据删除支持多线程操作的消息队列模板实现
 *功能描述：每个具体的数据重复删除线程分为两个子线程：分片线程（生产者），索引线程（消费者），消息队列bounded_buffer
 *        是唯一的生产者-消费者数据通信方式
 *说明：本实现需安装boost准C++标准程序库：循环队列以及多线程API，
 *     为华为公司(http://www.huawei.com/)的数据重复删除项目使用
 *创建日期：2013.1.26
 *修改作者：刘屹
 */

#include <boost/circular_buffer.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include <boost/progress.hpp>
#include <boost/bind.hpp>

template<class T>
class bounded_buffer {
public:

	typedef boost::circular_buffer<T> container_type;
	typedef typename container_type::size_type size_type;
	typedef typename container_type::value_type value_type;

	explicit bounded_buffer(size_type capacity) :
			m_unread(0), m_container(capacity) {
	}

	//压入数据函数
	void push_front(const value_type& item) {
		boost::mutex::scoped_lock lock(m_mutex);
		m_not_full.wait(lock,
				boost::bind(&bounded_buffer<value_type>::is_not_full, this));
		m_container.push_front(item);
		++m_unread;
		lock.unlock();
		m_not_empty.notify_one();
	}

	//取出数据函数
	void pop_back(value_type* pItem) {
		boost::mutex::scoped_lock lock(m_mutex);
		m_not_empty.wait(lock,
				boost::bind(&bounded_buffer<value_type>::is_not_empty, this));
		*pItem = m_container[--m_unread];
		lock.unlock();
		m_not_full.notify_one();
	}

private:
	bounded_buffer(const bounded_buffer&);          // Disabled copy constructor
	bounded_buffer& operator =(const bounded_buffer&); // Disabled assign operator

	bool is_not_empty() const {
		return m_unread > 0;
	}
	bool is_not_full() const {
		return m_unread < m_container.capacity();
	}
	size_type m_unread;
	container_type m_container;
	boost::mutex m_mutex;
	boost::condition m_not_empty;
	boost::condition m_not_full;
};
