#include "RBTree.h"

template <class T>
RBTree<T>::RBTree()
{
}

template <class T>
RBTree<T>::RBTree(T* left, T* right) : left(left),
                                       right(right)
{
}

template <class T>
void RBTree<T>::release(void* & pointer)
{
	if (pointer != nullptr)
	{
		delete[] pointer;
		pointer = nullptr;
	}
}

template <class T>
bool RBTree<T>::has_children()
{
	return left != nullptr && right != nullptr;
}

template <class T>
void RBTree<T>::init_children()
{
	dispose_children();
	left = new T();
	right = new T();
}

template <class T>
void RBTree<T>::dispose_children()
{
	if (left != nullptr)
		static_cast<RBTree<T>*>(left)->dispose_children();
	if (right != nullptr)
		static_cast<RBTree<T>*>(right)->dispose_children();
	release(&left);
	release(&right);
}

template <class T>
std::vector<T*> RBTree<T>::get_children()
{
	std::vector<T*> v;
	if (left != nullptr)
	{
		for (T* child : left->get_children())
		{
			v.push_back(child);
		}
	}
	if (right != nullptr)
	{
		for (T* child : right->get_children())
		{
			v.push_back(child);
		}
	}
	return v;
}
