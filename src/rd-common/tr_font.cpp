/*
===========================================================================
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2013 - 2015, OpenJK contributors

This file is part of the OpenJK source code.

OpenJK is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

#include "server/exe_headers.h"

#include <limits.h>
#include <string>
#include "qcommon/sstring.h"	// stl string class won't compile in here (MS shite), so use Gil's.
#include "tr_local.h"
#include "tr_font.h"

#include "qcommon/strippublic.h"

cvar_t *r_fontSharpness;

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// This file is shared in the single and multiplayer codebases, so be CAREFUL WHAT YOU ADD/CHANGE!!!!!
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum
{
	eWestern,	// ( I only care about asian languages in here at the moment )
	eKorean,
	eTaiwanese,	// 15x15 glyphs tucked against BR of 16x16 space
	eJapanese,	// 15x15 glyphs tucked against TL of 16x16 space
} Language_e;

// this is to cut down on all the stupid string compares I've been doing, and convert asian stuff to switch-case
//
static Language_e GetLanguageEnum()
{
	static Language_e	eLanguage = eWestern;
	if ( !sp_language )
		return eLanguage;
	else if ( sp_language->integer == SP_LANGUAGE_KOREAN )
		eLanguage = eKorean;
	else if ( sp_language->integer == SP_LANGUAGE_JAPANESE )
		eLanguage = eJapanese;
	else if ( sp_language->integer == SP_LANGUAGE_TAIWANESE )
		eLanguage = eTaiwanese;

	return eLanguage;
}

#define GLYPH_MAX_KOREAN_SHADERS	3
#define GLYPH_MAX_TAIWANESE_SHADERS 4
#define GLYPH_MAX_JAPANESE_SHADERS	3
#define GLYPH_MAX_CHINESE_SHADERS	3
#define GLYPH_MAX_THAI_SHADERS		3
#define GLYPH_MAX_ASIAN_SHADERS		4	// this MUST equal the larger of the above defines

#define MAX_FONT_VARIANTS 8

class CFontInfo
{
private:
	// From the fontdat file
	glyphInfo_t		mGlyphs[GLYPH_COUNT];

//	int				mAsianHack;				// unused junk from John's fontdat file format.
	// end of fontdat data

	int				mShader;   				// handle to the shader with the glyph

	int				m_hAsianShaders[GLYPH_MAX_ASIAN_SHADERS];	// shaders for Korean glyphs where applicable
	glyphInfo_t		m_AsianGlyph;			// special glyph containing asian->western scaling info for all glyphs
	int				m_iAsianGlyphsAcross;	// needed to dynamically calculate S,T coords
	int				m_iAsianPagesLoaded;
	bool			m_bAsianLastPageHalfHeight;
	int				m_iAsianLanguageLoaded;	// doesn't matter what this is, so long as it's comparable as being changed

public:
	char			m_sFontName[MAX_QPATH];	// eg "fonts/lcd"	// needed for korean font-hint if we need >1 hangul set
	int				mPointSize;
	int				mHeight;
	int				mAscender;
	int				mDescender;

	bool			mbRoundCalcs;	// trying to make this !@#$%^ thing work with scaling
	bool			m_bIsFakeAlienLanguage;	// ... if true, don't process as MBCS or override as SBCS etc

	CFontInfo		*m_variants[MAX_FONT_VARIANTS];
	int				m_numVariants;
	int				m_handle;
	qboolean		m_isVariant;

	CFontInfo(const char *fontName);
//	CFontInfo(int fill) { memset(this, fill, sizeof(*this)); }	// wtf?
	~CFontInfo(void) {}

	int GetPointSize(void) const { return(mPointSize); }
	int GetHeight(void) const { return(mHeight); }
	int GetAscender(void) const { return(mAscender); }
	int GetDescender(void) const { return(mDescender); }

	const glyphInfo_t *GetLetter(const unsigned int uiLetter, int *piShader = NULL);
	int GetCollapsedAsianCode(ulong uiLetter) const;

	int GetLetterWidth(const unsigned int uiLetter);
	int GetLetterHorizAdvance(const unsigned int uiLetter);
	int GetShader(void) const { return(mShader); }

	void FlagNoAsianGlyphs(void) { m_hAsianShaders[0] = 0; m_iAsianLanguageLoaded = -1; }	// used during constructor
	bool AsianGlyphsAvailable(void) const { return !!(m_hAsianShaders[0]); }

	void UpdateAsianIfNeeded( bool bForceReEval = false);

	int GetHandle();

	void AddVariant(CFontInfo *variant);
	int GetNumVariants();
	CFontInfo *GetVariant(int index);
};

//================================================




// round float to one decimal place...
//
float RoundTenth( float fValue )
{
	return ( floorf( (fValue*10.0f) + 0.5f) ) / 10.0f;
}


int							g_iCurrentFontIndex;	// entry 0 is reserved index for missing/invalid, else ++ with each new font registered
std::vector<CFontInfo *>			g_vFontArray;
typedef std::map<sstring_t, int>	FontIndexMap_t;
FontIndexMap_t g_mapFontIndexes;
int g_iNonScaledCharRange;	// this is used with auto-scaling of asian fonts, anything below this number is preserved in scale, anything above is scaled down by 0.75f

//paletteRGBA_c				lastcolour;

// =============================== some korean stuff =======================================

#define KSC5601_HANGUL_HIBYTE_START		0xB0	// range is...
#define KSC5601_HANGUL_HIBYTE_STOP		0xC8	// ... inclusive
#define KSC5601_HANGUL_LOBYTE_LOBOUND	0xA0	// range is...
#define KSC5601_HANGUL_LOBYTE_HIBOUND	0xFF	// ...bounding (ie only valid in between these points, but NULLs in charsets for these codes)
#define KSC5601_HANGUL_CODES_PER_ROW	96		// 2 more than the number of glyphs

extern qboolean Language_IsKorean( void );

static inline qboolean Korean_ValidKSC5601Hangul( byte _iHi, byte _iLo )
{
	return (qboolean)(
		_iHi >=KSC5601_HANGUL_HIBYTE_START		&&
		_iHi <=KSC5601_HANGUL_HIBYTE_STOP		&&
		_iLo > KSC5601_HANGUL_LOBYTE_LOBOUND	&&
		_iLo < KSC5601_HANGUL_LOBYTE_HIBOUND);
}

static inline qboolean Korean_ValidKSC5601Hangul( unsigned int uiCode )
{
	return Korean_ValidKSC5601Hangul( uiCode >> 8, uiCode & 0xFF );
}


// takes a KSC5601 double-byte hangul code and collapses down to a 0..n glyph index...
// Assumes rows are 96 wide (glyph slots), not 94 wide (actual glyphs), so I can ignore boundary markers
//
// (invalid hangul codes will return 0)
//
static int Korean_CollapseKSC5601HangulCode(unsigned int uiCode)
{
	if (Korean_ValidKSC5601Hangul( uiCode ))
	{
		uiCode -= (KSC5601_HANGUL_HIBYTE_START * 256) + KSC5601_HANGUL_LOBYTE_LOBOUND;	// sneaky maths on both bytes, reduce to 0x0000 onwards
		uiCode  = ((uiCode >> 8) * KSC5601_HANGUL_CODES_PER_ROW) + (uiCode & 0xFF);
		return uiCode;
	}
	return 0;
}

static int Korean_InitFields(int &iGlyphTPs, const char *&psLang)
{
	psLang		= "kor";
	iGlyphTPs	= GLYPH_MAX_KOREAN_SHADERS;
	g_iNonScaledCharRange = 255;
	return 32;	// m_iAsianGlyphsAcross
}

// ======================== some taiwanese stuff ==============================

// (all ranges inclusive for Big5)...
//
#define BIG5_HIBYTE_START0		0xA1	// (misc chars + level 1 hanzi)
#define BIG5_HIBYTE_STOP0		0xC6	//
#define BIG5_HIBYTE_START1		0xC9	// (level 2 hanzi)
#define BIG5_HIBYTE_STOP1		0xF9	//
#define BIG5_LOBYTE_LOBOUND0	0x40	//
#define BIG5_LOBYTE_HIBOUND0	0x7E	//
#define BIG5_LOBYTE_LOBOUND1	0xA1	//
#define BIG5_LOBYTE_HIBOUND1	0xFE	//
#define BIG5_CODES_PER_ROW		160		// 3 more than the number of glyphs

extern qboolean Language_IsTaiwanese( void );

static qboolean Taiwanese_ValidBig5Code( unsigned int uiCode )
{
	const byte _iHi = (uiCode >> 8)&0xFF;
	if (	(_iHi >= BIG5_HIBYTE_START0 && _iHi <= BIG5_HIBYTE_STOP0)
		||	(_iHi >= BIG5_HIBYTE_START1 && _iHi <= BIG5_HIBYTE_STOP1)
		)
	{
		const byte _iLo = uiCode & 0xFF;

		if ( (_iLo >= BIG5_LOBYTE_LOBOUND0 && _iLo <= BIG5_LOBYTE_HIBOUND0) ||
			 (_iLo >= BIG5_LOBYTE_LOBOUND1 && _iLo <= BIG5_LOBYTE_HIBOUND1)
			)
		{
			return qtrue;
		}
	}

	return qfalse;
}


// only call this when Taiwanese_ValidBig5Code() has already returned true...
//
static qboolean Taiwanese_IsTrailingPunctuation( unsigned int uiCode )
{
	// so far I'm just counting the first 21 chars, those seem to be all the basic punctuation...
	//
	if (	uiCode >= ((BIG5_HIBYTE_START0<<8)|BIG5_LOBYTE_LOBOUND0) &&
			uiCode <  (((BIG5_HIBYTE_START0<<8)|BIG5_LOBYTE_LOBOUND0)+20)
		)
	{
		return qtrue;
	}

	return qfalse;
}


// takes a BIG5 double-byte code (including level 2 hanzi) and collapses down to a 0..n glyph index...
// Assumes rows are 160 wide (glyph slots), not 157 wide (actual glyphs), so I can ignore boundary markers
//
// (invalid big5 codes will return 0)
//
static int Taiwanese_CollapseBig5Code( unsigned int uiCode )
{
	if (Taiwanese_ValidBig5Code( uiCode ))
	{
		uiCode -= (BIG5_HIBYTE_START0 * 256) + BIG5_LOBYTE_LOBOUND0;	// sneaky maths on both bytes, reduce to 0x0000 onwards
		if ( (uiCode & 0xFF) >= (BIG5_LOBYTE_LOBOUND1-1)-BIG5_LOBYTE_LOBOUND0)
		{
			uiCode -= ((BIG5_LOBYTE_LOBOUND1-1) - (BIG5_LOBYTE_HIBOUND0+1)) -1;
		}
		uiCode = ((uiCode >> 8) * BIG5_CODES_PER_ROW) + (uiCode & 0xFF);
		return uiCode;
	}
	return 0;
}

static int Taiwanese_InitFields(int &iGlyphTPs, const char *&psLang)
{
	psLang		= "tai";
	iGlyphTPs	= GLYPH_MAX_TAIWANESE_SHADERS;
	g_iNonScaledCharRange = 255;
	return 64;	// m_iAsianGlyphsAcross
}

// ======================== some Japanese stuff ==============================


// ( all ranges inclusive for Shift-JIS )
//
#define SHIFTJIS_HIBYTE_START0	0x81
#define SHIFTJIS_HIBYTE_STOP0	0x9F
#define SHIFTJIS_HIBYTE_START1	0xE0
#define SHIFTJIS_HIBYTE_STOP1	0xEF
//
#define SHIFTJIS_LOBYTE_START0	0x40
#define SHIFTJIS_LOBYTE_STOP0	0x7E
#define SHIFTJIS_LOBYTE_START1	0x80
#define SHIFTJIS_LOBYTE_STOP1	0xFC
#define SHIFTJIS_CODES_PER_ROW	(((SHIFTJIS_LOBYTE_STOP0-SHIFTJIS_LOBYTE_START0)+1)+((SHIFTJIS_LOBYTE_STOP1-SHIFTJIS_LOBYTE_START1)+1))


extern qboolean Language_IsJapanese( void );

static qboolean Japanese_ValidShiftJISCode( byte _iHi, byte _iLo )
{
	if (	(_iHi >= SHIFTJIS_HIBYTE_START0 && _iHi <= SHIFTJIS_HIBYTE_STOP0)
		||	(_iHi >= SHIFTJIS_HIBYTE_START1 && _iHi <= SHIFTJIS_HIBYTE_STOP1)
		)
	{
		if ( (_iLo >= SHIFTJIS_LOBYTE_START0 && _iLo <= SHIFTJIS_LOBYTE_STOP0) ||
			 (_iLo >= SHIFTJIS_LOBYTE_START1 && _iLo <= SHIFTJIS_LOBYTE_STOP1)
			)
		{
			return qtrue;
		}
	}

	return qfalse;
}

static inline qboolean Japanese_ValidShiftJISCode( unsigned int uiCode )
{
	return Japanese_ValidShiftJISCode( uiCode >> 8, uiCode & 0xFF );
}


// only call this when Japanese_ValidShiftJISCode() has already returned true...
//
static qboolean Japanese_IsTrailingPunctuation( unsigned int uiCode )
{
	// so far I'm just counting the first 18 chars, those seem to be all the basic punctuation...
	//
	if (	uiCode >= ((SHIFTJIS_HIBYTE_START0<<8)|SHIFTJIS_LOBYTE_START0) &&
			uiCode <  (((SHIFTJIS_HIBYTE_START0<<8)|SHIFTJIS_LOBYTE_START0)+18)
		)
	{
		return qtrue;
	}

	return qfalse;
}


// takes a ShiftJIS double-byte code and collapse down to a 0..n glyph index...
//
// (invalid codes will return 0)
//
static int Japanese_CollapseShiftJISCode( unsigned int uiCode )
{
	if (Japanese_ValidShiftJISCode( uiCode ))
	{
		uiCode -= ((SHIFTJIS_HIBYTE_START0<<8)|SHIFTJIS_LOBYTE_START0);	// sneaky maths on both bytes, reduce to 0x0000 onwards

		if ( (uiCode & 0xFF) >= (SHIFTJIS_LOBYTE_START1)-SHIFTJIS_LOBYTE_START0)
		{
			uiCode -= ((SHIFTJIS_LOBYTE_START1)-SHIFTJIS_LOBYTE_STOP0)-1;
		}

		if ( ((uiCode>>8)&0xFF) >= (SHIFTJIS_HIBYTE_START1)-SHIFTJIS_HIBYTE_START0)
		{
			uiCode -= (((SHIFTJIS_HIBYTE_START1)-SHIFTJIS_HIBYTE_STOP0)-1) << 8;
		}

		uiCode = ((uiCode >> 8) * SHIFTJIS_CODES_PER_ROW) + (uiCode & 0xFF);

		return uiCode;
	}
	return 0;
}


static int Japanese_InitFields(int &iGlyphTPs, const char *&psLang)
{
	psLang		= "jap";
	iGlyphTPs	= GLYPH_MAX_JAPANESE_SHADERS;
	g_iNonScaledCharRange = 255;
	return 64;	// m_iAsianGlyphsAcross
}

// ============================================================================

// takes char *, returns integer char at that point, and advances char * on by enough bytes to move
//	past the letter (either western 1 byte or Asian multi-byte)...
//
// looks messy, but the actual execution route is quite short, so it's fast...
//
// Note that I have to have this 3-param form instead of advancing a passed-in "const char **psText" because of VM-crap where you can only change ptr-contents, not ptrs themselves. Bleurgh. Ditto the qtrue:qfalse crap instead of just returning stuff straight through.
//
unsigned int AnyLanguage_ReadCharFromString( char *psText, int *piAdvanceCount, qboolean *pbIsTrailingPunctuation /* = NULL */)
{
	// JK2 does this func a little differently --eez
	const byte *psString = (const byte *) psText;	// avoid sign-promote bug
	unsigned int uiLetter;

	if ( Language_IsKorean() )
	{
		if ( Korean_ValidKSC5601Hangul( psString[0], psString[1] ))
		{
			uiLetter = (psString[0] * 256) + psString[1];
			psText += 2;
			*piAdvanceCount = 2;

			// not going to bother testing for korean punctuation here, since korean already
			//	uses spaces, and I don't have the punctuation glyphs defined, only the basic 2350 hanguls
			//
			if ( pbIsTrailingPunctuation)
			{
				*pbIsTrailingPunctuation = qfalse;
			}

			return uiLetter;
		}
	}
	else
	if ( Language_IsTaiwanese() )
	{
		if ( Taiwanese_ValidBig5Code( (psString[0] * 256) + psString[1] ))
		{
			uiLetter = (psString[0] * 256) + psString[1];
			psText += 2;
			*piAdvanceCount = 2;

			// need to ask if this is a trailing (ie like a comma or full-stop) punctuation?...
			//
			if ( pbIsTrailingPunctuation)
			{
				*pbIsTrailingPunctuation = Taiwanese_IsTrailingPunctuation( uiLetter );
			}

			return uiLetter;
		}
	}
	else
	if ( Language_IsJapanese() )
	{
		if ( Japanese_ValidShiftJISCode( psString[0], psString[1] ))
		{
			uiLetter = (psString[0] * 256) + psString[1];
			psText += 2;
			*piAdvanceCount = 2;

			// need to ask if this is a trailing (ie like a comma or full-stop) punctuation?...
			//
			if ( pbIsTrailingPunctuation)
			{
				*pbIsTrailingPunctuation = Japanese_IsTrailingPunctuation( uiLetter );
			}

			return uiLetter;
		}
	}

	// ... must not have been an MBCS code...
	//
	uiLetter = psString[0];
	psText += 1;	// NOT ++
	*piAdvanceCount = 1;

	if (pbIsTrailingPunctuation)
	{
		*pbIsTrailingPunctuation = (qboolean)(
			uiLetter == '!' ||
			uiLetter == '?' ||
			uiLetter == ',' ||
			uiLetter == '.' ||
			uiLetter == ';' ||
			uiLetter == ':');
	}

	return uiLetter;
}

