#ifndef EMPIRE_BUILDING_H
#define EMPIRE_BUILDING_H

#include "empire/building_type.h"
#include "translation/translation.h"

typedef struct {
	int id;
	int type;
} empire_building;

int empire_building_image(empire_building_type type);
translation_key empire_building_name(empire_building_type type);
translation_key empire_building_description(empire_building_type type);

#endif // EMPIRE_BUILD_SPOT_H
