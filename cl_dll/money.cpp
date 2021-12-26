
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "hud.h"
#include "util.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"

DECLARE_MESSAGE( m_Money, Money )
DECLARE_MESSAGE( m_Money, BlinkAcct )

int CHudMoney::Init( )
{
	gHUD.AddHudElem(this);

	HOOK_MESSAGE( Money );
	HOOK_MESSAGE( BlinkAcct );
	m_fFade = 0;
	m_iFlags = 0;
	return 1;
}

int CHudMoney::VidInit()
{
	m_hDollar.SetSpriteByName("dollar");
	m_hMinus.SetSpriteByName("minus");
	m_hPlus.SetSpriteByName("plus");

	return 1;
}

int CHudMoney::Draw(float flTime)
{
	if(( gHUD.m_iHideHUDDisplay & ( HIDEHUD_HEALTH ) ))
		return 1;

	if (!(gHUD.m_iWeaponBits & (1<<(WEAPON_SUIT))))
		return 1;

	int r, g, b, alphaBalance;
	m_fFade -= gHUD.m_flTimeDelta;
	if( m_fFade < 0)
	{
		m_fFade = 0.0f;
		m_iDelta = 0;
	}
	float interpolate = ( 5 - m_fFade ) / 5;

	int iDollarWidth = m_hDollar.rect.right - m_hDollar.rect.left;

	int x = ScreenWidth - iDollarWidth * 7;
	int y = MONEY_YPOS;

	if( m_iBlinkAmt )
	{
		m_fBlinkTime += gHUD.m_flTimeDelta;
		UnpackRGB( r, g, b, m_fBlinkTime > 0.5f? RGB_REDISH : RGB_YELLOWISH );

		if( m_fBlinkTime > 1.0f )
		{
			m_fBlinkTime = 0.0f;
			--m_iBlinkAmt;
		}
	}
	else
	{
		if( m_iDelta != 0 )
		{
			int iDeltaR, iDeltaG, iDeltaB;
			int iDollarHeight = m_hDollar.rect.bottom - m_hDollar.rect.top;
			int iDeltaAlpha = 255 - interpolate * (255);

			UnpackRGB  (iDeltaR, iDeltaG, iDeltaB, m_iDelta < 0 ? RGB_REDISH : RGB_GREENISH);
			ScaleColors(iDeltaR, iDeltaG, iDeltaB, iDeltaAlpha);

			if( m_iDelta > 0 )
			{
				r = interpolate * ((RGB_YELLOWISH & 0xFF0000) >> 16);
				g = (RGB_GREENISH & 0xFF00) >> 8;
				b = (RGB_GREENISH & 0xFF);

				// draw delta
				SPR_Set(m_hPlus.spr, iDeltaR, iDeltaG, iDeltaB );
				SPR_DrawAdditive(0, x, y - iDollarHeight * 1.5, &m_hPlus.rect );
			}
			else if( m_iDelta < 0)
			{
				r = (RGB_REDISH & 0xFF0000) >> 16;
				g = ((RGB_REDISH & 0xFF00) >> 8) + interpolate * (((RGB_YELLOWISH & 0xFF00) >> 8) - ((RGB_REDISH & 0xFF00) >> 8));
				b = (RGB_REDISH & 0xFF) - interpolate * (RGB_REDISH & 0xFF);

				SPR_Set(m_hMinus.spr, iDeltaR, iDeltaG, iDeltaB );
				SPR_DrawAdditive(0, x, y - iDollarHeight * 1.5, &m_hMinus.rect );
			}

			gHUD.DrawHudNumber2( x + iDollarWidth, y - iDollarHeight * 1.5 , false, 5,
									   m_iDelta < 0 ? -m_iDelta : m_iDelta,
									   iDeltaR, iDeltaG, iDeltaB);
			gEngfuncs.pfnFillRGBA(x + iDollarWidth / 4, y - iDollarHeight * 1.5 + gHUD.m_iFontHeight / 4, 2, 2, iDeltaR, iDeltaG, iDeltaB, iDeltaAlpha );
		}
		else UnpackRGB(r, g, b, RGB_YELLOWISH );
	}

	alphaBalance = 255 - interpolate * (255 - MIN_ALPHA);


	ScaleColors( r, g, b, alphaBalance );

	SPR_Set(m_hDollar.spr, r, g, b);
	SPR_DrawAdditive(0, x, y, &m_hDollar.rect);

	gHUD.DrawHudNumber2( x + iDollarWidth, y, false, 5, m_iMoneyCount, r, g, b );
	gEngfuncs.pfnFillRGBA(x + iDollarWidth / 4, y + gHUD.m_iFontHeight / 4, 2, 2, r, g, b, alphaBalance );
	return 1;
}

int CHudMoney::MsgFunc_Money(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );
	int iOldCount = m_iMoneyCount;
	m_iMoneyCount = READ_LONG();
	m_iDelta = m_iMoneyCount - iOldCount;
	m_fFade = 5.0f;
	m_iFlags |= HUD_ACTIVE;
	return 1;
}

int CHudMoney::MsgFunc_BlinkAcct(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	m_iBlinkAmt = READ_BYTE();
	m_fBlinkTime = 0;
	return 1;
}
