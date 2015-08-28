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
	void get_children(std::vector<T*>& children);
};