unsigned int AnyLanguage_ReadCharFromString( char **psText, qboolean *pbIsTrailingPunctuation /* = NULL */)
{
	int advance = 0;
	unsigned int advance2 = AnyLanguage_ReadCharFromString (*psText, &advance, pbIsTrailingPunctuation);
	*psText += advance;

	return advance2;
}

// needed for subtitle printing since original code no longer worked once camera bar height was changed to 480/10
//	rather than refdef height / 10. I now need to bodge the coords to come out right.
//
qboolean Language_IsAsian(void)
{
	switch ( GetLanguageEnum() )
	{
		case eKorean:
		case eTaiwanese:
		case eJapanese:
			return qtrue;
		default:
			break;
	}

	return qfalse;
}

qboolean Language_UsesSpaces(void)
{
	// ( korean uses spaces )
	switch ( GetLanguageEnum() )
	{
		case eTaiwanese:
		case eJapanese:
			return qfalse;
		default:
			break;
	}

	return qtrue;
}

// ======================================================================
// name is (eg) "ergo" or "lcd", no extension.
//
//  If path present, it's a special language hack for SBCS override languages, eg: "lcd/russian", which means
//	  just treat the file as "russian", but with the "lcd" part ensuring we don't find a different registered russian font
//
static const char *FontDatPath( const char *_fontName ) {
	static char fontName[MAX_QPATH];
	sprintf( fontName,"fonts/%s.fontdat",COM_SkipPath(const_cast<char*>(_fontName)) );	// COM_SkipPath should take a const char *, but it's just possible people use it as a char * I guess, so I have to hack around like this <groan>
	return fontName;
}
CFontInfo::CFontInfo(const char *_fontName)
{
	int			len, i;
	void		*buff;
	dfontdat_t	*fontdat;

	// remove any special hack name insertions...
	//
	const char *fontName = FontDatPath( _fontName );

	// clear some general things...
	//
	m_bIsFakeAlienLanguage = !strcmp(_fontName,"aurabesh");	// dont try and make SBCS or asian overrides for this
	m_isVariant = qfalse;

	len = ri.FS_ReadFile(fontName, NULL);
	if (len == sizeof(dfontdat_t))
	{
		ri.FS_ReadFile(fontName, &buff);
		fontdat = (dfontdat_t *)buff;

		for(i = 0; i < GLYPH_COUNT; i++)
		{
#ifdef Q3_BIG_ENDIAN
			mGlyphs[i].width = LittleShort(fontdat->mGlyphs[i].width);
			mGlyphs[i].height = LittleShort(fontdat->mGlyphs[i].height);
			mGlyphs[i].horizAdvance = LittleShort(fontdat->mGlyphs[i].horizAdvance);
			mGlyphs[i].horizOffset = LittleShort(fontdat->mGlyphs[i].horizOffset);
			mGlyphs[i].baseline = LittleLong(fontdat->mGlyphs[i].baseline);
			mGlyphs[i].s = LittleFloat(fontdat->mGlyphs[i].s);
			mGlyphs[i].t = LittleFloat(fontdat->mGlyphs[i].t);
			mGlyphs[i].s2 = LittleFloat(fontdat->mGlyphs[i].s2);
			mGlyphs[i].t2 = LittleFloat(fontdat->mGlyphs[i].t2);
#else
			mGlyphs[i] = fontdat->mGlyphs[i];
#endif
		}
		mPointSize = LittleShort(fontdat->mPointSize);
		mHeight = LittleShort(fontdat->mHeight);
		mAscender = LittleShort(fontdat->mAscender);
		mDescender = LittleShort(fontdat->mDescender);
//		mAsianHack = LittleShort(fontdat->mKoreanHack);	// ignore this crap, it's some junk in the fontdat file that no-one uses
		mbRoundCalcs = false /*!!strstr(fontName,"ergo")*/;

		// cope with bad fontdat headers...
		//
		if (mHeight == 0)
		{
			mHeight = mPointSize;
            mAscender = mPointSize - Round( ((float)mPointSize/10.0f)+2 );	// have to completely guess at the baseline... sigh.
            mDescender = mHeight - mAscender;
		}

		ri.FS_FreeFile(buff);
	}
	else
	{
		mHeight = 0;
		mShader = 0;
	}

	Q_strncpyz(m_sFontName, fontName, sizeof(m_sFontName));
	COM_StripExtension( m_sFontName, m_sFontName, sizeof(m_sFontName) );	// so we get better error printing if failed to load shader (ie lose ".fontdat")
	mShader = RE_RegisterShaderNoMip(m_sFontName);

	FlagNoAsianGlyphs();
	UpdateAsianIfNeeded(true);

	// finished...
	g_vFontArray.resize(g_iCurrentFontIndex + 1);
	m_handle = g_iCurrentFontIndex;
	g_vFontArray[g_iCurrentFontIndex++] = this;


	extern cvar_t *com_buildScript;
	if (com_buildScript->integer == 2)
	{
		Com_Printf( "com_buildScript(2): Registering foreign fonts...\n" );
		static qboolean bDone = qfalse;	// Do this once only (for speed)...
		if (!bDone)
		{
			bDone = qtrue;

			char sTemp[MAX_QPATH];
			int iGlyphTPs = 0;
			const char *psLang = NULL;

			// SBCS override languages...
			//
			fileHandle_t f;
			// asian MBCS override languages...
			//
			for (int iLang=0; iLang<3; iLang++)
			{
				switch (iLang)
				{
					case 0:	m_iAsianGlyphsAcross = Korean_InitFields	(iGlyphTPs, psLang);	break;
					case 1: m_iAsianGlyphsAcross = Taiwanese_InitFields	(iGlyphTPs, psLang);	break;
					case 2: m_iAsianGlyphsAcross = Japanese_InitFields	(iGlyphTPs, psLang);	break;
				}

				for (int i=0; i<iGlyphTPs; i++)
				{
					Com_sprintf(sTemp,sizeof(sTemp), "fonts/%s_%d_1024_%d.tga", psLang, 1024/m_iAsianGlyphsAcross, i);

					// RE_RegisterShaderNoMip( sTemp );	// don't actually need to load it, so...
					ri.FS_FOpenFileRead( sTemp, &f, qfalse );
					if (f) {
						ri.FS_FCloseFile( f );
					}
				}
			}
		}
	}

	m_numVariants = 0;
}

