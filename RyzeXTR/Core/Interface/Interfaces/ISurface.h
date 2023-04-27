#pragma once
#include "../../SDK/DataTyes/Color.h"

enum EFontDrawType : int
{
	FONT_DRAW_DEFAULT = 0,
	FONT_DRAW_NONADDITIVE,
	FONT_DRAW_ADDITIVE,
	FONT_DRAW_TYPE_COUNT = 2
};

enum EFontFlags
{
	FONTFLAG_NONE,
	FONTFLAG_ITALIC = 0x001,
	FONTFLAG_UNDERLINE = 0x002,
	FONTFLAG_STRIKEOUT = 0x004,
	FONTFLAG_SYMBOL = 0x008,
	FONTFLAG_ANTIALIAS = 0x010,
	FONTFLAG_GAUSSIANBLUR = 0x020,
	FONTFLAG_ROTARY = 0x040,
	FONTFLAG_DROPSHADOW = 0x080,
	FONTFLAG_ADDITIVE = 0x100,
	FONTFLAG_OUTLINE = 0x200,
	FONTFLAG_CUSTOM = 0x400,
	FONTFLAG_BITMAP = 0x800,
};

struct Vertex_t
{
	Vertex_t() = default;

	Vertex_t(const Vector2D& vecPosition, const Vector2D& vecCoordinate = Vector2D(0, 0)) {

		this->vecPosition = vecPosition;
		this->vecCoordinate = vecCoordinate;
	}

	void Init(const Vector2D& vecPosition, const Vector2D& vecCoordinate = Vector2D(0, 0)) {

		this->vecPosition = vecPosition;
		this->vecCoordinate = vecCoordinate;
	}

	Vector2D vecPosition = { };
	Vector2D vecCoordinate = { };
};

typedef unsigned long HScheme, HPanel, HTexture, HCursor, HFont;

class ISurface {

public:

	void DrawT(int X, int Y, Color Color, HFont Font, bool Center, const char* _Input, ...) {

		int apple = 0;
		/* set up buffer */
		char Buffer[256] = { '\0' };

		/* set up varargs*/
		va_list Args;

		va_start(Args, _Input);
		vsprintf_s(Buffer, _Input, Args);
		va_end(Args);

		size_t Size = strlen(Buffer) + 1;

		/* set up widebuffer*/
		wchar_t* WideBuffer = new wchar_t[Size];

		/* char -> wchar */
		mbstowcs_s(0, WideBuffer, Size, Buffer, Size - 1);

		/* check center */
		int Width = 0, Height = 0;

		if (Center) {
			GetTextSize(Font, WideBuffer, Width, Height);
		}

		/* call and draw*/
		DrawSetTextColor(Color[0], Color[1], Color[2], Color[3]);
		DrawSetTextFont(Font);
		DrawSetTextPos(X - (Width / 2), Y);
		DrawPrintText(WideBuffer, wcslen(WideBuffer));

		delete WideBuffer, Size;

		return;
	}

	void DrawSetColor(Color colDraw) {
		util::CallVFunc<void>(this, 14, colDraw);
	}

	void DrawSetColor(int r, int g, int b, int a) {
		util::CallVFunc<void>(this, 15, r, g, b, a);
	}

	void DrawFilledRect(int x0, int y0, int x1, int y1) {
		util::CallVFunc<void>(this, 16, x0, y0, x1, y1);
	}

	void DrawFilledRectFade(int x0, int y0, int x1, int y1, uint32_t uAlpha0, uint32_t uAlpha1, bool bHorizontal) {
		util::CallVFunc<void>(this, 123, x0, y0, x1, y1, uAlpha0, uAlpha1, bHorizontal);
	}

	void DrawOutlinedRect(int x0, int y0, int x1, int y1) {
		util::CallVFunc<void>(this, 18, x0, y0, x1, y1);
	}

	void DrawLine(int x0, int y0, int x1, int y1) {
		util::CallVFunc<void>(this, 19, x0, y0, x1, y1);
	}

	void DrawPolyLine(int* x, int* y, int nPoints) {
		util::CallVFunc<void>(this, 20, x, y, nPoints);
	}

	void DrawSetTextFont(HFont hFont) {
		util::CallVFunc<void>(this, 23, hFont);
	}

	void DrawSetTextColor(Color color) {
		util::CallVFunc<void>(this, 24, color);
	}

	void DrawSetTextColor(int r, int g, int b, int a) {
		util::CallVFunc<void>(this, 25, r, g, b, a);
	}

	void DrawSetTextPos(int x, int y) {
		util::CallVFunc<void>(this, 26, x, y);
	}

	void DrawPrintText(const wchar_t* wszText, int nTextLength, EFontDrawType drawType = FONT_DRAW_DEFAULT) {
		util::CallVFunc<void>(this, 28, wszText, nTextLength, drawType);
	}

	void DrawSetTextureRGBA(int nIndex, const unsigned char* rgba, int iWide, int iTall) {
		util::CallVFunc<void>(this, 37, nIndex, rgba, iWide, iTall);
	}

	void DrawSetTexture(int nIndex) {
		util::CallVFunc<void>(this, 38, nIndex);
	}

	int CreateNewTextureID(bool bProcedural = false) {
		return util::CallVFunc<int>(this, 43, bProcedural);
	}

	void UnLockCursor() {
		return util::CallVFunc<void>(this, 66);
	}

	void LockCursor() {
		return util::CallVFunc<void>(this, 67);
	}

	HFont FontCreate() {
		return util::CallVFunc<HFont>(this, 71);
	}

	bool SetFontGlyphSet(HFont hFont, const char* szWindowsFontName, int iTall, int iWeight, int iBlur, int nScanLines, int iFlags, int nRangeMin = 0, int nRangeMax = 0) {
		return util::CallVFunc<bool>(this, 72, hFont, szWindowsFontName, iTall, iWeight, iBlur, nScanLines, iFlags, nRangeMin, nRangeMax);
	}

	void GetTextSize(HFont hFont, const wchar_t* wszText, int& iWide, int& iTall) {
		util::CallVFunc<void>(this, 79, hFont, wszText, std::ref(iWide), std::ref(iTall));
	}

	void PlaySoundSurface(const char* szFileName) {
		util::CallVFunc<void>(this, 82, szFileName);
	}

	void DrawOutlinedCircle(int x, int y, int iRadius, int nSegments) {
		util::CallVFunc<void>(this, 103, x, y, iRadius, nSegments);
	}

	void DrawTexturedPolygon(int n, Vertex_t* pVertice, bool bClipVertices = true) {
		util::CallVFunc<void>(this, 106, n, pVertice, bClipVertices);
	}
};