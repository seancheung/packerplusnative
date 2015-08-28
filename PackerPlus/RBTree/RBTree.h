#include <vector>

/*T should be derived from RBTree*/
template <class T>
class RBTree
{
public:
	virtual ~RBTree()
	{
	}

	RBTree();
	RBTree(T* left, T* right);
	/*left and right node; 
	they should be initilized and disposed at the same time;*/
	T *left, *right;
	/*check if both children nodes are not null*/
	bool has_children();
	/*initialize both children*/
	void init_children();
	/*dispose children recursively*/
	virtual void dispose_children();
	/*get children recursively*/
	std::vector<T*> get_children();
protected:
	/*release pointer*/
	static void release(void* & pointer);
};