int CFontInfo::GetHandle() {
	return m_handle;
}

void CFontInfo::AddVariant(CFontInfo * replacer) {
	m_variants[m_numVariants++] = replacer;
}

int CFontInfo::GetNumVariants() {
	return m_numVariants;
}

CFontInfo *CFontInfo::GetVariant(int index) {
	return m_variants[index];
}

void CFontInfo::UpdateAsianIfNeeded( bool bForceReEval /* = false */ )
{
	// if asian language, then provide an alternative glyph set and fill in relevant fields...
	//
	if (mHeight && !m_bIsFakeAlienLanguage)	// western charset exists in first place, and isn't alien rubbish?
	{
		Language_e eLanguage = GetLanguageEnum();

		if (eLanguage == eKorean || eLanguage == eTaiwanese || eLanguage == eJapanese)
		{
			int iCappedHeight = mHeight < 16 ? 16: mHeight;	// arbitrary limit on small char sizes because Asian chars don't squash well

			const int iThisLanguage = Language_GetIntegerValue();
			if (m_iAsianLanguageLoaded != iThisLanguage || !AsianGlyphsAvailable() || bForceReEval)
			{
				m_iAsianLanguageLoaded  = iThisLanguage;

				int iGlyphTPs = 0;
				const char *psLang = NULL;

				switch ( eLanguage )
				{
					case eKorean:		m_iAsianGlyphsAcross = Korean_InitFields(iGlyphTPs, psLang);	break;
					case eTaiwanese:	m_iAsianGlyphsAcross = Taiwanese_InitFields(iGlyphTPs, psLang);	break;
					case eJapanese:		m_iAsianGlyphsAcross = Japanese_InitFields(iGlyphTPs, psLang);	break;
					default:
					break;
				}

				// textures need loading...
				//
				if (m_sFontName[0])
				{
					// Use this sometime if we need to do logic to load alternate-height glyphs to better fit other fonts.
					// (but for now, we just use the one glyph set)
					//
				}

				for (int i = 0; i < iGlyphTPs; i++)
				{
					// (Note!!  assumption for S,T calculations: all Asian glyph textures pages are square except for last one)
					//
					char sTemp[MAX_QPATH];
					Com_sprintf(sTemp,sizeof(sTemp), "fonts/%s_%d_1024_%d", psLang, 1024/m_iAsianGlyphsAcross, i);
					//
					// returning 0 here will automatically inhibit Asian glyph calculations at runtime...
					//
					m_hAsianShaders[i] = RE_RegisterShaderNoMip( sTemp );
				}

				// for now I'm hardwiring these, but if we ever have more than one glyph set per language then they'll be changed...
				//
				m_iAsianPagesLoaded = iGlyphTPs;	// not necessarily true, but will be safe, and show up obvious if something missing
				m_bAsianLastPageHalfHeight = true;

				bForceReEval = true;
			}

			if (bForceReEval)
			{
				// now init the Asian member glyph fields to make them come out the same size as the western ones
				//	that they serve as an alternative for...
				//
				m_AsianGlyph.width			= iCappedHeight;	// square Asian chars same size as height of western set
				m_AsianGlyph.height			= iCappedHeight;	// ""
				switch (eLanguage)
				{
					default:		m_AsianGlyph.horizAdvance	= iCappedHeight;	break;
					case eKorean:	m_AsianGlyph.horizAdvance	= iCappedHeight - 1;break;	// korean has a small amount of space at the edge of the glyph

					case eTaiwanese:
					case eJapanese:
									m_AsianGlyph.horizAdvance	= iCappedHeight + 3;	// need to force some spacing for these
//					case eThai:	// this is done dynamically elsewhere, since Thai glyphs are variable width
				}
				m_AsianGlyph.horizOffset	= 0;				// ""
				m_AsianGlyph.baseline		= mAscender + ((iCappedHeight - mHeight) >> 1);
			}
		}
		else
		{
			// not using Asian...
			//
			FlagNoAsianGlyphs();
		}
	}
	else
	{
		// no western glyphs available, so don't attempt to match asian...
		//
		FlagNoAsianGlyphs();
	}
}

