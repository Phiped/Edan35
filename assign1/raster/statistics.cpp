// -------------------------------------------------------------------------
// File:    statistics (static class)
// Desc:    class for gathering statistics
//
// Author:  Tomas Akenine-Möller
// History: November, 2004 (started)
// -------------------------------------------------------------------------

#include <stdio.h>

#include "statistics.h"
#include "misc.h"

#include "depthunit.h"
#include "colorunit.h"

#include "mmgr/mmgr.h"

int64 statistics::mStats[STATS_SIZE];


void statistics::clear(void)
{
	printf("Clearing statistics...\n");
	for(int q=0;q<STATS_SIZE;q++) mStats[q]=0;
}

int64 statistics::get(tStatsType type)
{
	ASSERT(type<STATS_SIZE);
	return mStats[type];
}

void statistics::print(void)
{
	if(!cStatsOn) return;
	printf("---===### STATISTICS ###===---\n");
	printf(" %8d visited pixels\n",mStats[STATS_VISITED_PIXELS]);
	printf(" %8d processed pixels\n",mStats[STATS_PROCESSED_PIXELS]);
	printf(" %8d shaded pixels\n",mStats[STATS_SHADED_PIXELS]);
	int64 tmp;
	
	// print depth buffer BW stats
	tmp=(statistics::get(STATS_DEPTH_BUFFER_READ_BW)+statistics::get(STATS_DEPTH_BUFFER_WRITE_BW))>>3L;

	printf("Depth buffer BW: %I64d [R] + %I64d [W] = %I64d bytes = %5.3f Mbytes\n",statistics::get(STATS_DEPTH_BUFFER_READ_BW)>>3L,
			statistics::get(STATS_DEPTH_BUFFER_WRITE_BW)>>3L,tmp,float(double(tmp)/(1024.0*1024.0)));

	printf("Depth clear BW: %I64d bytes = %5.3f Mbytes\n",statistics::get(STATS_DEPTH_CLEAR_BW)>>3L,
		float(double(statistics::get(STATS_DEPTH_CLEAR_BW)>>3L)/(1024.0*1024.0)));

	tmp=(statistics::get(STATS_COLOR_BUFFER_READ_BW)+statistics::get(STATS_COLOR_BUFFER_WRITE_BW))>>3L;

	printf("Color buffer BW: %I64d [R] + %I64d [W] = %I64d bytes = %5.3f Mbytes\n",statistics::get(STATS_COLOR_BUFFER_READ_BW)>>3L,
			statistics::get(STATS_COLOR_BUFFER_WRITE_BW)>>3L,tmp,float(double(tmp)/(1024.0*1024.0)));

	printf("Color clear BW: %I64d bytes = %5.3f Mbytes\n",statistics::get(STATS_COLOR_CLEAR_BW)>>3L,
		float(double(statistics::get(STATS_COLOR_CLEAR_BW)>>3L)/(1024.0*1024.0)));

	printf("Texture reads: %I64d = %5.3f Mbytes\n",statistics::get(STATS_TEXTURE_READ_BW)>>3L,
			float(double(statistics::get(STATS_TEXTURE_READ_BW)>>3L)/(1024.0*1024.0)));

	int64 tot_r=(statistics::get(STATS_DEPTH_BUFFER_READ_BW) + statistics::get(STATS_COLOR_BUFFER_READ_BW) + statistics::get(STATS_TEXTURE_READ_BW)  )>>3L;
	int64 tot_w=(statistics::get(STATS_DEPTH_BUFFER_WRITE_BW) + statistics::get(STATS_COLOR_BUFFER_WRITE_BW) )>>3L;
	int64 tot_c=(statistics::get(STATS_DEPTH_CLEAR_BW) + statistics::get(STATS_COLOR_CLEAR_BW) )>>3L;

	printf("Total bandwidth (BW): %5.3f [R] + %5.3f [W] + %5.3f [C] = %5.3f Mbytes\n",
		float(double(tot_r)/(1024.0*1024.0)),float(double(tot_w)/(1024.0*1024.0)),float(double(tot_c)/(1024.0*1024.0)),
		float(double(tot_r+tot_w+tot_c)/(1024.0*1024.0)));

	printf("==============================\n");
}
