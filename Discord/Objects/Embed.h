#pragma once
#ifndef _GREEKBOT_EMBED_H_
#define _GREEKBOT_EMBED_H_
#include "Types.h"

class cEmbed final {
private:
	char* title;
	int color;

public:
	cEmbed() : title(nullptr), color(-1) {}
	const char* GetTitle() const { return title; }
	int GetColor() const { return color; }

	cEmbed& SetTitle(const char* v) {
		delete[] title;
		title = new char[strlen(v) + 1];
		strcpy(title, v);
		return *this;
	}
	cEmbed& SetColor(int v) {
		color = v;
		return *this;
	}

	json::object ToJson() const {
		json::object obj;
		if (title)
			obj["title"] = title;
		if (color != -1)
			obj["color"] = color;
		return obj;
	}

};
typedef   hHandle<cEmbed>   hEmbed;
typedef  chHandle<cEmbed>  chEmbed;
typedef  uhHandle<cEmbed>  uhEmbed;
typedef uchHandle<cEmbed> uchEmbed;
typedef  shHandle<cEmbed>  shEmbed;
typedef schHandle<cEmbed> schEmbed;
#endif /* _GREEKBOT_EMBED_H_ */
