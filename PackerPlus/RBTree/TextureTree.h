#include "RBTree.h"
#include "Rect.h"
#include "../CXImage/ximage.h"

class TextureTree :
	public RBTree<TextureTree>
{
public:

	TextureTree();
	/*initialize with a specific rect*/
	explicit TextureTree(const Rect<int> rect);
	/*global padding*/
	static int padding;
	/*use bleeding tech;
	requires padding*/
	static bool bleed;
	/*texture rect*/
	Rect<int> rect;
	/*texture*/
	CxImage texture;
	/*tile name*/
	char* name;
	/*texture index*/
	int index;
	/*add texture;
	if failed, null will be returned*/
	bool add_texture(CxImage& texture, int index, const char* name);
	/*get valid tiles*/
	void get_bounds(std::vector<TextureTree*>& bounds);
	/*get valid tiles' names*/
	void get_names(std::vector<char*>& names);
	/*build all tiles into target texture*/
	void build(CxImage& output);
	/*dispose texture then call base*/
	void dispose_children() override;
	virtual ~TextureTree();
private:
	bool _filled;
};
