//Primitive intrusive linked list implementation by pastelmind
//Based on StarCraft's internal code and Patrick Wyatt's article at:
// http://www.codeofhonor.com/blog/avoiding-game-crashes-related-to-linked-lists

#pragma once
#include <cstddef>
#include "../../types.h"

//V241 for VS2008

template <class T>
struct Link
{
	T* prev;
	T* next;
};

template <class T>
struct List
{
	T* head;
	T* tail;

	/// Inserts @p t into the linked list, before the @p target.
	template <Link<T> T::* link>
	void insertBefore(T* t, T* target);

	/// Inserts @p t into the linked list, before the list head.
	template <Link<T> T::* link>
	void insertBeforeHead(T* t);

	/// Inserts @p t into the linked list, after the @p target.
	template <Link<T> T::* link>
	void insertAfter(T* t, T* target);

	/// Inserts @p t into the linked list, after the list head.
	template <Link<T> T::* link>
	void insertAfterHead(T* t);

	/// Unlinks @p t from the linked list.
	template <Link<T> T::* link>
	void unlink(T* t);

	/// Unlinks and returns the head element from the linked list.
	template <Link<T> T::* link>
	T* popHead();
};

//List whose head and tail are placed separately in memory.
template <class T, Link<T> T::* link>
struct ListExtern
{
	T*& head;
	T*& tail;
	ListExtern(T** head, T** tail) : head(*head), tail(*tail) {};
	ListExtern(T*& head, T*& tail) : head(head), tail(tail) {};
	ListExtern(u32 head, u32 tail) : head(*(T**)head), tail(*(T**)tail) {};

	/// Inserts @p t into the linked list, before the @p target.
	void insertBefore(T* t, T* target) const;

	/// Inserts @p t into the linked list, before the list head.
	void insertBeforeHead(T* t) const;

	/// Inserts @p t into the linked list, after the @p target.
	void insertAfter(T* t, T* target) const;

	/// Inserts @p t into the linked list, after the list head.
	void insertAfterHead(T* t) const;

	/// Unlinks @p t from the linked list.
	void unlink(T* t) const;

	/// Unlinks and returns the head element from the linked list.
	T* popHead() const;
};

//-------- Actual algorithms --------//

namespace ListFuncs
{

	template <class T, Link<T> T::* link>
	void insertBefore(T*& head, T*& tail, T* t, T* target)
	{
		if (head)
		{
			if (head == target)
				head = t;
			(t->*link).prev = (target->*link).prev;
			(t->*link).next = target;
			if ((target->*link).prev)
				((target->*link).prev->*link).next = t;
			(target->*link).prev = t;
		}
		else
		{
			head = t;
			tail = t;
		}
	}

	template <class T, Link<T> T::* link>
	void insertAfter(T*& head, T*& tail, T* t, T* target)
	{
		if (head)
		{
			if (tail == target)
				tail = t;
			(t->*link).prev = target;
			(t->*link).next = (target->*link).next;
			if ((target->*link).next)
				((target->*link).next->*link).prev = t;
			(target->*link).next = t;
		}
		else
		{
			head = t;
			tail = t;
		}
	}

	template <class T, Link<T> T::* link>
	void unlink(T*& head, T*& tail, T* t)
	{
		if (head == t)
			head = (t->*link).next;
		if (tail == t)
			tail = (t->*link).prev;

		if ((t->*link).prev)
			((t->*link).prev->*link).next = (t->*link).next;
		if ((t->*link).next)
			((t->*link).next->*link).prev = (t->*link).prev;

		(t->*link).prev = nullptr;
		(t->*link).next = nullptr;
	}

	//Based on function @ 0x004D4E30
	template <class T, Link<T> T::* link>
	T* popHead(T*& head, T*& tail)
	{
		T* t = head;
		unlink<T, link>(head, tail, t);
		return t;
	}

} //ListFuncs

//-------- List template member function definitions --------//

template <class T>
template <Link<T> T::* link>
void List<T>::insertBefore(T* t, T* target)
{
	ListFuncs::insertBefore<T, link>(head, tail, t, target);
}

template <class T>
template <Link<T> T::* link>
void List<T>::insertBeforeHead(T* t)
{
	insertBefore<link>(t, head);
}

template <class T>
template <Link<T> T::* link>
void List<T>::insertAfter(T* t, T* target)
{
	ListFuncs::insertAfter<T, link>(head, tail, t, target);
}

template <class T>
template <Link<T> T::* link>
void List<T>::insertAfterHead(T* t)
{
	insertAfter<link>(t, head);
}

template <class T>
template <Link<T> T::* link>
void List<T>::unlink(T* t)
{
	ListFuncs::unlink<T, link>(head, tail, t);
}


template <class T>
template <Link<T> T::* link>
T* List<T>::popHead()
{
	return ListFuncs::popHead<T, link>(head, tail);
}

//-------- ListExtern template member function definitions --------//

template <class T, Link<T> T::* link>
void ListExtern<T, link>::insertBefore(T* t, T* target) const
{
	ListFuncs::insertBefore<T, link>(head, tail, t, target);
}

template <class T, Link<T> T::* link>
void ListExtern<T, link>::insertBeforeHead(T* t) const
{
	insertBefore(t, head);
}

template <class T, Link<T> T::* link>
void ListExtern<T, link>::insertAfter(T* t, T* target) const
{
	ListFuncs::insertAfter<T, link>(head, tail, t, target);
}

template <class T, Link<T> T::* link>
void ListExtern<T, link>::insertAfterHead(T* t) const
{
	insertAfter(t, head);
}

template <class T, Link<T> T::* link>
void ListExtern<T, link>::unlink(T* t) const
{
	ListFuncs::unlink<T, link>(head, tail, t);
}

template <class T, Link<T> T::* link>
T* ListExtern<T, link>::popHead() const
{
	return ListFuncs::popHead<T, link>(head, tail);
}
