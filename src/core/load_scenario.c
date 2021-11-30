#include "assets/assets.h"
#include "assets/xml.h"

#include "core/calc.h"
#include "core/dir.h"
#include "core/events.h"
#include "core/file.h"
#include "core/log.h"
#include "core/png_read.h"
#include "core/string.h"

#include "empire/object.h"

#include "scenario/data.h"
#include "scenario/scenario.h"

#include "expat.h"

#include <string.h>

#define MAX_TAG_TYPES 24
#define XML_HASH_SEED 0x12345678
#define XML_BUFFER_SIZE 1024
#define XML_MAX_DEPTH 4
#define XML_MAX_ELEMENTS_PER_DEPTH 6
#define XML_MAX_ATTRIBUTES 12
#define XML_TAG_MAX_LENGTH 36
extern int fired_events[180];

static struct {
    struct {
        char file_name[FILE_NAME_MAX];
        size_t file_name_position;
        int depth;
        int error;
        int finished;
        char current_tag[XML_TAG_MAX_LENGTH];
        custom_event current_event;
        int total_conditions_for_current_event;
        int total_custom_events;
        struct scenario_t *scenario;
    } xml;
} scenario_data;

static const char SCEN_XML_FILE_ELEMENTS[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH][XML_TAG_MAX_LENGTH] = { { "scenario" }, { "description", "event", "invasion", "message" }, { "condition" } };
static const char SCEN_XML_FILE_ATTRIBUTES[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH][XML_MAX_ATTRIBUTES][XML_TAG_MAX_LENGTH] = {
    { { "name", "startDate", "author" } }, // scenario
    { 
        { "tagline", "text" }, // description
        { "text", "title", "cityName", "resourceId", "amount", "deadlineMonths", "favorGained", "size", "god", "duration", "isSea", "figures" }, //event
        { "text", "entrypointId", "amount", "monthsWarning" }, //invasion
        { "text", "title", "header", "signature",  "sound", "advisorId"} //message
    },
    { { "value", "requirement" } } //condition
};

static const char EVENT_TYPE_TAGS[MAX_TAG_TYPES][XML_TAG_MAX_LENGTH] = {
    "demandChange",
    "priceChange",
    "request",
    "cityNowTrades",
    "wageChange",
    "festival",
    "victory",
    "tradeInterruption",
    "riot"
};

static const char INVASION_TYPE_TAGS[MAX_TAG_TYPES][XML_TAG_MAX_LENGTH] = {
    "uprising",
    "distantBattle"
};

static void scen_xml_start_scenario_element(const char** attributes);
static void scen_xml_start_description_element(const char** attributes);
static void scen_xml_start_event_element(const char** attributes);
static void scen_xml_start_invasion_element(const char** attributes);
static void scen_xml_start_message_element(const char** attributes);
static void scen_xml_start_condition_element(const char** attributes);
static void scen_xml_end_scenario_element(void);
static void scen_xml_end_description_element(void);
static void scen_xml_end_event_element(void);
static void scen_xml_end_invasion_element(void);
static void scen_xml_end_message_element(void);
static void scen_xml_end_condition_element(void);

static void (*scen_xml_start_element_callback[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH])(const char** attributes) = {
    { scen_xml_start_scenario_element }, { scen_xml_start_description_element, scen_xml_start_event_element, scen_xml_start_invasion_element, scen_xml_start_message_element }, { scen_xml_start_condition_element }
};

static void (*scen_xml_end_element_callback[XML_MAX_DEPTH][XML_MAX_ELEMENTS_PER_DEPTH])(void) = {
    { scen_xml_end_scenario_element }, { scen_xml_end_description_element, scen_xml_end_event_element, scen_xml_end_invasion_element, scen_xml_end_message_element }, { scen_xml_end_condition_element }
};

// import?
static int count_xml_attributes(const char** attributes)
{
    int total = 0;
    while (attributes[total]) {
        ++total;
    }
    return total;
}


static void scen_xml_start_scenario_element(const char** attributes)
{
    scenario_data.xml.scenario = &scenario;

    int total_attributes = count_xml_attributes(attributes);
    for (int i = 0; i < total_attributes; i += 2) {

        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[0][0][0]) == 0) {
            strcpy((char *)scenario_data.xml.scenario->scenario_name, attributes[i + 1]);
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[0][0][1]) == 0) {
            scenario_data.xml.scenario->start_year = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[0][0][2]) == 0) {
            strcpy((char*)scenario_data.xml.scenario->author, attributes[i + 1]);
        }
    }
}