static CFontInfo *GetFont_Actual(int index)
{
	index &= SET_MASK;
	if((index >= 1) && (index < g_iCurrentFontIndex))
	{
		CFontInfo *pFont = g_vFontArray[index];

		if (pFont)
		{
			pFont->UpdateAsianIfNeeded();
		}

		return pFont;
	}
	return(NULL);
}

static CFontInfo *RE_Font_GetVariant(CFontInfo *font, float *scale) {
	int variants = font->GetNumVariants();

	if (variants > 0) {
		CFontInfo *variant;
		int requestedSize = font->GetPointSize() * *scale *
			r_fontSharpness->value * (glConfig.vidHeight / SCREEN_HEIGHT);

		if (requestedSize <= font->GetPointSize())
			return font;

		for (int i = 0; i < variants; i++) {
			variant = font->GetVariant(i);

			if (requestedSize <= variant->GetPointSize())
				break;
		}

		*scale *= (float)font->GetPointSize() / variant->GetPointSize();
		return variant;
	}

	return font;
}

// needed to add *piShader param because of multiple TPs,
//	if not passed in, then I also skip S,T calculations for re-usable static asian glyphinfo struct...
//
const glyphInfo_t *CFontInfo::GetLetter(const unsigned int uiLetter, int *piShader /* = NULL */)
{
	if ( AsianGlyphsAvailable() )
	{
		int iCollapsedAsianCode = GetCollapsedAsianCode( uiLetter );
		if (iCollapsedAsianCode)
		{
			if (piShader)
			{
				// (Note!!  assumption for S,T calculations: all asian glyph textures pages are square except for last one
				//			which may or may not be half height) - but not for Thai
				//
				int iTexturePageIndex = iCollapsedAsianCode / (m_iAsianGlyphsAcross * m_iAsianGlyphsAcross);

				if (iTexturePageIndex > m_iAsianPagesLoaded)
				{
					Q_assert(0);				// should never happen
					iTexturePageIndex = 0;
				}

				iCollapsedAsianCode -= iTexturePageIndex *  (m_iAsianGlyphsAcross * m_iAsianGlyphsAcross);

				const int iColumn	= iCollapsedAsianCode % m_iAsianGlyphsAcross;
				const int iRow		= iCollapsedAsianCode / m_iAsianGlyphsAcross;
				const bool bHalfT	= (iTexturePageIndex == (m_iAsianPagesLoaded - 1) && m_bAsianLastPageHalfHeight);
				const int iAsianGlyphsDown = (bHalfT) ? m_iAsianGlyphsAcross / 2 : m_iAsianGlyphsAcross;

				switch ( GetLanguageEnum() )
				{
					case eKorean:
					default:
					{
						m_AsianGlyph.s  = (float)( iColumn    ) / (float)m_iAsianGlyphsAcross;
						m_AsianGlyph.t  = (float)( iRow       ) / (float)  iAsianGlyphsDown;
						m_AsianGlyph.s2 = (float)( iColumn + 1) / (float)m_iAsianGlyphsAcross;
						m_AsianGlyph.t2 = (float)( iRow + 1   ) / (float)  iAsianGlyphsDown;
					}
					break;

					case eTaiwanese:
					{
						m_AsianGlyph.s  = (float)(((1024 / m_iAsianGlyphsAcross) * ( iColumn    ))+1) / 1024.0f;
						m_AsianGlyph.t  = (float)(((1024 / iAsianGlyphsDown    ) * ( iRow       ))+1) / 1024.0f;
						m_AsianGlyph.s2 = (float)(((1024 / m_iAsianGlyphsAcross) * ( iColumn+1  ))  ) / 1024.0f;
						m_AsianGlyph.t2 = (float)(((1024 / iAsianGlyphsDown    ) * ( iRow+1     ))  ) / 1024.0f;
					}
					break;

					case eJapanese:
					{
						m_AsianGlyph.s  = (float)(((1024 / m_iAsianGlyphsAcross) * ( iColumn    ))  ) / 1024.0f;
						m_AsianGlyph.t  = (float)(((1024 / iAsianGlyphsDown    ) * ( iRow       ))  ) / 1024.0f;
						m_AsianGlyph.s2 = (float)(((1024 / m_iAsianGlyphsAcross) * ( iColumn+1  ))-1) / 1024.0f;
						m_AsianGlyph.t2 = (float)(((1024 / iAsianGlyphsDown    ) * ( iRow+1     ))-1) / 1024.0f;
					}
					break;
				}
				*piShader = m_hAsianShaders[ iTexturePageIndex ];
			}
			return &m_AsianGlyph;
		}
	}

	if (piShader)
	{
		*piShader = GetShader();
	}

	const glyphInfo_t *pGlyph = &mGlyphs[ uiLetter & 0xff ];
	//
	// SBCS language substitution?...
	//
	return pGlyph;
}

