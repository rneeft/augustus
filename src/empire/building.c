#include "building.h"

#include "assets/assets.h"
#include "core/image.h"
#include "core/image_group.h"
#include "empire/building_type.h"
#include "translation/translation.h"


int empire_building_image(empire_building_type type)
{
	switch (type) {
	case EMPIRE_BUILDING_WHARF:
		return 7744;
	case EMPIRE_BUILDING_WALLS:
		return 7745;
	case EMPIRE_BUILDING_HIGHWAY:
		return 7747;
	case EMPIRE_BUILDING_WOODCUTTER:
		return 7748;
	default:
		return 7737;
		break;
	}
}

translation_key empire_building_name(empire_building_type type)
{
	switch (type) {
	case EMPIRE_BUILDING_WHARF:
		return TR_EMPIRE_BUILDING_WHARF_NAME;
	case EMPIRE_BUILDING_WOODCUTTER:
		return TR_EMPIRE_BUILDING_WOODCUTTER_NAME;
	case EMPIRE_BUILDING_HIGHWAY:
		return TR_EMPIRE_BUILDING_HIGHWAY_NAME;
	case EMPIRE_BUILDING_WALLS:
		return TR_EMPIRE_BUILDING_WALLS_NAME;
	default:
		return TR_EMPIRE_BUILDING_NONE_NAME;
		break;
	}
}

translation_key empire_building_description(empire_building_type type)
{
	switch (type) {
	case EMPIRE_BUILDING_WHARF:
		return TR_EMPIRE_BUILDING_WHARF_DESC;
	case EMPIRE_BUILDING_WOODCUTTER:
		return TR_EMPIRE_BUILDING_WOODCUTTER_DESC;
	case EMPIRE_BUILDING_HIGHWAY:
		return TR_EMPIRE_BUILDING_HIGHWAY_DESC;
	case EMPIRE_BUILDING_WALLS:
		return TR_EMPIRE_BUILDING_WALLS_DESC;
	default:
		return TR_EMPIRE_BUILDING_NONE_DESC;
		break;
	}
}