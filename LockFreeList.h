#pragma once

#include <atomic>
#include <iostream>
#include <mutex>

template<typename T>
class LockFreeList {

private:
	struct Node;

public:
	class iterator
	{
		Node* m_node;

	public:
		friend class LockFreeList<T>;

		iterator(Node* node);

		iterator operator++ (int);

		iterator& operator++ ();

		bool operator!= (const iterator&) const;

		bool operator== (const iterator&) const;

		T& operator* ();
	};

	class const_iterator
	{
		const Node* m_node;

	public:
		friend class LockFreeList<T>;

		const_iterator(const Node* node);

		const_iterator operator++ (int);

		const_iterator& operator++ ();

		bool operator!= (const const_iterator& check) const;

		bool operator== (const const_iterator& check) const;

		const T& operator* () const;
	};

	iterator find(const T& el);

	const_iterator find(const T& el) const;

	iterator begin();

	iterator end();

	const_iterator cbegin() const;

	const_iterator cend() const;

	LockFreeList();
	~LockFreeList();

	LockFreeList& operator=(const LockFreeList& other);
	T& operator[](unsigned int pos);

	bool startFromHead(const unsigned int& pos);

	bool isEmpty();

	void insert(const T& data, const unsigned int& pos);
	void insert(const T& data);

	void remove(const unsigned int& pos);
	void remove();

	void clear();

	void print();

	unsigned int peek(T& out);

	size_t size();

private:
	struct Node {
		T data;
		Node* next;
		Node* prev;
		std::mutex mutex;
	};

	typedef Node* NodePtr;

	// returns the node at the specified position
	NodePtr getNode(unsigned int pos) const
	{
		// check for out of bounds
		if (pos > m_size)
			throw std::out_of_range("");

		// start at the first node then walk the list until you hit the specified position
		NodePtr current = head->next;
		for (unsigned int i = 0; i < pos && current; ++i)
			current = current->next;

		// return node
		return current;
	}

	NodePtr head;
	NodePtr tail;
	NodePtr current;
	std::atomic<size_t> m_size;
};

template <typename T>
LockFreeList<T>::iterator::iterator(Node* node) : m_node(node) {}

template <typename T>
bool LockFreeList<T>::iterator::operator!=(const iterator& other) const
{
	return !(*this == other);
}

template <typename T>
bool LockFreeList<T>::iterator::operator==(const iterator& other) const
{
	return other.m_node == m_node;
}

template <typename T>
T& LockFreeList<T>::iterator::operator* ()
{
	return m_node->data;
}

template <typename T>
typename LockFreeList<T>::iterator LockFreeList<T>::iterator::operator++(int)
{
	assert(m_node != nullptr);
	iterator temp(*this);
	m_node = m_node->next;
	return temp;
}

template <typename T>
typename LockFreeList<T>::iterator& LockFreeList<T>::iterator::operator++()
{
	assert(m_node != nullptr);
	m_node = m_node->next;
	return *this;

}

template <typename T>
LockFreeList<T>::const_iterator::const_iterator(const Node* node) : m_node(node) {}

template <typename T>
bool LockFreeList<T>::const_iterator::operator!=(const const_iterator& other) const
{
	return !(*this == other);
}

template <typename T>
bool LockFreeList<T>::const_iterator::operator==(const const_iterator& other) const
{
	return other.m_node == m_node;
}

template <typename T>
const T& LockFreeList<T>::const_iterator::operator* () const
{
	return m_node->data;
}

template <typename T>
typename LockFreeList<T>::const_iterator LockFreeList<T>::const_iterator::operator++(int)
{
	assert(m_node != nullptr);
	iterator temp(*this);
	m_node = m_node->next;
	return temp;
}

template <typename T>
typename LockFreeList<T>::const_iterator& LockFreeList<T>::const_iterator::operator++()
{
	assert(m_node != nullptr);
	m_node = m_node->next;
	return *this;
}

template <typename T>
typename LockFreeList<T>::iterator LockFreeList<T>::find(const T& el)
{
	bool search = true;

	iterator curr(head);
	while (search)
	{
		std::lock_guard<std::mutex>((++curr).m_node->mutex);
		if ((curr == tail) || (*curr == el))
			search = false;
	}
	if (curr != tail)
		return curr;

	return end();
}

template <typename T>
typename LockFreeList<T>::const_iterator LockFreeList<T>::find(const T& el) const
{
	bool search = true;

	const_iterator curr(head);
	while (search)
	{
		std::lock_guard<std::mutex>((++curr).m_node->mutex);
		if ((curr == tail) || (*curr == el))
			search = false;
	}
	if (curr != tail)
		return curr;

	return cend();
}