int CFontInfo::GetCollapsedAsianCode(ulong uiLetter) const
{
	int iCollapsedAsianCode = 0;

	if (AsianGlyphsAvailable())
	{
		switch ( GetLanguageEnum() )
		{
			case eKorean:		iCollapsedAsianCode = Korean_CollapseKSC5601HangulCode( uiLetter );	break;
			case eTaiwanese:	iCollapsedAsianCode = Taiwanese_CollapseBig5Code( uiLetter );		break;
			case eJapanese:		iCollapsedAsianCode = Japanese_CollapseShiftJISCode( uiLetter );	break;
			default:			Q_assert(0);	/* unhandled asian language */							break;
		}
	}

	return iCollapsedAsianCode;
}

int CFontInfo::GetLetterWidth(unsigned int uiLetter)
{
	const glyphInfo_t *pGlyph = GetLetter( uiLetter );
	return pGlyph->width ? pGlyph->width : mGlyphs[(unsigned)'.'].width;
}

int CFontInfo::GetLetterHorizAdvance(unsigned int uiLetter)
{
	const glyphInfo_t *pGlyph = GetLetter( uiLetter );
	return pGlyph->horizAdvance ? pGlyph->horizAdvance : mGlyphs[(unsigned)'.'].horizAdvance;
}

