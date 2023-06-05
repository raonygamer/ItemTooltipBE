#include "Zenova.h"

#include <string>

#include "generated/initcpp.h"
#include <optional>

class ColorFormat {
public:
    inline static const std::string ESCAPE = "§";

    inline static const std::string BLACK = "§0";
    inline static const std::string DARK_BLUE = "§1";
    inline static const std::string DARK_GREEN = "§2";
    inline static const std::string DARK_AQUA = "§3";
    inline static const std::string DARK_RED = "§4";
    inline static const std::string DARK_PURPLE = "§5";
    inline static const std::string GOLD = "§6";
    inline static const std::string GRAY = "§7";
    inline static const std::string DARK_GRAY = "§8";
    inline static const std::string BLUE = "§9";
    inline static const std::string GREEN = "§a";
    inline static const std::string AQUA = "§b";
    inline static const std::string RED = "§c";
    inline static const std::string LIGHT_PURPLE = "§d";
    inline static const std::string YELLOW = "§e";
    inline static const std::string WHITE = "§f";
    inline static const std::string MINECOIN_GOLD = "§g";

    inline static const std::string OBFUSCATED = "§k";
    inline static const std::string BOLD = "§l";
    inline static const std::string ITALIC = "§o";
    inline static const std::string RESET = "§r";
};
namespace std {
	std::vector<std::string> split(const std::string& s, char delim) {
		std::vector<std::string> result;
		std::stringstream ss(s);
		std::string item;

		while (getline(ss, item, delim)) {
			result.push_back(item);
		}

		return result;
	}
	std::string toUpperPost(const std::string& s, char delim) {
		std::string res;
		auto vec = split(s, delim);
		for (auto& sub : vec) {
			sub[0] = toupper(sub[0]);
			res += sub + delim;
		}
		return res;
	}
	size_t find_first(const std::string& s, char delim) {
		auto it = s.find(delim, 0);
		if (it != std::string::npos)
			return it;
		return -1;
	}
};
class ItemStackBase {
public:
	void* filler000 = nullptr;
	void* filler001 = nullptr;
	void* filler002 = nullptr;
	short mAuxValue;
	virtual void dummy() = 0;
};
class Level;
class Item {
public:
	void appendFormattedHovertext(const ItemStackBase&, Level&, std::string&, const bool) const;
	const std::string& getCommandName() const;
};

struct Tooltip {
	std::string buildedAuxVal = ColorFormat::DARK_GRAY + " (#0000)" + ColorFormat::RESET;
	size_t posAfterName = -1;
	std::string string = "";

	std::string identifier;
	short auxValue;

	void operator()(std::string& text) {
		text += buildedAuxVal;
		text += string;
	}
};
std::vector<Tooltip> mTooltipCache;

extern "C" __declspec(dllexport) void addTooltipForItem(const std::string & fullItemName, std::string & text, const ItemStackBase * stackPtr) {
	auto& it = std::find_if(mTooltipCache.begin(), mTooltipCache.end(),
		[&fullItemName, stackPtr](const Tooltip& other) -> bool {
			return other.identifier == fullItemName && (stackPtr ? stackPtr->mAuxValue : 0) == other.auxValue;
		}
	);
	if (it != mTooltipCache.end())
	{
		(*it)(text);
		return;
	}

	Tooltip tip;

	auto splitted = std::split(fullItemName, ':');
	std::string itemNamespace = "minecraft";
	if (splitted.size() > 1)
		itemNamespace = splitted[0];
	std::string fullName = "";
	if (itemNamespace == "minecraft")
		fullName += "minecraft:" + fullItemName;
	else
		fullName += fullItemName;

	std::replace(itemNamespace.begin(), itemNamespace.end(), '_', ' ');
	auto fullNamespace = std::toUpperPost(itemNamespace, ' ');

	tip.identifier = fullItemName;
	tip.auxValue = (stackPtr ? stackPtr->mAuxValue : 0);
	tip.buildedAuxVal = (stackPtr ? fmt::format(" {}(#{:04}){}", ColorFormat::DARK_GRAY, stackPtr->mAuxValue, ColorFormat::RESET) : ColorFormat::DARK_GRAY + " (#0000)" + ColorFormat::RESET);
	tip.posAfterName = std::find_first(text, '\n');
	tip.string += "\n" + ColorFormat::DARK_GRAY + fullName + ColorFormat::RESET;
	tip.string += "\n" + ColorFormat::BLUE + ColorFormat::ITALIC + fullNamespace + ColorFormat::RESET;

	mTooltipCache.push_back(tip);
};

inline static void (*_Item_appendFormattedHovertext)(Item* ptr, const ItemStackBase&, Level&, std::string&, const bool);
static void Item_appendFormattedHovertext(Item* ptr, const ItemStackBase& a, Level& b, std::string& text, const bool showCategory) {
	_Item_appendFormattedHovertext(ptr, a, b, text, showCategory);
	if (ptr) {
		addTooltipForItem(ptr->getCommandName(), text, &a);
	}
}

MOD_FUNCTION void ModStart() {
	mTooltipCache.clear();
	Zenova_Hook(Item::appendFormattedHovertext, &Item_appendFormattedHovertext, &_Item_appendFormattedHovertext);
}
MOD_FUNCTION void ModStop() {
	mTooltipCache.clear();
}