static void scen_xml_start_description_element(const char** attributes)
{
    scenario_data.xml.scenario = &scenario;
    
    int total_attributes = count_xml_attributes(attributes);
    for (int i = 0; i < total_attributes; i += 2) {

        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][0][0]) == 0) {
            strcpy((char*) scenario_data.xml.scenario->brief_description, attributes[i + 1]);
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][0][1]) == 0) {
            strcpy((char*) scenario_data.xml.scenario->briefing, attributes[i + 1]);
        }
    }
}

static void scen_xml_start_event_element(const char** attributes)
{
    strcpy(scenario_data.xml.current_event.event_data.type, scenario_data.xml.current_tag);

    int total_attributes = count_xml_attributes(attributes);
    for (int i = 0; i < total_attributes; i += 2) {
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][1][0]) == 0) {
            strcpy((char*) scenario_data.xml.current_event.event_data.text, attributes[i + 1]);
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][1][1]) == 0) {
            strcpy((char*) scenario_data.xml.current_event.event_data.title, attributes[i + 1]);
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][1][2]) == 0) {
            strcpy(scenario_data.xml.current_event.event_data.city_name, attributes[i + 1]);
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][1][3]) == 0) {
            scenario_data.xml.current_event.event_data.resource_id = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][1][4]) == 0) {
            scenario_data.xml.current_event.event_data.amount = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][1][5]) == 0) {
            scenario_data.xml.current_event.event_data.deadline_months = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][1][6]) == 0) {
            scenario_data.xml.current_event.event_data.favor_gained = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][1][7]) == 0) {
            scenario_data.xml.current_event.event_data.size = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][1][8]) == 0) {
            strcpy(scenario_data.xml.current_event.event_data.god, attributes[i + 1]);
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][1][9]) == 0) {
            scenario_data.xml.current_event.event_data.duration = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][1][10]) == 0) {
            scenario_data.xml.current_event.event_data.is_sea = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][1][11]) == 0) {
            scenario_data.xml.current_event.event_data.figures = string_to_int(string_from_ascii(attributes[i + 1]));
        }
    }
}

static void scen_xml_start_invasion_element(const char** attributes)
{
    strcpy(scenario_data.xml.current_event.event_data.type, scenario_data.xml.current_tag);

    int total_attributes = count_xml_attributes(attributes);
    for (int i = 0; i < total_attributes; i += 2) {
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][2][0]) == 0) {
            strcpy((char*)scenario_data.xml.current_event.event_data.text, attributes[i + 1]);
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][2][1]) == 0) {
            scenario_data.xml.current_event.event_data.entrypoint_id = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][2][2]) == 0) {
            scenario_data.xml.current_event.event_data.amount = string_to_int(string_from_ascii(attributes[i + 1]));
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][2][3]) == 0) {
            scenario_data.xml.current_event.event_data.months_warning = string_to_int(string_from_ascii(attributes[i + 1]));
        }
    }
}

static void scen_xml_start_message_element(const char** attributes) {
    
    strcpy(scenario_data.xml.current_event.event_data.type, scenario_data.xml.current_tag);

    int total_attributes = count_xml_attributes(attributes);
    for (int i = 0; i < total_attributes; i += 2) {

        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][3][0]) == 0) {
            strcpy((char*) scenario_data.xml.current_event.event_data.text, attributes[i + 1]);
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][3][1]) == 0) {
            strcpy((char*) scenario_data.xml.current_event.event_data.title, attributes[i + 1]);
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][3][2]) == 0) {
            strcpy((char*) scenario_data.xml.current_event.event_data.header, attributes[i + 1]);
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][3][3]) == 0) {
            strcpy((char*) scenario_data.xml.current_event.event_data.signature, attributes[i + 1]);
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][3][4]) == 0) {
            strcpy(scenario_data.xml.current_event.event_data.sound, attributes[i + 1]);
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[1][3][5]) == 0) {
            scenario_data.xml.current_event.event_data.advisor_id = string_to_int(string_from_ascii(attributes[i + 1]));
        }
    }
}

static void scen_xml_start_condition_element(const char** attributes)
{
    int total_attributes = count_xml_attributes(attributes);
    for (int i = 0; i < total_attributes; i += 2) {

        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[2][0][0]) == 0) {
            strcpy(scenario_data.xml.current_event.conditions[scenario_data.xml.total_conditions_for_current_event].value, attributes[i + 1]);
        }
        if (strcmp(attributes[i], SCEN_XML_FILE_ATTRIBUTES[2][0][1]) == 0) {
            scenario_data.xml.current_event.conditions[scenario_data.xml.total_conditions_for_current_event].requirement = string_to_int(string_from_ascii(attributes[i + 1]));
        }
    }
}


static void scen_xml_end_scenario_element(void)
{
    scenario_data.xml.total_custom_events = 0;
    scenario_data.xml.finished = 1;
}