template <typename T>
typename LockFreeList<T>::iterator LockFreeList<T>::begin()
{
	return iterator(head->next);
}

template <typename T>
typename LockFreeList<T>::iterator LockFreeList<T>::end()
{
	return iterator(tail);
}

template <typename T>
typename LockFreeList<T>::const_iterator LockFreeList<T>::cbegin() const
{
	return const_iterator(head->next);
}

template <typename T>
typename LockFreeList<T>::const_iterator LockFreeList<T>::cend() const
{
	return const_iterator(tail);
}

template <typename T>
LockFreeList<T>::LockFreeList()
{
	head = new Node;
	tail = new Node;

	head->prev = 0;
	head->next = tail;

	tail->next = 0;
	tail->prev = head;

	current = 0;
	m_size = 0;
}

template <typename T>
LockFreeList<T>::~LockFreeList()
{
	// delete everything inbetween head and tail then deletes those
	current = head->next;
	while (current != tail) {
		head->next = current->next;
		delete current;
		current = head->next;
	}
	delete head;
	delete tail;
}

template <typename T>
LockFreeList<T>& LockFreeList<T>::operator=(const LockFreeList& other)
{
	//clear the list just in case
	clear();

	//walk down other and assign the data of each node to the lhs list
	for (unsigned int i = 0; i < other.m_size; ++i)
		insert(other.getNode(i)->data, i);

	return *this;
}

template <typename T>
T& LockFreeList<T>::operator[](unsigned int pos)
{
	//return the data of the node at specified position
	return getNode(pos)->data;
}


template <typename T>
bool LockFreeList<T>::startFromHead(const unsigned int& pos)
{
	// return whether starting at head is more efficient
	return pos < (m_size / 2);
}

template <typename T>
bool LockFreeList<T>::isEmpty()
{
	return head->next == tail;
}

template <typename T>
void LockFreeList<T>::insert(const T& data, const unsigned int& pos)
{
	//out of bounds check
	if (!(pos <= m_size)) {
		throw std::out_of_range("");
		return;
	}
	NodePtr n = new Node;

	//if starting from head is more efficient
	if (startFromHead(pos))
	{
		current = head->next;
		unsigned int i = 0;
		while (i++<pos)
		{
			current = current->next;
		}

	}
	//otherwise start from tail
	else
	{
		current = tail;
		unsigned int i = m_size;
		while (i-->pos)
		{
			current = current->prev;
		}
	}

	//rearrange nodes to fit the new node
	n->next = current;
	n->prev = current->prev;
	n->prev->next = n;
	current->prev = n;

	n->data = data;
	m_size++;
}

template <typename T>
void LockFreeList<T>::insert(const T& data)
{
	// insert data at the end of the list
	insert(data, m_size);
}

template <typename T>
void LockFreeList<T>::remove(const unsigned int& pos)
{
	//if size is 0, there are no nodes to remove
	if (m_size == 0) {
		return;
	}
	//out of bounds check
	if (pos > m_size) {
		throw std::out_of_range("");
		return;
	}
	//check if starting from head is more efficient
	if (startFromHead(pos))
	{
		unsigned int i = 0;
		current = head->next;
		while (i++ < pos)
		{
			current = current->next;
		}
		//connect the nodes surrounding current then delete it
		current->prev->next = current->next;
		current->next->prev = current->prev;
		delete current;
	}
	//otherwise start from tail
	else
	{
		unsigned int i = m_size - 1;
		current = tail->prev;
		while (i-- > pos)
		{
			current = current->prev;
		}
		current->prev->next = current->next;
		current->next->prev = current->prev;
		delete current;
	}

	m_size--;
}

template <typename T>
void LockFreeList<T>::remove()
{
	// remove node from end of list
	remove(m_size);
}

template <typename T>
void LockFreeList<T>::clear()
{
	// delete all nodes inbetween head and tail then connect the two
	current = head->next;
	while (current != tail) {
		head->next = current->next;
		delete current;
		current = head->next;
	}
	tail->prev = head;
	m_size = 0;
}

template <typename T>
void LockFreeList<T>::print()
{
	if (head->next == tail) {
		return;
	}
	// walk down list and print each element
	current = head->next;
	while (current != tail) {
		std::cout << current->data << std::endl;
		current = current->next;
	}
}

template <typename T>
unsigned int LockFreeList<T>::peek(T& out)
{
	if (head->next == tail)
	{
		return 0;
	}
	//set out to the data of the last node
	out = tail->prev->data;
	return 1;
}

template <typename T>
size_t LockFreeList<T>::size()
{
	return m_size;
}