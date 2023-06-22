#pragma once
#include <format>
#include "../../SDK/DataTyes/Color.h"
#include <codecvt>

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
class ISurface : ROP::VirtualCallable_t<ROP::ClientGadget_t> {

public:

	void DrawT(int X, int Y, Color Color, HFont Font, bool Center, const char* _Input, ...) noexcept {

		int w, h;
		static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		static std::wstring text;
		try {
			text = converter.from_bytes(_Input);
		}
		catch (std::range_error){
			util::LogConsole(std::format("Name: {} cannot be converted (not utf-8)\n", _Input).c_str());
			return;
		}

		GetTextSize(Font, text.c_str(), w, h);
		DrawSetTextFont(Font);
		DrawSetTextColor(Color);
		DrawSetTextPos(Center ? X - w / 2 : X, Y);
		DrawPrintText(text.c_str(), text.size());
	}

	void DrawSetColor(Color colDraw)
	{
		CallVFunc<void, 14U>(this, colDraw);
	}

	void DrawSetColor(int r, int g, int b, int a)
	{
		CallVFunc<void, 15U>(this, r, g, b, a);
	}

	void DrawFilledRect(int x0, int y0, int x1, int y1)
	{
		CallVFunc<void, 16U>(this, x0, y0, x1, y1);
	}

	void DrawOutlinedRect(int x0, int y0, int x1, int y1)
	{
		CallVFunc<void, 18U>(this, x0, y0, x1, y1);
	}

	void DrawLine(int x0, int y0, int x1, int y1)
	{
		CallVFunc<void, 19U>(this, x0, y0, x1, y1);
	}

	void DrawPolyLine(int* x, int* y, int nPoints)
	{
		CallVFunc<void, 20U>(this, x, y, nPoints);
	}

	void DrawSetTextFont(HFont hFont)
	{
		CallVFunc<void, 23U>(this, hFont);
	}

	void DrawSetTextColor(Color color)
	{
		CallVFunc<void, 24U>(this, color);
	}

	void DrawSetTextColor(int r, int g, int b, int a)
	{
		CallVFunc<void, 25U>(this, r, g, b, a);
	}

	void DrawSetTextPos(int x, int y)
	{
		CallVFunc<void, 26U>(this, x, y);
	}

	void DrawPrintText(const wchar_t* wszText, int nTextLength, EFontDrawType drawType = FONT_DRAW_DEFAULT)
	{
		CallVFunc<void, 28U>(this, wszText, nTextLength, drawType);
	}

	void DrawSetTextureRGBA(int nIndex, const unsigned char* arrRGBA, int iWide, int iTall)
	{
		CallVFunc<void, 37U>(this, nIndex, arrRGBA, iWide, iTall);
	}

	void DrawSetTexture(int nIndex)
	{
		CallVFunc<void, 38U>(this, nIndex);
	}

	int CreateNewTextureID(bool bProcedural = false)
	{
		return CallVFunc<int, 43U>(this, bProcedural);
	}

	void UnLockCursor()
	{
		CallVFunc<void, 66U>(this);
	}

	void LockCursor()
	{
		CallVFunc<void, 67U>(this);
	}

	HFont CreateFontGame()
	{
		return CallVFunc<HFont, 71U>(this);
	}

	bool SetFontGlyphSet(HFont hFont, const char* szWindowsFontName, int iTall, int iWeight, int iBlur, int nScanLines, int iFlags, int nRangeMin = 0, int nRangeMax = 0)
	{
		return CallVFunc<bool, 72U>(this, hFont, szWindowsFontName, iTall, iWeight, iBlur, nScanLines, iFlags, nRangeMin, nRangeMax);
	}

	void GetTextSize(HFont hFont, const wchar_t* wszText, int& iWide, int& iTall)
	{
		CallVFunc<void, 79U>(this, hFont, wszText, &iWide, &iTall);
	}

	void PlaySound(const char* szFileName)
	{
		CallVFunc<void, 82U>(this, szFileName);
	}

	void DrawOutlinedCircle(int x, int y, int iRadius, int nSegments)
	{
		CallVFunc<void, 103U>(this, x, y, iRadius, nSegments);
	}

	void DrawTexturedPolygon(int nCount, Vertex_t* pVertices, bool bClipVertices = true)
	{
		CallVFunc<void, 106U>(this, nCount, pVertices, bClipVertices);
	}

	void DrawFilledRectFade(int x0, int y0, int x1, int y1, std::uint32_t uAlpha0, std::uint32_t uAlpha1, bool bHorizontal)
	{
		CallVFunc<void, 123U>(this, x0, y0, x1, y1, uAlpha0, uAlpha1, bHorizontal);
	}

	void GetClipRect(int& x0, int& y0, int& x1, int& y1)
	{
		CallVFunc<void, 146U>(this, &x0, &y0, &x1, &y1);
	}

	void SetClipRect(int x0, int y0, int x1, int y1)
	{
		CallVFunc<void, 147U>(this, x0, y0, x1, y1);
	}
};