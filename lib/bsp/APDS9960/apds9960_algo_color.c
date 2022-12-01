#include "config.h"
#if USE_APDS9960

/*
	Ce fichier présente un algorithme visant à rechercher la couleur observée la plus proche parmi des couleurs de références (calibrées à la main dans les define ci-dessous).
	Pour plus d'explication, contactez samuel.poiraud@eseo.fr après avoir consulté attentivement ce fichier.
*/
#include "macro_types.h"
#include "apds9960.h"

uint16_t APDS9960_searchMin(int32_t values[], uint16_t nbValue){
	uint8_t i, indiceMin = 0;
	for(i=0; i<nbValue; i++){
		if(values[i] < values[indiceMin]){
			indiceMin = i;
		}
	}
	return indiceMin;
}

#define COLOR_SENSOR_AVERAGE_DIFFRENCIAL_MULTIPLIER			1
#define COLOR_SENSOR_DIFFRENCIAL_DIFFRENCIAL_MULTIPLIER		0

// Red
#define COLOR_SENSOR_RED__RED			75
#define COLOR_SENSOR_RED__GREEN			14
#define COLOR_SENSOR_RED__BLUE			25
#define COLOR_SENSOR_RED__RED_GREEN		(absolute(COLOR_SENSOR_RED__RED - COLOR_SENSOR_RED__GREEN))
#define COLOR_SENSOR_RED__RED_BLUE		(absolute(COLOR_SENSOR_RED__RED - COLOR_SENSOR_RED__BLUE))
#define COLOR_SENSOR_RED__GREEN_BLUE	(absolute(COLOR_SENSOR_RED__GREEN - COLOR_SENSOR_RED__BLUE))

// Green
#define COLOR_SENSOR_GREEN__RED				13
#define COLOR_SENSOR_GREEN__GREEN			51
#define COLOR_SENSOR_GREEN__BLUE			43
#define COLOR_SENSOR_GREEN__RED_GREEN		(absolute(COLOR_SENSOR_GREEN__RED - COLOR_SENSOR_GREEN__GREEN))
#define COLOR_SENSOR_GREEN__RED_BLUE		(absolute(COLOR_SENSOR_GREEN__RED - COLOR_SENSOR_GREEN__BLUE))
#define COLOR_SENSOR_GREEN__GREEN_BLUE		(absolute(COLOR_SENSOR_GREEN__GREEN - COLOR_SENSOR_GREEN__BLUE))

// Blue
#define COLOR_SENSOR_BLUE__RED				13
#define COLOR_SENSOR_BLUE__GREEN			38
#define COLOR_SENSOR_BLUE__BLUE				55
#define COLOR_SENSOR_BLUE__RED_GREEN		(absolute(COLOR_SENSOR_BLUE__RED - COLOR_SENSOR_BLUE__GREEN))
#define COLOR_SENSOR_BLUE__RED_BLUE			(absolute(COLOR_SENSOR_BLUE__RED - COLOR_SENSOR_BLUE__BLUE))
#define COLOR_SENSOR_BLUE__GREEN_BLUE		(absolute(COLOR_SENSOR_BLUE__GREEN - COLOR_SENSOR_BLUE__BLUE))

// White
#define COLOR_SENSOR_WHITE__RED				45 //43
#define COLOR_SENSOR_WHITE__GREEN			32 //60
#define COLOR_SENSOR_WHITE__BLUE			38 //55
#define COLOR_SENSOR_WHITE__AMBIANT			255
#define COLOR_SENSOR_WHITE__RED_GREEN		(absolute(COLOR_SENSOR_WHITE__RED - COLOR_SENSOR_WHITE__GREEN))
#define COLOR_SENSOR_WHITE__RED_BLUE		(absolute(COLOR_SENSOR_WHITE__RED - COLOR_SENSOR_WHITE__BLUE))
#define COLOR_SENSOR_WHITE__GREEN_BLUE		(absolute(COLOR_SENSOR_WHITE__GREEN - COLOR_SENSOR_WHITE__BLUE))

// Black
#define COLOR_SENSOR_BLACK__RED				76 //26
#define COLOR_SENSOR_BLACK__GREEN			21 //37
#define COLOR_SENSOR_BLACK__BLUE			32 //35
#define COLOR_SENSOR_BLACK__AMBIANT			150
#define COLOR_SENSOR_BLACK__RED_GREEN		(absolute(COLOR_SENSOR_BLACK__RED - COLOR_SENSOR_BLACK__GREEN))
#define COLOR_SENSOR_BLACK__RED_BLUE		(absolute(COLOR_SENSOR_BLACK__RED - COLOR_SENSOR_BLACK__BLUE))
#define COLOR_SENSOR_BLACK__GREEN_BLUE		(absolute(COLOR_SENSOR_BLACK__GREEN - COLOR_SENSOR_BLACK__BLUE))

	typedef enum{
		COLOR_SENSOR_NONE,
		COLOR_SENSOR_RED,
		COLOR_SENSOR_GREEN,
		COLOR_SENSOR_BLUE,
		COLOR_SENSOR_WHITE,
		COLOR_SENSOR_BLACK,
		COLOR_SENSOR_NB
	}colorSensor_e;
	
	
	typedef struct{
	colorSensor_e color;
	Uint8 red;
	Uint8 green;
	Uint8 blue;
	Uint8 ambiant;
	Uint8 proximity;
	}colorSensor_s;


/**
 * Fonction de reconnaissance de couleurs à partir des composantes R, G et B fournies par le capteur couleur.
 * @param sensor_datas les données du capteur (composantes RGB)
 * @param search les couleurs à rechercher (i.e., les couleurs possibles) sous la forme d'un masque binaire
 */
