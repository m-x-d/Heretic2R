#pragma once

template <class V>
class List 
{
	struct Node;
	friend struct Node;
	struct Node
	{
		Node *next;
		Node *prev;
		V value;
	};

	Node *NewNode() 
	{
		Node *r=new Node;
		r->prev=r;
		r->next=r;
		return r;
	}
	Node *NewNode(const V& val,Node *n=0,Node *p=0) 
	{
		Node *r=new Node;
		r->value=val;
		r->prev=p?p:r;
		r->next=n?n:r;
		return r;
	}
	void DelNode(Node *n) {delete n;}
	Node *Head;
	int size;
public:
	class Iter;
	friend class Iter;
	class Iter
	{
		Node *cur;
	public:
		Iter() {}
		Iter(Node *t) {cur=t;}
		V & operator*() const {return cur->value;}
		Iter& operator++() {cur=cur->next;return *this;}
		Iter operator++(int) {Iter Tmp = *this;++*this;return Tmp;}
		Iter& operator--() {cur=cur->prev;return *this;}
		Iter operator--(int) {Iter Tmp = *this;--*this;return Tmp;}
		bool operator==(const Iter& x) const {return cur == x.cur; }
		bool operator!=(const Iter& x) const {return !(*this == x); }
		Node *Mynode() const {return cur;}
	};

	List() {Head=NewNode();size=0;}
	~List()
	{
		Erase(Begin(), End());
		DelNode(Head);
		Head = 0;
		size = 0; 
	}
	int Size() const {return size;}
	Iter Begin() {return Iter(Head->next);}
	Iter End() {return Iter(Head);}
	void Insert(Iter P, const V& X)
	{
		Node *S = P.Mynode();
		S->prev = NewNode(X,S,S->prev);
		S = S->prev;
		S->prev->next=S;
		size++;
	}
	Iter Erase(Iter P)
	{
		Node *S = (P++).Mynode();
		S->prev->next=S->next;
		S->next->prev=S->prev;
		DelNode(S);
		--size;
		return (P); 
	}
	Iter Erase(Iter F,Iter L)
	{
		while (F != L)
			Erase(F++);
		return (F); 
	}
	void PushFront(const V& X) {Insert(Begin(), X); }
	void PopFront() {Erase(Begin()); }
	void PushBack(const V& X) {Insert(End(), X); }
	void PopBack() {Erase(--End()); }
};
