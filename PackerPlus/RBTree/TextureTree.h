#include "RBTree.cpp"
#include "Rect.h"
#include "../CXImage/ximage.h"

class TextureTree :
	public RBTree<TextureTree>
{
public:

	TextureTree();
	/*initialize with a specific rect*/
	TextureTree(const Rect<int> rect);
	virtual ~TextureTree();
	/*global padding*/
	int padding;
	/*use bleeding tech;
	requires padding*/
	bool bleed;
	/*texture rect*/
	Rect<int> rect;
	/*texture*/
	CxImage* image;
	/*tile name*/
	char* name;
	/*texture index*/
	int index;
	/*add texture;
	if failed, null will be returned*/
	bool add_texture(CxImage* input, int index, const char* name);
	/*get valid tiles*/
	void get_bounds(std::vector<TextureTree*>& bounds);
	/*get valid tiles' names*/
	void get_names(std::vector<char*>& names);
	/*build all tiles into target texture*/
	void build(CxImage& output);
	bool operator < (const TextureTree& tree) const;	
private:
	bool _filled;
};
