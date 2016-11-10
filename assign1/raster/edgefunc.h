// -------------------------------------------------------------------------
// File:    cEdgeFunc
// Desc:    an edge function class, used for traversal and interpolation
//			edgefunction(x,y) = mA*x + mB*y + mC
//
// Author:  Tomas Akenine-Möller
// History: February, 2005 (started)
// -------------------------------------------------------------------------

#ifndef C_EDGE_FUNC_H
#define C_EDGE_FUNC_H

class cEdgeFunc
{
public:
					cEdgeFunc				(void);
	void			setup					(int signed_tri_area,int x0,int y0,int x1,int y1);
	bool			inside					(const int x, const int y, int &signed_dist) const;
protected:
	int				mA;
	int				mB;
	int				mC;
};

#endif