#include "RBTree.h"

template <class T>
RBTree<T>::RBTree()
{
	left = nullptr;
	right = nullptr;
}

template <class T>
RBTree<T>::~RBTree()
{
	if (left != nullptr)
		delete[] left;
	if (right != nullptr)
		delete[] right;
}

template <class T>
bool RBTree<T>::has_children()
{
	return left != nullptr && right != nullptr;
}

template <class T>
void RBTree<T>::init_children()
{
	left = new T();
	right = new T();
}

template <class T>
void RBTree<T>::get_children(std::vector<T*>& children)
{
	if (left != nullptr)
	{
		static_cast<RBTree<T>*>(left)->get_children(children);
		children.push_back(left);
	}
	if (right != nullptr)
	{
		static_cast<RBTree<T>*>(right)->get_children(children);
		children.push_back(right);
	}
}
