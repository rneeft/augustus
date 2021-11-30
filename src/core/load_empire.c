#include "assets/assets.h"
#include "assets/xml.h"

#include "core/calc.h"
#include "core/dir.h"
#include "core/file.h"
#include "core/image_group.h"
#include "core/log.h"
#include "core/png_read.h"
#include "core/string.h"

#include "empire/object.h"
#include "empire/type.h"

#include "scenario/data.h"

#include "expat.h"

#include <string.h>

#define XML_HASH_SEED 0x12345678
#define XML_BUFFER_SIZE 1024
#define XML_MAX_DEPTH 4
#define XML_MAX_ELEMENTS_PER_DEPTH 2
#define XML_MAX_ATTRIBUTES 12
#define XML_TAG_MAX_LENGTH 12

static struct {
    struct {
        char file_name[FILE_NAME_MAX];
        size_t file_name_position;
        int depth;
        int error;
        int finished;
        full_empire_object *current_city;
    } xml;
    int total_objects;
    int total_trade_routes;
    int imports_for_city;
    int exports_for_city;
    int num_routes_for_city;
    int resource_id;
    int resource_flag;
} data;

static const char XML_FILE_ELEMENTS[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH][XML_TAG_MAX_LENGTH] = { { "empire" }, { "city", "invasionPath"}, { "trade" }, { "export", "import" } };
static const char XML_FILE_ATTRIBUTES[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH][XML_MAX_ATTRIBUTES][XML_TAG_MAX_LENGTH] = {
    { { "id" } }, // empire
    { 
        { "type", "name", "x", "y" }, //city
        { "x", "y", "yearsBefore" }, //invasion warning
    }, 
    { { "open", "cost", "isWater" } }, //trade
    { 
        { "resourceId", "amount" }, // export
        { "resourceId", "amount" } // import
    } 
};

static void xml_start_empire_element(const char** attributes);
static void xml_start_city_element(const char** attributes);
//static void xml_start_enemy_path_element(const char** attributes);
//static void xml_start_roman_path_element(const char** attributes);
static void xml_start_invasion_element(const char** attributes);
static void xml_start_trade_element(const char** attributes);
static void xml_start_export_element(const char** attributes);
static void xml_start_import_element(const char** attributes);
static void xml_end_empire_element(void);
static void xml_end_city_element(void);
//static void xml_end_enemy_path_element(void);
//static void xml_end_roman_path_element(void);
static void xml_end_invasion_element(void);
static void xml_end_trade_element(void);
static void xml_end_export_element(void);
static void xml_end_import_element(void);

static void (*xml_start_element_callback[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH])(const char** attributes) = {
    { xml_start_empire_element }, { xml_start_city_element, xml_start_invasion_element }, { xml_start_trade_element },  { xml_start_export_element, xml_start_import_element }
};

static void (*xml_end_element_callback[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH])(void) = {
    { xml_end_empire_element }, { xml_end_city_element, xml_end_invasion_element }, { xml_end_trade_element }, { xml_end_import_element, xml_end_export_element }
};

static int count_xml_attributes(const char** attributes)
{
    int total = 0;
    while (attributes[total]) {
        ++total;
    }
    return total;
}


static void xml_start_empire_element(const char** attributes)
{
    data.total_objects = 0;
    data.total_trade_routes = 1;
}