// ensure any GetFont calls that need SBCS overriding (such as when playing in Russian) have the appropriate stuff done...
//

CFontInfo *GetFont(int index)
{
	CFontInfo *pFont = GetFont_Actual( index );
	return pFont;
}


int RE_Font_StrLenPixels(const char *psText, const int iFontHandle, const float fScaleIn)
{
	float fScale = fScaleIn;
	// Yes..even this func is a little different, to the point where it doesn't work. --eez
	float		fMaxWidth = 0.0f;
	float		fThisWidth = 0.0f;
	CFontInfo	*curfont;

	curfont = GetFont(iFontHandle);
	if(!curfont)
	{
		return(0);
	}
	curfont = RE_Font_GetVariant(curfont, &fScale);

	float fScaleAsian = fScale;
	if (Language_IsAsian() && fScale > 0.7f )
	{
		fScaleAsian = fScale * 0.75f;
	}

	while(*psText)
	{
		unsigned int uiLetter = AnyLanguage_ReadCharFromString( (char **)&psText );
		if (uiLetter == 0x0A)
		{
			fThisWidth = 0.0f;
		}
		else
		{
			int iPixelAdvance = curfont->GetLetterHorizAdvance( uiLetter );

			float fValue = iPixelAdvance * ((uiLetter > 255) ? fScaleAsian : fScale);
			fThisWidth += curfont->mbRoundCalcs ? Round( fValue ) : fValue;
			if (fThisWidth > fMaxWidth)
			{
				fMaxWidth = fThisWidth;
			}
		}
	}

	// using ceil because we need to make sure that all the text is contained within the integer pixel width we're returning
	return (int)ceilf(fMaxWidth);
}

// not really a font function, but keeps naming consistant...
//
int RE_Font_StrLenChars(const char *psText)
{
	// logic for this function's letter counting must be kept same in this function and RE_Font_DrawString()
	//
	int iCharCount = 0;

	while ( *psText )
	{
		// in other words, colour codes and CR/LF don't count as chars, all else does...
		//
		int iAdvanceCount;
		unsigned int uiLetter = AnyLanguage_ReadCharFromString( (char *)psText, &iAdvanceCount, NULL );
		psText += iAdvanceCount;

		switch (uiLetter)
		{
			case '^':
				if (*psText >= '0' &&
					*psText <= '9')
				{
					psText++;
				}
				else
				{
					iCharCount++;
				}
				break;	// colour code (note next-char skip)
			case 10:								break;	// linefeed
			case 13:								break;	// return
			default:	iCharCount++;				break;
		}
	}

	return iCharCount;
}

int RE_Font_HeightPixels(const int iFontHandle, const float fScaleIn)
{
	float fScale = fScaleIn;
	CFontInfo	*curfont;

	curfont = GetFont(iFontHandle);
	if(curfont)
	{
		float fValue;
		curfont = RE_Font_GetVariant(curfont, &fScale);
		fValue = curfont->GetPointSize() * fScale;
		return curfont->mbRoundCalcs ? Round(fValue) : fValue;
	}
	return(0);
}

