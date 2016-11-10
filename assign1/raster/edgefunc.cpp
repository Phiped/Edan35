// -------------------------------------------------------------------------
// File:    cEdgeFunc
// Desc:    an edge function class, used for traversal and interpolation
//			edgefunction(x,y) = mA*x + mB*y + mC
//
// Author:  Tomas Akenine-Möller
// History: February, 2005 (started)
// -------------------------------------------------------------------------

#include "edgefunc.h"

#include "mmgr/mmgr.h"

cEdgeFunc::cEdgeFunc()
{
}

void cEdgeFunc::setup(int signed_tri_area,int x0,int y0,int x1,int y1)
{
	mA=y0-y1;
	mB=x1-x0;
	if(signed_tri_area<0)	// need to flip the sign of edge functions?
	{
		mA=-mA;
		mB=-mB;
	}
	mC= -mA*x0 - mB*y0;
}

bool cEdgeFunc::inside(const int x, const int y, int &signed_dist) const
{
	signed_dist = mA*x + mB*y + mC;
	if(signed_dist>0) return true;
	if (signed_dist < 0) return false;
	if (mA > 0) return true;
	if (mA < 0) return false;
	return mB > 0;
	return false;
}