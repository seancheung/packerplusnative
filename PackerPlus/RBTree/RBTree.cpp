#include "RBTree.h"

template <class T>
RBTree<T>::RBTree()
{
	left = nullptr;
	right = nullptr;
	parent = nullptr;
}

template <class T>
RBTree<T>::~RBTree()
{
	if (left != nullptr)
		delete[] left;
	if (right != nullptr)
		delete[] right;
	left = nullptr;
	right = nullptr;
	parent = nullptr;
}

template <class T>
T* RBTree<T>::get_root()
{
	if (parent == nullptr)
		return static_cast<T*>(this);
	return static_cast<RBTree<T>*>(parent)->get_root();
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
	static_cast<RBTree<T>*>(left)->parent = static_cast<T*>(this);
	static_cast<RBTree<T>*>(right)->parent = static_cast<T*>(this);
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

template <class T>
void RBTree<T>::get_nodes(std::vector<T*>& nodes)
{
	if (left != nullptr)
	{
		static_cast<RBTree<T>*>(left)->get_nodes(nodes);
		nodes.push_back(left);
	}
	if (right != nullptr)
	{
		static_cast<RBTree<T>*>(right)->get_nodes(nodes);
		nodes.push_back(right);
	}

	if (parent == nullptr)
		nodes.push_back(static_cast<T*>(this));
}
