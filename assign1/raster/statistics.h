// -------------------------------------------------------------------------
// File:    statistics (static class)
// Desc:    class for gathering statistics
//
// Author:  Tomas Akenine-Möller
// History: November, 2004 (started)
// -------------------------------------------------------------------------

#ifndef STATISTICS_H
#define STATISTICS_H

#include "misc.h"

typedef enum 
{ 
	STATS_VISITED_PIXELS=0,

	STATS_DEPTH_BUFFER_READ_BW=1,			// all BandWidth numbers are in bits, not bytes!
	STATS_DEPTH_BUFFER_NUM_READS=2,

	STATS_DEPTH_BUFFER_WRITE_BW=3,
	STATS_DEPTH_BUFFER_NUM_WRITES=4,

	STATS_COLOR_BUFFER_READ_BW=5,
	STATS_COLOR_BUFFER_NUM_READS=6,

	STATS_COLOR_BUFFER_WRITE_BW=7,
	STATS_COLOR_BUFFER_NUM_WRITES=8,
	
	STATS_TEXTURE_READ_BW=9,
	STATS_TEXTURE_NUM_READS=10,

	STATS_COLOR_CLEAR_BW=11,	
	STATS_DEPTH_CLEAR_BW=12,	

	STATS_PROCESSED_PIXELS=13,
	STATS_SHADED_PIXELS=14,

	STATS_SIZE=15
} tStatsType;

static bool cStatsOn=true;
//static bool cStatsOn=false;

class statistics
{
public:
	static void						clear(void);
	static void						add(tStatsType type,int increment=1);
	static int64					get(tStatsType type);
	static void						print(void);
protected:
	static int64					mStats[STATS_SIZE];
};

inline void statistics::add(tStatsType type,int increment)
{
	if(!cStatsOn) return;
	ASSERT(type<STATS_SIZE);
	mStats[type]+=increment;
}

#endif

