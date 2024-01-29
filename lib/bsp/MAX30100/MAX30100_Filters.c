/*
 * MAX30100_Filters.c
 *
 *  Created on: Jun 13, 2022
 *      Author: durieuro
 */


float v[2] = {0.0};

float Filters_step(float x) //class II
{
	v[0] = v[1];
	v[1] = (2.452372752527856026e-1 * x)
		 + (0.50952544949442879485 * v[0]);
	return
		 (v[0] + v[1]);
}