static void scen_xml_end_description_element(void)
{
}

static void scen_xml_end_message_element(void) {
    scenario_data.xml.current_event.in_use = 1;
    if (fired_events[scenario_data.xml.total_custom_events]) {
        scenario_data.xml.current_event.fired = 1;
    }
    custom_events[scenario_data.xml.total_custom_events] = scenario_data.xml.current_event;
    scenario_data.xml.total_conditions_for_current_event = 0;
    scenario_data.xml.total_custom_events++;
    memset(&scenario_data.xml.current_event, 0, sizeof(custom_event));
}


static void scen_xml_end_event_element(void)
{
    scenario_data.xml.current_event.in_use = 1;
    if (fired_events[scenario_data.xml.total_custom_events]) {
        scenario_data.xml.current_event.fired = 1;
    }
    custom_events[scenario_data.xml.total_custom_events] = scenario_data.xml.current_event;
    scenario_data.xml.total_conditions_for_current_event = 0;
    scenario_data.xml.total_custom_events++;
    memset(&scenario_data.xml.current_event, 0, sizeof(custom_event));
}

static void scen_xml_end_invasion_element(void)
{
    scenario_data.xml.current_event.in_use = 1;
    if (fired_events[scenario_data.xml.total_custom_events]) {
        scenario_data.xml.current_event.fired = 1;
    }
    custom_events[scenario_data.xml.total_custom_events] = scenario_data.xml.current_event;
    scenario_data.xml.total_conditions_for_current_event = 0;
    scenario_data.xml.total_custom_events++;
    memset(&scenario_data.xml.current_event, 0, sizeof(custom_event));
}

static void scen_xml_end_condition_element(void)
{
    scenario_data.xml.total_conditions_for_current_event++;
}

// import?
static int get_element_index(const char* name)
{
    for (int i = 0; i < MAX_TAG_TYPES; ++i) {
        if (strcmp(name, EVENT_TYPE_TAGS[i]) == 0) {
            name = "event";
            continue;
        }
    }    

    for (int i = 0; i < MAX_TAG_TYPES; ++i) {
        if (strcmp(name, INVASION_TYPE_TAGS[i]) == 0) {
            name = "invasion";
            continue;
        }
    }
    
    for (int i = 0; i < XML_MAX_ELEMENTS_PER_DEPTH; ++i) {
        if (SCEN_XML_FILE_ELEMENTS[scenario_data.xml.depth][i][0] == 0) {
            continue;
        }
        if (strcmp(SCEN_XML_FILE_ELEMENTS[scenario_data.xml.depth][i], name) == 0) {
            return i;
        }
    }
    return -1;
}

//import?
static void XMLCALL xml_start_element(void* unused, const char* name, const char** attributes)
{
    if (scenario_data.xml.error) {
        return;
    }
    if (scenario_data.xml.finished || scenario_data.xml.depth == XML_MAX_DEPTH) {
        scenario_data.xml.error = 1;
        log_error("Invalid XML parameter", name, 0);
        return;
    }
    int index = get_element_index(name);
    if (index == -1) {
        scenario_data.xml.error = 1;
        log_error("Invalid XML parameter", name, 0);
        return;
    }
    strcpy(scenario_data.xml.current_tag, name);
    (*scen_xml_start_element_callback[scenario_data.xml.depth][index])(attributes);
    scenario_data.xml.depth++;    
}

//import?
static void XMLCALL xml_end_element(void* unused, const char* name)
{
    if (scenario_data.xml.error) {
        return;
    }
    scenario_data.xml.depth--;
    int index = get_element_index(name);
    if (index == -1) {
        scenario_data.xml.error = 1;
        log_error("Invalid XML parameter", name, 0);
        return;
    }
    (*scen_xml_end_element_callback[scenario_data.xml.depth][index])();
}


//import?
static void clear_xml_info(void)
{
    scenario_data.xml.error = 0;
    scenario_data.xml.depth = 0;
    scenario_data.xml.finished = 0;
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
        if (XML_Parse(parser, buffer, (int)bytes_read, done) == XML_STATUS_ERROR || scenario_data.xml.error) {
            log_error("Error parsing file", xml_file_name, 0);
            break;
        }
    } while (!done);

    if (scenario_data.xml.error || !scenario_data.xml.finished) {
        log_info("scenario xml load failed", "", 0);
    }

    clear_xml_info();

    XML_ParserFree(parser);
    file_close(xml_file);
}


void scenario_load_from_file(const char* file_path)
{
   
    process_file(file_path);
    load_all_custom_messages();

}

void clear_custom_events(void) {
    for (int i = 0; i < MAX_CUSTOM_EVENTS; i++) {
        memset(&custom_events[i], 0, sizeof(custom_event));
    }
}