// iMaxPixelWidth is -1 for "all of string", else pixel display count...
//
void RE_Font_DrawString(int ox, int oy, const char *psText, const float *rgba, const int iFontHandleIn, int iMaxPixelWidth, const float fScaleIn)
{
	int iFontHandle = iFontHandleIn;
	float fScale = fScaleIn;
	// HAAAAAAAAAAAAAAAX..fix me please --eez
	static qboolean gbInShadow = qfalse;	// MUST default to this
	float				fox, foy, fx, fy;
	int					colour, offset;
	const glyphInfo_t	*pLetter;
	qhandle_t			hShader;

	Q_assert(psText);

	if(iFontHandle & STYLE_BLINK)
	{
		if((ri.Milliseconds() >> 7) & 1)
		{
			return;
		}
	}

/*	if (Language_IsTaiwanese())
	{
		psText = "Wp:\B6}\B7F\A7a \BFp\B7G\B4\B5\A1A\A7Ʊ\E6\A7A\B9\B3\A5L\AD̻\A1\AA\BA\A4@\BC˦\E6\A1C";
	}
	else
	if (Language_IsKorean())
	{
		psText = "Wp:\BC\EEŸ\C0\D3\C0̴\D9 \B8ָ\B0. \B1׵\E9\C0\CC \B8\BB\C7Ѵ\EB\B7\CE \B3װ\A1 \C0\DF\C7\D2\C1\F6 \B1\E2\B4\EB\C7ϰڴ\D9.";
	}
	else
	if (Language_IsJapanese())
	{
		char sBlah[200];
		sprintf(sBlah,va("%c%c %c%c %c%c %c%c",0x82,0xA9,0x82,0xC8,0x8A,0xBF,0x8E,0x9A));
		psText = &sBlah[0];
		//psText = \A1@\A1A\A1B\A1C\A1D\A1E\A1F\A1G\A1H\A1I\A1J\A1K\A1L\A1M\A1N\A1O\A1P\A1Q\A1R\A1S\A1T\A1U\A1V\A1W\A1X\A1Y\A1Z\A1[\A1\\A1]\A1^\A1_\A1`\A1a\A1b\A1c\A1d\A1e\A1f\A1g\A1h\A1i\A1j\A1k\A1l\A1m\A1n\A1o\A1p\A1q\A1r\A1s\A1t\A1u\A1v\A1w\A1x\A1y\A1z\A1{\A1|\A1}\A1~    \A1\A1\A1\A2\A1\A3\A1\A4\A1\A5\A1\A6\A1\A7\A1\A8\A1\A9\A1\AA\A1\AB\A1\AC\A1\AD\A1\AE\A1\AF\A1\B0\A1\B1\A1\B2\A1\B3\A1\B4\A1\B5\A1\B6\A1\B7\A1\B8\A1\B9\A1\BA\A1\BB\A1\BC\A1\BD\A1\BE\A1\BF\A1\C0\A1\C1\A1¡áġšơǡȡɡʡˡ̡͡ΡϡСѡҡӡԡա֡סء١ڡۡܡݡޡߡ\E0\A1\E1\A1\E2\A1\E3\A1\E4\A1\E5\A1\E6\A1\E7\A1\E8\A1\E9\A1\EA\A1\EB\A1\EC\A1\ED\A1\EE\A1\EF\A1\F0\A1\F1\A1\F2\A1\F3\A1\F4\A1\F5\A1\F6\A1\F7\A1\F8\A1\F9\A1\FA\A1\FB\A1\FC\A1\FD\A1\FE 1\A2@\A2A\A2B\A2C\A2D\A2E\A2F\A2G\A2H\A2I\A2J\A2K\A2L\A2M\A2N\A2O\A2P\A2Q\A2R\A2S\A2T\A2U\A2V\A2W\A2X\A2Y\A2Z\A2[\A2\\A2]\A2^\A2_\A2`\A2a\A2b\A2c\A2d\A2e\A2f\A2g\A2h\A2i\A2j\A2k\A2l\A2m\A2n\A2o\A2p\A2q\A2r\A2s\A2t\A2u\A2v\A2w\A2x\A2y\A2z\A2{\A2|\A2}\A2~    \A2\A1\A2\A2\A2\A3\A2\A4\A2\A5\A2\A6\A2\A7\A2\A8\A2\A9\A2\AA\A2\AB\A2\AC\A2\AD\A2\AE\A2\AF\A2\B0\A2\B1\A2\B2\A2\B3\A2\B4\A2\B5\A2\B6\A2\B7\A2\B8\A2\B9\A2\BA\A2\BB\A2\BC\A2\BD\A2\BE\A2\BF\A2\C0\A2\C1\A2¢âĢŢƢǢȢɢʢˢ̢͢΢ϢТѢҢӢԢբ֢עآ٢ڢۢܢݢޢߢ\E0\A2\E1\A2\E2\A2\E3\A2\E4\A2\E5\A2\E6\A2\E7\A2\E8\A2\E9\A2\EA\A2\EB\A2\EC\A2\ED\A2\EE\A2\EF\A2\F0\A2\F1\A2\F2\A2\F3\A2\F4\A2\F5\A2\F6\A2\F7\A2\F8\A2\F9\A2\FA\A2\FB\A2\FC\A2\FD\A2\FE 2\A3@\A3A\A3B\A3C\A3D\A3E\A3F\A3G\A3H\A3I\A3J\A3K\A3L\A3M\A3N\A3O\A3P\A3Q\A3R\A3S\A3T\A3U\A3V\A3W\A3X\A3Y\A3Z\A3[\A3\\A3]\A3^\A3_\A3`\A3a\A3b\A3c\A3d\A3e\A3f\A3g\A3h\A3i\A3j\A3k\A3l\A3m\A3n\A3o\A3p\A3q\A3r\A3s\A3t\A3u\A3v\A3w\A3x\A3y\A3z\A3{\A3|\A3}\A3~    \A3\A1\A3\A2\A3\A3\A3\A4\A3\A5\A3\A6\A3\A7\A3\A8\A3\A9\A3\AA\A3\AB\A3\AC\A3\AD\A3\AE\A3\AF\A3\B0\A3\B1\A3\B2\A3\B3\A3\B4\A3\B5\A3\B6\A3\B7\A3\B8\A3\B9\A3\BA\A3\BB\A3\BC\A3\BD\A3\BE\A3\BF\A3\C0\A3\C1\A3£ãģţƣǣȣɣʣˣ̣ͣΣϣУѣңӣԣգ֣ףأ٣ڣۣܣݣޣߣ\E0\A3\E1\A3\E2\A3\E3\A3\E4\A3\E5\A3\E6\A3\E7\A3\E8\A3\E9\A3\EA\A3\EB\A3\EC\A3\ED\A3\EE\A3\EF\A3\F0\A3\F1\A3\F2\A3\F3\A3\F4\A3\F5\A3\F6\A3\F7\A3\F8\A3\F9\A3\FA\A3\FB\A3\FC\A3\FD\A3\FE 3\A4@\A4A\A4B\A4C\A4D\A4E\A4F\A4G\A4H\A4I\A4J\A4K\A4L\A4M\A4N\A4O\A4P\A4Q\A4R\A4S\A4T\A4U\A4V\A4W\A4X\A4Y\A4Z\A4[\A4\\A4]\A4^\A4
	}
*/

	CFontInfo *curfont = GetFont(iFontHandle);
	if(!curfont)
	{
		return;
	}
	curfont = RE_Font_GetVariant(curfont, &fScale);
	iFontHandle = curfont->GetHandle() | (iFontHandle & ~SET_MASK);

	float fScaleAsian = fScale;
	float fAsianYAdjust = 0.0f;
	if (Language_IsAsian() && fScale > 0.7f)
	{
		fScaleAsian = fScale * 0.75f;
		fAsianYAdjust = ((curfont->GetPointSize() * fScale) - (curfont->GetPointSize() * fScaleAsian)) / 2.0f;
	}

	// Draw a dropshadow if required
	if(iFontHandle & STYLE_DROPSHADOW)
	{
		offset = Round(curfont->GetPointSize() * fScale * 0.075f);

		const vec4_t v4DKGREY2 = {0.15f, 0.15f, 0.15f, rgba?rgba[3]:1.0f};

		gbInShadow = qtrue;
		RE_Font_DrawString(ox + offset, oy + offset, psText, v4DKGREY2, iFontHandle & SET_MASK, iMaxPixelWidth, fScale);
		gbInShadow = qfalse;
	}

	RE_SetColor( rgba );

	// Now we take off the training wheels and become a big font renderer
	// It's all floats from here on out
	fox = ox;
	foy = oy;

	fx = fox;
	foy += curfont->mbRoundCalcs ? Round((curfont->GetHeight() - (curfont->GetDescender() >> 1)) * fScale) : (curfont->GetHeight() - (curfont->GetDescender() >> 1)) * fScale;

	qboolean bNextTextWouldOverflow = qfalse;
	while (*psText && !bNextTextWouldOverflow)
	{
		unsigned int uiLetter = AnyLanguage_ReadCharFromString((char **)&psText);	// 'psText' ptr has been advanced now
		switch( uiLetter )
		{
		case '^':
			if (*psText >= '0' &&
				*psText <= '9')
			{
				colour = ColorIndex(*psText++);
				if (!gbInShadow)
				{
					vec4_t color;
					Com_Memcpy( color, g_color_table[colour], sizeof( color ) );
					color[3] = rgba ? rgba[3] : 1.0f;
					RE_SetColor( color );
				}
			}
			break;
		case 10:						//linefeed
			fx = fox;
			foy += curfont->mbRoundCalcs ? Round(curfont->GetPointSize() * fScale) : curfont->GetPointSize() * fScale;
			if (Language_IsAsian())
			{
				foy += 4.0f;	// this only comes into effect when playing in asian for "A long time ago in a galaxy" etc, all other text is line-broken in feeder functions
			}
			break;
		case 13:						// Return
			break;
		case 32:						// Space
			pLetter = curfont->GetLetter(' ');
			fx += curfont->mbRoundCalcs ? Round(pLetter->horizAdvance * fScale) : pLetter->horizAdvance * fScale;
			bNextTextWouldOverflow = (qboolean)(
				iMaxPixelWidth != -1 &&
				((fx - fox) > (float)iMaxPixelWidth));
			break;

		default:
			pLetter = curfont->GetLetter( uiLetter, &hShader );			// Description of pLetter
			if(!pLetter->width)
			{
				pLetter = curfont->GetLetter('.');
			}

			float fThisScale = uiLetter > 255 ? fScaleAsian : fScale;
			float fAdvancePixels = curfont->mbRoundCalcs ? Round(pLetter->horizAdvance * fThisScale) : pLetter->horizAdvance * fThisScale;
			bNextTextWouldOverflow = (qboolean)(
				iMaxPixelWidth != -1 &&
				(((fx + fAdvancePixels) - fox) > (float)iMaxPixelWidth));
			if (!bNextTextWouldOverflow)
			{
				// this 'mbRoundCalcs' stuff is crap, but the only way to make the font code work. Sigh...
				//
				fy = foy - (curfont->mbRoundCalcs ? Round(pLetter->baseline * fThisScale) : pLetter->baseline * fThisScale);

				RE_StretchPic(curfont->mbRoundCalcs ? fx + Round(pLetter->horizOffset * fThisScale) : fx + pLetter->horizOffset * fThisScale, // float x
								(uiLetter > 255) ? fy - fAsianYAdjust : fy,	// float y
								curfont->mbRoundCalcs ? Round(pLetter->width * fThisScale) : pLetter->width * fThisScale,	// float w
								curfont->mbRoundCalcs ? Round(pLetter->height * fThisScale) : pLetter->height * fThisScale, // float h
								pLetter->s,						// float s1
								pLetter->t,						// float t1
								pLetter->s2,					// float s2
								pLetter->t2,					// float t2
								//lastcolour.c,
								hShader,						// qhandle_t hShader
								1, 1 );

				fx += fAdvancePixels;
			}
			break;
		}
	}
	//let it remember the old color //RE_SetColor(NULL);
}