static colorSensor_e SENSOR_analyse_color(void)
{
	uint16_t ambiant, red, blue, green;
	uint16_t redGreen, redBlue, greenBlue;
	uint32_t redFound, greenFound, blueFound,whiteFound,blackFound;
	colorSensor_e color;
	color = COLOR_SENSOR_NONE;


	APDS9960_readAmbientLight(&ambiant);
	APDS9960_readRedLight(&red);
	APDS9960_readBlueLight(&blue);
	APDS9960_readGreenLight(&green);


	if(ambiant)
	{
		//Composantes RGB, ramenés en pourcentage de la mesure de lumière ambiante
		blue = (uint16_t)(((uint32_t)blue*100)/ambiant);
		red = (uint16_t)(((uint32_t)red*100)/ambiant);
		green = (uint16_t)(((uint32_t)green*100)/ambiant);
	}

	if(ambiant)
	{
//		debug_printf("%d, %d, %d, %d\n", red, green, blue, sensors[i].ambiant);
		redGreen = absolute(red - green);
		redBlue = absolute(red - red);
		greenBlue = absolute(green - blue);

		Sint32 colors_value[COLOR_SENSOR_NB] = {0};
		colorSensor_e colors_id[COLOR_SENSOR_NB] = {0};
		Uint8 nb = 0;

	
		redFound = (absolute(red - COLOR_SENSOR_RED__RED) + absolute(green - COLOR_SENSOR_RED__GREEN) + absolute(blue - COLOR_SENSOR_RED__BLUE)) * COLOR_SENSOR_AVERAGE_DIFFRENCIAL_MULTIPLIER
					+ (absolute(redGreen - COLOR_SENSOR_RED__RED_GREEN) + absolute(redBlue - COLOR_SENSOR_RED__RED_BLUE) + absolute(greenBlue - COLOR_SENSOR_RED__GREEN_BLUE)) * COLOR_SENSOR_DIFFRENCIAL_DIFFRENCIAL_MULTIPLIER;
		colors_value[nb] = redFound;
		colors_id[nb] = COLOR_SENSOR_RED;
		nb++;
	

		greenFound = (absolute(red - COLOR_SENSOR_GREEN__RED) + absolute(green - COLOR_SENSOR_GREEN__GREEN) + absolute(blue - COLOR_SENSOR_GREEN__BLUE)) * COLOR_SENSOR_AVERAGE_DIFFRENCIAL_MULTIPLIER
					+ (absolute(redGreen - COLOR_SENSOR_GREEN__RED_GREEN) + absolute(redBlue - COLOR_SENSOR_GREEN__RED_BLUE) + absolute(greenBlue - COLOR_SENSOR_GREEN__GREEN_BLUE)) * COLOR_SENSOR_DIFFRENCIAL_DIFFRENCIAL_MULTIPLIER;
		colors_value[nb] = greenFound;
		colors_id[nb] = COLOR_SENSOR_GREEN;
		nb++;
	

		blueFound = (absolute(red - COLOR_SENSOR_BLUE__RED) + absolute(green - COLOR_SENSOR_BLUE__GREEN) + absolute(blue - COLOR_SENSOR_BLUE__BLUE)) * COLOR_SENSOR_AVERAGE_DIFFRENCIAL_MULTIPLIER
					+ (absolute(redGreen - COLOR_SENSOR_BLUE__RED_GREEN) + absolute(redBlue - COLOR_SENSOR_BLUE__RED_BLUE) + absolute(greenBlue - COLOR_SENSOR_BLUE__GREEN_BLUE)) * COLOR_SENSOR_DIFFRENCIAL_DIFFRENCIAL_MULTIPLIER;
		colors_value[nb] = blueFound;
		colors_id[nb] = COLOR_SENSOR_BLUE;
		nb++;


		whiteFound = (absolute(ambiant - COLOR_SENSOR_WHITE__AMBIANT) + absolute(red - COLOR_SENSOR_WHITE__RED) + absolute(green - COLOR_SENSOR_WHITE__GREEN) + absolute(blue - COLOR_SENSOR_WHITE__BLUE)) * COLOR_SENSOR_AVERAGE_DIFFRENCIAL_MULTIPLIER
					+ (absolute(redGreen - COLOR_SENSOR_WHITE__RED_GREEN) + absolute(redBlue - COLOR_SENSOR_WHITE__RED_BLUE) + absolute(greenBlue - COLOR_SENSOR_WHITE__GREEN_BLUE)) * COLOR_SENSOR_DIFFRENCIAL_DIFFRENCIAL_MULTIPLIER;
		colors_value[nb] = whiteFound;
		colors_id[nb] = COLOR_SENSOR_WHITE;
		nb++;
	
	
		blackFound = (absolute(ambiant - COLOR_SENSOR_BLACK__AMBIANT) + absolute(red - COLOR_SENSOR_BLACK__RED) + absolute(green - COLOR_SENSOR_BLACK__GREEN) + absolute(blue - COLOR_SENSOR_BLACK__BLUE)) * COLOR_SENSOR_AVERAGE_DIFFRENCIAL_MULTIPLIER
					+ (absolute(redGreen - COLOR_SENSOR_BLACK__RED_GREEN) + absolute(redBlue - COLOR_SENSOR_BLACK__RED_BLUE) + absolute(greenBlue - COLOR_SENSOR_BLACK__GREEN_BLUE)) * COLOR_SENSOR_DIFFRENCIAL_DIFFRENCIAL_MULTIPLIER;
		colors_value[nb] = blackFound;
		colors_id[nb] = COLOR_SENSOR_BLACK;
		nb++;
	

		Uint8 indiceMin = APDS9960_searchMin(colors_value, nb);
		color = colors_id[indiceMin];
	}

	return color;
}

#endif
