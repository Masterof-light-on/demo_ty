#ifndef __USR_MILE_H__
#define __USR_MILE_H__

#include "usr_gps.h"
#include <math.h>

#define PI 3.14159265359
#define EARTH_RADIUS 637.8137


#define  FLASH_SECTOR_SIZE     (64*1024)
typedef struct {
	unsigned int	sector;					/* ÉÈÇøºÅ */ 
	unsigned int	offset;					/* Î»ÖÃÆ«ÒÆ*/ 
} gps_block_t;

#define  FLASH_SECTOR_GPSDATA_SIZE     (100*1024)
#define  SYS_PARAM_GPS_SIZE 		   sizeof(gpsinfo)


unsigned int gps_Calc_Place(char* slat0,char*slon0,char*slat1,char*slon1);
Point gps_MakePoint(char const * const sLon,char const * const slat);
double gps_Distance(Point A,Point B);
void gps_mileage_calc(void);
void gps_mileage_save(void);
void gps_mileage_read(void);

//add
void gps_valid_save(gpsinfo *gpsValid);
void gps_valid_read(void);
void gps_save_validPoint(void);


#endif
