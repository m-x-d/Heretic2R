//
// list.h
//
// Copyright 1998 Raven Software
//

#pragma once

template <class V>
class List
{
	struct Node
	{
		Node* next;
		Node* prev;
		V value;
	};

	Node* NewNode()
	{
		Node* n = new Node;
		n->prev = n;
		n->next = n;

		return n;
	}

	Node* NewNode(const V& val, Node* next = nullptr, Node* prev = nullptr)
	{
		Node* n = new Node;
		n->value = val;
		n->prev = (prev != nullptr ? prev : n);
		n->next = (next != nullptr ? next : n);

		return n;
	}

	Node* head;
	int size;

public:
	class Iter
	{
		Node* cur;

	public:
		Iter()
		{
			cur = nullptr; //mxd. Initialize.
		}

		Iter(Node* node)
		{
			cur = node;
		}

		V& operator*() const
		{
			return cur->value;
		}

		Iter& operator++()
		{
			cur = cur->next;
			return *this;
		}

		Iter operator++(int)
		{
			Iter tmp = *this;
			++*this;

			return tmp;
		}

		Iter& operator--()
		{
			cur = cur->prev;
			return *this;
		}

		Iter operator--(int)
		{
			Iter tmp = *this;
			--*this;

			return tmp;
		}

		bool operator==(const Iter& other) const
		{
			return cur == other.cur; //TODO: why implementation differs from operator!= ?
		}

		bool operator!=(const Iter& other) const
		{
			return !(*this == other);
		}

		Node* MyNode() const
		{
			return cur;
		}
	};

	List()
	{
		head = NewNode();
		size = 0;
	}

	~List()
	{
		Erase(Begin(), End());
		delete head;
		head = nullptr;
		size = 0;
	}

	int Size() const
	{
		return size;
	}

	Iter Begin()
	{
		return Iter(head->next);
	}

	Iter End()
	{
		return Iter(head);
	}

	void Insert(Iter iter, const V& val)
	{
		Node* n = iter.MyNode();
		n->prev = NewNode(val, n, n->prev);
		n = n->prev;
		n->prev->next = n;
		size++;
	}

	Iter Erase(Iter iter)
	{
		Node* n = (iter++).MyNode();
		n->prev->next = n->next;
		n->next->prev = n->prev;
		delete n;
		--size;

		return iter;
	}

	Iter Erase(Iter start, Iter end)
	{
		while (start != end)
			Erase(start++);

		return start;
	}

	void PushFront(const V& val)
	{
		Insert(Begin(), val);
	}

	void PopFront()
	{
		Erase(Begin());
	}

	void PushBack(const V& val)
	{
		Insert(End(), val);
	}

	void PopBack()
	{
		Erase(--End());
	}
};