static void xml_start_invasion_element(const char** attributes)
{
    data.xml.current_city = &objects[data.total_objects];
    full_empire_object* invasion_warning = data.xml.current_city;

    int total_attributes = count_xml_attributes(attributes);
    if (total_attributes < 6 || total_attributes > 6 || total_attributes % 2) {
        data.xml.error = 1;
        return;
    }

    invasion_warning->obj.id = data.total_objects;
    invasion_warning->in_use = 1;
    invasion_warning->obj.image_id = image_group(GROUP_EMPIRE_BATTLE_ICON);
    invasion_warning->obj.width = 50;
    invasion_warning->obj.height = 50;
    invasion_warning->obj.type = EMPIRE_OBJECT_BATTLE_ICON;
    invasion_warning->obj.invasion_path_id = 1;

    for (int i = 0; i < total_attributes; i += 2) {
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][1][0]) == 0) {
            invasion_warning->obj.x = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][1][1]) == 0) {
            invasion_warning->obj.y = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][1][2]) == 0) {
            invasion_warning->obj.invasion_years = string_to_int(string_from_ascii(attributes[i + 1]));
        }
    }
}
//
//static void xml_start_enemy_path_element(const char** attributes)
//{
//    data.xml.current_city = &objects[data.total_objects];
//    full_empire_object* enemy_path = data.xml.current_city;
//
//    int total_attributes = count_xml_attributes(attributes);
//    if (total_attributes < 6 || total_attributes > 6 || total_attributes % 2) {
//        data.xml.error = 1;
//        return;
//    }
//
//    enemy_path->obj.id = data.total_objects;
//    enemy_path->in_use = 1;
//    enemy_path->obj.image_id = 8055;
//    enemy_path->obj.width = 50;
//    enemy_path->obj.height = 50;
//    enemy_path->obj.type = EMPIRE_OBJECT_ENEMY_ARMY;
//
//    for (int i = 0; i < total_attributes; i += 2) {
//        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][2][0]) == 0) {
//            enemy_path->obj.x = string_to_int(string_from_ascii(attributes[i + 1]));
//        }
//        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][2][1]) == 0) {
//            enemy_path->obj.y = string_to_int(string_from_ascii(attributes[i + 1]));
//        }
//        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][2][2]) == 0) {
//            enemy_path->obj.distant_battle_travel_months = string_to_int(string_from_ascii(attributes[i + 1]));
//        }
//    }
//}
//
//static void xml_start_roman_path_element(const char** attributes)
//{
//    data.xml.current_city = &objects[data.total_objects];
//    full_empire_object* roman_path = data.xml.current_city;
//
//    int total_attributes = count_xml_attributes(attributes);
//    if (total_attributes < 6 || total_attributes > 6 || total_attributes % 2) {
//        data.xml.error = 1;
//        return;
//    }
//
//    roman_path->obj.id = data.total_objects;
//    roman_path->in_use = 1;
//    roman_path->obj.image_id = image_group(GROUP_EMPIRE_ROMAN_ARMY);
//    roman_path->obj.width = 50;
//    roman_path->obj.height = 50;
//    roman_path->obj.type = EMPIRE_OBJECT_ROMAN_ARMY;
//
//    for (int i = 0; i < total_attributes; i += 2) {
//        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][3][0]) == 0) {
//            roman_path->obj.x = string_to_int(string_from_ascii(attributes[i + 1]));
//        }
//        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][3][1]) == 0) {
//            roman_path->obj.y = string_to_int(string_from_ascii(attributes[i + 1]));
//        }
//        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][3][2]) == 0) {
//            roman_path->obj.distant_battle_travel_months = string_to_int(string_from_ascii(attributes[i + 1]));
//        }
//    }
//}

static void xml_start_city_element(const char** attributes)
{
    data.xml.current_city = &objects[data.total_objects];
    full_empire_object* city = data.xml.current_city;

    int total_attributes = count_xml_attributes(attributes);
    if (total_attributes < 8 || total_attributes > 8 || total_attributes % 2) {
        data.xml.error = 1;
        return;
    }

    city->obj.id = data.total_objects;
    city->in_use = 1;
    city->obj.image_id = GROUP_EMPIRE_CITY;
    city->obj.width = 50;
    city->obj.height = 50;
    city->obj.type = 1;

    for (int i = 0; i < total_attributes; i += 2) {
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][0][0]) == 0) {
            city->city_type = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][0][1]) == 0) {
            strcpy((char *)city->city_display_name, attributes[i + 1]);
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][0][2]) == 0) {
            city->obj.x = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[1][0][3]) == 0) {
            city->obj.y = string_to_int(string_from_ascii(attributes[i + 1]));
        }

    }
}

static void xml_start_trade_element(const char** attributes)
{
    data.xml.current_city = &objects[data.total_objects];
    full_empire_object* city = data.xml.current_city;

    int total_attributes = count_xml_attributes(attributes);

    for (int i = 0; i < total_attributes; i += 2) {
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][0]) == 0) {
            city->trade_route_open = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][1]) == 0) {
            city->trade_route_cost = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[2][0][2]) == 0) {            
            int is_sea_route = string_to_int(string_from_ascii(attributes[i + 1])); 
            full_empire_object* trade_route = &objects[data.total_objects + 1];
            trade_route->in_use = 1;
            trade_route->obj.id = data.total_objects + 1;

            city->obj.trade_route_id = data.total_trade_routes;
            trade_route->obj.trade_route_id = data.total_trade_routes;

            trade_route->obj.type = EMPIRE_OBJECT_LAND_TRADE_ROUTE + is_sea_route;
            
            //set x and y to city location for drawing this route later
            trade_route->obj.x = city->obj.x;
            trade_route->obj.y = city->obj.y;

            data.total_trade_routes++;
            data.num_routes_for_city++;

            data.xml.current_city->trade15 = 0;
            data.xml.current_city->trade25 = 0;
            data.xml.current_city->trade40 = 0;
        }
    }
}

static void xml_start_import_element(const char** attributes)
{
    data.xml.current_city = &objects[data.total_objects];

    int total_attributes = count_xml_attributes(attributes);
    if (total_attributes < 4 || total_attributes > 4 || total_attributes % 2) {
        data.xml.error = 1;
        return;
    }


    int amount = 0;

    // todo: refactor so that attribute order does not matter
    for (int i = 0; i < total_attributes; i += 2) {

        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[3][0][0]) == 0) {
            data.resource_id = string_to_int(string_from_ascii(attributes[i + 1]));
            data.resource_flag = 1 << data.resource_id;
            data.xml.current_city->city_buys_resource[data.imports_for_city] = data.resource_id;
            data.imports_for_city++;
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[3][0][1]) == 0) {
            amount = string_to_int(string_from_ascii(attributes[i + 1]));
            switch (amount) {
                case 15:
                    data.xml.current_city->trade15 += data.resource_flag;
                    break;
                case 25:
                    data.xml.current_city->trade25 += data.resource_flag;
                    break;
                case 40:
                    data.xml.current_city->trade40 += data.resource_flag;
                    break;
            }
        }
    }
}