int RE_RegisterFont_Real(const char *psName)
{
	FontIndexMap_t::iterator it = g_mapFontIndexes.find(psName);
	if (it != g_mapFontIndexes.end() )
	{
		int iFontIndex = (*it).second;
		return iFontIndex;
	}

	// not registered, so...
	//
	{
		CFontInfo *pFont = new CFontInfo(psName);
		if (pFont->GetPointSize() > 0)
		{
			int iFontIndex = g_iCurrentFontIndex - 1;
			g_mapFontIndexes[psName] = iFontIndex;
			return iFontIndex;
		}
		else
		{
			g_mapFontIndexes[psName] = 0;	// missing/invalid
		}
	}

	return 0;
}

int RE_RegisterFont(const char *psName) {
	int oriFontHandle = RE_RegisterFont_Real(psName);
	if (oriFontHandle) {
		CFontInfo *oriFont = GetFont_Actual(oriFontHandle);

		if (oriFont->GetNumVariants() == 0) {
			for (int i = 0; i < MAX_FONT_VARIANTS; i++) {
				const char *variantName = va( "%s_sharp%i", psName, i + 1 );
				const char *fontDatPath = FontDatPath( variantName );
				if ( ri.FS_ReadFile(fontDatPath, NULL) > 0 ) {
					int replacerFontHandle = RE_RegisterFont_Real(variantName);
					if (replacerFontHandle) {
						CFontInfo *replacerFont = GetFont_Actual(replacerFontHandle);
						replacerFont->m_isVariant = qtrue;
						oriFont->AddVariant(replacerFont);
					} else {
						break;
					}
				} else {
					break;
				}
			}
		}
	} else {
		ri.Printf( PRINT_WARNING, "RE_RegisterFont: Couldn't find font %s\n", psName );
	}

	return oriFontHandle;
}

void R_InitFonts(void)
{
	g_iCurrentFontIndex = 1;			// entry 0 is reserved for "missing/invalid"
	g_iNonScaledCharRange = INT_MAX;	// default all chars to have no special scaling (other than user supplied)

	r_fontSharpness = ri.Cvar_Get( "r_fontSharpness", "1", CVAR_ARCHIVE_ND );
}

/*
===============
R_FontList_f
===============
*/

void R_FontList_f( void )
{
	Com_Printf ("------------------------------------\n");

	FontIndexMap_t::iterator it;
	for (it = g_mapFontIndexes.begin(); it != g_mapFontIndexes.end(); ++it)
	{
		CFontInfo *font = GetFont((*it).second);
		if( font )
		{
			Com_Printf("%3i:%s  ps:%hi h:%hi a:%hi d:%hi\n", (*it).second, font->m_sFontName,
				font->mPointSize, font->mHeight, font->mAscender, font->mDescender);
		}
	}
	Com_Printf ("------------------------------------\n");
}

void R_ShutdownFonts(void)
{
	for(int i = 1; i < g_iCurrentFontIndex; i++)	// entry 0 is reserved for "missing/invalid"
	{
		delete g_vFontArray[i];
	}
	g_mapFontIndexes.clear();
	g_vFontArray.clear();
	g_iCurrentFontIndex = 1;	// entry 0 is reserved for "missing/invalid"
}

// this is only really for debugging while tinkering with fonts, but harmless to leave in...
//
void R_ReloadFonts_f(void)
{
	// first, grab all the currently-registered fonts IN THE ORDER THEY WERE REGISTERED...
	//
	std::vector <sstring_t> vstrFonts;

	int iFontToFind = 1;
	for (; iFontToFind < g_iCurrentFontIndex; iFontToFind++)
	{
		FontIndexMap_t::iterator it = g_mapFontIndexes.begin();
		CFontInfo *font = GetFont( iFontToFind );
		if ( font && font->m_isVariant ) continue;
		for (; it != g_mapFontIndexes.end(); ++it)
		{
			if (iFontToFind == (*it).second)
			{
				vstrFonts.push_back( (*it).first );
				break;
			}
		}
		if ( it == g_mapFontIndexes.end() )
		{
			break;	// couldn't find this font
		}
	}
	if ( iFontToFind == g_iCurrentFontIndex ) // found all of them?
	{
		// now restart the font system...
		//
		R_ShutdownFonts();
		R_InitFonts();
		//
		// and re-register our fonts in the same order as before (note that some menu items etc cache the string lengths so really a vid_restart is better, but this is just for my testing)
		//
		for (size_t font = 0; font < vstrFonts.size(); font++)
		{
#ifdef _DEBUG
			int iNewFontHandle = RE_RegisterFont( vstrFonts[font].c_str() );
			Q_assert( (unsigned)iNewFontHandle == font+1 );
#else
			RE_RegisterFont( vstrFonts[font].c_str() );
#endif
		}
		Com_Printf( "Done.\n" );
	}
	else
	{
		Com_Printf( "Problem encountered finding current fonts, ignoring.\n" );	// poo. Oh well, forget it.
	}
}


// end