static void xml_start_export_element(const char** attributes)
{
    data.xml.current_city = &objects[data.total_objects];

    int total_attributes = count_xml_attributes(attributes);
    if (total_attributes < 4 || total_attributes > 4 || total_attributes % 2) {
        data.xml.error = 1;
        return;
    }

    int amount = 0;

     for (int i = 0; i < total_attributes; i += 2) {

        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[3][0][0]) == 0) {
            data.resource_id = string_to_int(string_from_ascii(attributes[i + 1]));
            data.resource_flag = 1 << data.resource_id;
            data.xml.current_city->city_sells_resource[data.exports_for_city] = data.resource_id;
            data.exports_for_city++;
        }
        if (strcmp(attributes[i], XML_FILE_ATTRIBUTES[3][0][1]) == 0) {
            amount = string_to_int(string_from_ascii(attributes[i + 1]));
            switch (amount) {
            case 15:
                data.xml.current_city->trade15 += data.resource_flag;
                break;
            case 25:
                data.xml.current_city->trade25 += data.resource_flag;
                break;
            case 40:
                data.xml.current_city->trade40 += data.resource_flag;
                break;
            }
        }
    }
}

static void xml_end_empire_element(void)
{
    data.xml.finished = 1;
}

static void xml_end_invasion_element(void)
{
    data.total_objects++;
}

//static void xml_end_enemy_path_element(void)
//{
//    data.total_objects++;
//}
//
//static void xml_end_roman_path_element(void)
//{
//    data.total_objects++;
//}

static void xml_end_city_element(void)
{
    data.total_objects = data.total_objects + data.num_routes_for_city + 1;
    data.num_routes_for_city = 0;
}

static void xml_end_trade_element(void)
{
    data.imports_for_city = 0;
    data.exports_for_city = 0;
}

static void xml_end_import_element(void)
{
    data.resource_flag = 0;
    data.resource_id = 0;
}

static void xml_end_export_element(void)
{
    data.resource_flag = 0;
    data.resource_id = 0;
}

static int get_element_index(const char* name)
{
    for (int i = 0; i < XML_MAX_ELEMENTS_PER_DEPTH; ++i) {
        if (XML_FILE_ELEMENTS[data.xml.depth][i][0] == 0) {
            continue;
        }
        if (strcmp(XML_FILE_ELEMENTS[data.xml.depth][i], name) == 0) {
            return i;
        }
    }
    return -1;
}

static void XMLCALL xml_start_element(void* unused, const char* name, const char** attributes)
{
    if (data.xml.error) {
        return;
    }
    if (data.xml.finished || data.xml.depth == XML_MAX_DEPTH) {
        data.xml.error = 1;
        log_error("Invalid XML parameter", name, 0);
        return;
    }
    int index = get_element_index(name);
    if (index == -1) {
        data.xml.error = 1;
        log_error("Invalid XML parameter", name, 0);
        return;
    }
    (*xml_start_element_callback[data.xml.depth][index])(attributes);
    data.xml.depth++;
}

static void XMLCALL xml_end_element(void* unused, const char* name)
{
    if (data.xml.error) {
        return;
    }
    data.xml.depth--;
    int index = get_element_index(name);
    if (index == -1) {
        data.xml.error = 1;
        log_error("Invalid XML parameter", name, 0);
        return;
    }
    (*xml_end_element_callback[data.xml.depth][index])();
}


static void clear_xml_info(void)
{
    data.xml.error = 0;
    data.xml.depth = 0;
    data.xml.finished = 0;
}


static void process_file(const char* xml_file_name)
{
    log_info("Loading xml file", xml_file_name, 0);

    FILE* xml_file = file_open(xml_file_name, "r");

    if (!xml_file) {
        log_error("Error opening xml file", xml_file_name, 0);
        return;
    }

    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetElementHandler(parser, xml_start_element, xml_end_element);

    char buffer[XML_BUFFER_SIZE];
    int done = 0;

    do {
        size_t bytes_read = fread(buffer, 1, XML_BUFFER_SIZE, xml_file);
        done = bytes_read < sizeof(buffer);
        if (XML_Parse(parser, buffer, (int)bytes_read, done) == XML_STATUS_ERROR || data.xml.error) {
            log_error("Error parsing file", xml_file_name, 0);
            break;
        }
    } while (!done);

    if (data.xml.error || !data.xml.finished) {
        log_info("empire load failed", "", 0);
    }

    clear_xml_info();

    XML_ParserFree(parser);
    file_close(xml_file);
}


void empire_load_from_file(const char* file_path)
{
    clear_empire_objects();
    process_file(file_path);
        
}

