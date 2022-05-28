#include "events.h"

#include "game/time.h"
#include "city/constants.h"
#include "city/festival.h"
#include "city/finance.h"
#include "city/labor.h"
#include "city/population.h"
#include "city/ratings.h"
#include "city/sentiment.h"
#include "city/trade.h"
#include "city/victory.h"
#include "core/string.h"
#include "empire/object.h"
#include "empire/trade_prices.h"
#include "empire/trade_route.h"
#include "figuretype/crime.h"
#include "scenario/data.h"
#include "scenario/invasion.h"
#include "scenario/request.h"
#include "scenario/scenario.h"

#include <string.h>
#include <math.h>

custom_event custom_events[MAX_CUSTOM_EVENTS];

static god_mapping god_mappings[] = { 
	{"ceres", GOD_CERES},
	{"neptune", GOD_NEPTUNE},
	{"mercury", GOD_MERCURY},
	{"mars", GOD_MARS},
	{"venus", GOD_VENUS}
};

int get_months_passed() {
	return game_time_total_months();
}

int get_city_id_from_name(custom_event_data event_data) {
	for (int i = 0; i < MAX_CITIES; ++i) {
		if (strcmp(event_data.city_name, (char *)cities[i].display_name) == 0) {
			return i;
		}
	}

	return 0;
}

void perform_demand_change(custom_event_data event_data)
{
	int city_id = get_city_id_from_name(event_data);
	int route = empire_city_get_route_id(city_id);
    int resource = event_data.resource_id;
	int limit = event_data.amount;

	trade_route_change_limit(route, resource, limit);
	if (event_data.message_id) {
		city_message_post(1, event_data.message_id, city_id, resource);
	}
	else {
		city_message_post(1, MESSAGE_INCREASED_TRADING, city_id, resource);
	}
}

void perform_price_change(custom_event_data event_data)
{
	int resource = event_data.resource_id;
	int amount = event_data.amount;

	trade_price_change(resource, amount);
	if (event_data.message_id) {
		city_message_post(1, event_data.message_id, amount, resource);
	} else {
		if (amount > 0) {
			city_message_post(1, MESSAGE_PRICE_INCREASED, amount, resource);
		}
		else {
			city_message_post(1, MESSAGE_PRICE_DECREASED, amount, resource);
		}
	}
}

void perform_wage_change(custom_event_data event_data) {
	int amount = event_data.amount;

	city_labor_change_wages(amount);
	if (event_data.message_id) {
		city_message_post(1, event_data.message_id, 0, 0);
	}
	else {
		if (amount > 0) {
			city_message_post(1, MESSAGE_ROME_RAISES_WAGES, 0, 0);
		}
		else {
			city_message_post(1, MESSAGE_ROME_LOWERS_WAGES, 0, 0);
		}
	}
}


void make_request(custom_event_data event_data)
{
	int resource = event_data.resource_id;
	int amount = event_data.amount;
	int favor = event_data.favor_gained;
	int deadline = event_data.deadline_months;
	int nextRequestId = 0;

	for (int i = 0; i < MAX_REQUESTS; i++) {
		if (scenario.requests[i].state == 0 && scenario.requests[i].visible == 0) {
			nextRequestId = i;
			break;
		}
	}

	scenario.requests[nextRequestId].resource = resource;
	scenario.requests[nextRequestId].amount = amount;
	scenario.requests[nextRequestId].favor = favor;
	scenario.requests[nextRequestId].visible = 1;
	scenario.requests[nextRequestId].months_to_comply = deadline + 1;

	if (event_data.message_id) {
		city_message_post(1, event_data.message_id, nextRequestId, 0);
	}
	else {
		city_message_post(1, MESSAGE_CAESAR_REQUESTS_GOODS, nextRequestId, 0);
	}

	scenario_request_process();
}

void start_custom_invasion(custom_event_data event_data)
{
	int invasion_type = 0;
	int attack_type = 0;
	int months_warning = 0;

	int amount = event_data.amount;
	int from = event_data.entrypoint_id;

	int nextInvasionId = 0;

	for (int i = 0; i < MAX_INVASIONS; i++) {
		if (scenario.invasions[i].amount == 0) {
			nextInvasionId = i;
			break;
		}
	}

	if (strcmp(event_data.type, "distantBattle") == 0) {
		invasion_type = INVASION_TYPE_DISTANT_BATTLE;
		scenario.invasions[nextInvasionId].type = invasion_type;
		scenario.invasions[nextInvasionId].amount = amount;
		return;
	}

	if (strcmp(event_data.type, "uprising") == 0) {
		invasion_type = INVASION_TYPE_LOCAL_UPRISING;
		attack_type = 2;
	}
	else {
		invasion_type = INVASION_TYPE_ENEMY_ARMY;
		attack_type = 0;
		months_warning = event_data.months_warning;
	}

	scenario.invasions[nextInvasionId].type = invasion_type;
	scenario.invasions[nextInvasionId].amount = amount;
	scenario.invasions[nextInvasionId].from = from;
	scenario.invasions[nextInvasionId].attack_type = attack_type;

	if (months_warning == 0) {		
		//fire invasion
		scenario.invasions[nextInvasionId].year = scenario.start_year - game_time_year();
		scenario.invasions[nextInvasionId].month = game_time_month();
	} 
	else {
		//schedule
		int invasion_year = months_warning / 12;
		int invasion_month = game_time_month() + (months_warning % 12);
		scenario.invasions[nextInvasionId].year = invasion_year;
		scenario.invasions[nextInvasionId].month = invasion_month;
		
		//create warnings
		int path_current = 1;

		//invasion_warning* warning = &invasion_warning_data.warnings[1];
		for (int year = 0; year < 8; year++) {
			int obj_year = year;
			if (obj_year == 0) {
				obj_year = 1;
			}
			const empire_object* obj = empire_object_get_battle_icon(path_current, obj_year);
			//if (obj) {
			//	warning->empire_object_id = obj->id;
			//	warning->invasion_path_id = obj->invasion_path_id;
			//	warning->warning_years = obj->invasion_years;
			//	warning->x = obj->x;
			//	warning->y = obj->y;
			//	warning->image_id = obj->image_id;
			//}

			//warning->in_use = 1;
			//warning->invasion_id = nextInvasionId;			
			//warning->months_to_go += 12 * year;
			//++warning;
		}
		path_current++;
		
	}
}

void city_now_trades(custom_event_data event_data) {
	int city_id = get_city_id_from_name(event_data);
	cities[city_id].type = 2;

	if (event_data.message_id) {
		city_message_post(1, event_data.message_id, 0, 0);
	}
	else {
		city_message_post(1, MESSAGE_EMPIRE_HAS_EXPANDED, 0, 0);
	}	
}

void show_message(custom_event_data event_data) {
	city_message_post(1, event_data.message_id, 0, 0);
}

void start_festival(custom_event_data event_data) {
	int god_id = 0;
	for (int i = 0; i <= 4; ++i) {
		if (strcmp(event_data.god, god_mappings[i].god_string) == 0) {
			god_id = god_mappings[i].god_id;
		}
	}
	festival_sentiment_and_deity(event_data.size, god_id);
	if (event_data.message_id) {
		city_message_post(1, event_data.message_id, 0, 0);
	}
	else {
		city_message_post(1, MESSAGE_SMALL_FESTIVAL, 0, 0);
	}
}

void start_riot(custom_event_data event_data) {
	figure_force_riot(event_data.figures);
	if (event_data.message_id) {
		city_message_post(1, event_data.message_id, 0, 0);
	}
	else {
		city_message_post(1, MESSAGE_RIOT, 0, 0);
	}
}

void declare_victory(void) {
	city_victory_force_win();
}

void interrupt_trade(custom_event_data event_data) {
	if (event_data.is_sea) {
		city_trade_start_sea_trade_problems(event_data.duration);
	}
	else {
		city_trade_start_land_trade_problems(event_data.duration);
	}	

	if (event_data.message_id) {
		city_message_post(1, event_data.message_id, 0, 0);
	}
	else {
		if (event_data.is_sea) {
			city_message_post(1, MESSAGE_SEA_TRADE_DISRUPTED, 0, 0);
		}
		else {
			city_message_post(1, MESSAGE_LAND_TRADE_DISRUPTED_LANDSLIDES, 0, 0);
		}
	}


}

static condition_value all_condition_values[] = {
	{CONDITION_VALUE_MONTHS_PASSED, "monthsPassed", get_months_passed},
	{CONDITION_VALUE_RATING_CULTURE, "culture", city_rating_culture},
    {CONDITION_VALUE_RATING_PEACE, "peace", city_rating_peace},
	{CONDITION_VALUE_RATING_PROSPERITY,	"prosperity", city_rating_prosperity},
	{CONDITION_VALUE_RATING_FAVOR,	"favor", city_rating_favor},
	{CONDITION_VALUE_MONEY, "treasury", city_finance_treasury},
	{CONDITION_VALUE_POPULATION, "population", city_population},
	//{CONDITION_VALUE_TRADE_CITY_OPEN, "tradeRouteIsOpen"},
	{CONDITION_VALUE_CITY_SENTIMENT, "sentiment", city_sentiment}, // 0-100
	{CONDITION_VALUE_PATRICIANS, "numPatricians", city_population_in_villas_palaces},
	{CONDITION_VALUE_PERCENT_IN_TENTS, "percentTents", percentage_city_population_in_tents_shacks},
	{CONDITION_VALUE_PERCENT_PATRICIANS, "percentPatricians", percentage_city_population_in_villas_palaces},
	{CONDITION_VALUE_PERCENT_UNEMPLOYED, "percentUnemployed", city_labor_unemployment_percentage},
	//{CONDITION_VALUE_ARMY_STRENGTH,	"armyStrength"}
};

static custom_event_type all_custom_event_types[] = {
	{EVENT_TYPE_INVALID, "invalid", 0, 0},
	{EVENT_TYPE_DEMAND_CHANGE, "demandChange", perform_demand_change, MESSAGE_INCREASED_TRADING},
	{EVENT_TYPE_PRICE_CHANGE, "priceChange", perform_price_change, MESSAGE_PRICE_INCREASED},
	{EVENT_TYPE_REQUEST, "request", make_request, MESSAGE_CAESAR_REQUESTS_GOODS},
	{EVENT_TYPE_INVASION, "invasion", start_custom_invasion, MESSAGE_DISTANT_BATTLE},
	{EVENT_TYPE_UPRISING, "uprising", start_custom_invasion, MESSAGE_DISTANT_BATTLE},
	{EVENT_TYPE_DISTANT_BATTLE, "distantBattle", start_custom_invasion, MESSAGE_CAESAR_REQUESTS_ARMY},
	//{EVENT_TYPE_EARTHQUAKE, "earthquake", start_earthquake}, 
	{EVENT_TYPE_RIOTS, "riot", start_riot, MESSAGE_RIOT},
	//{EVENT_TYPE_SENTIMENT}
	//{EVENT_TYPE_CITY_FALLS, "cityNowTrades", city_now_trades, MESSAGE_EMPIRE_HAS_EXPANDED},
	{EVENT_TYPE_WAGE_CHANGE, "wageChange", perform_wage_change, MESSAGE_ROME_RAISES_WAGES},
	{EVENT_TYPE_CITY_NOW_TRADES, "cityNowTrades", city_now_trades, MESSAGE_EMPIRE_HAS_EXPANDED},
	{EVENT_TYPE_MESSAGE, "message", show_message, 2},
	{EVENT_TYPE_FESTIVAL, "festival", start_festival, MESSAGE_SMALL_FESTIVAL},
	{EVENT_TYPE_VICTORY, "victory", declare_victory, 2},
	{EVENT_TYPE_TRADE_INTERRUPTION, "tradeInterruption", interrupt_trade, MESSAGE_SEA_TRADE_DISRUPTED}
};


custom_event_type get_event_type(custom_event_data event_data) {
	for (int i = 0; i <= CONDITION_VALUE_MAX_KEY; ++i) {
		if (strcmp(event_data.type, all_custom_event_types[i].event_type_string) == 0) {
			return all_custom_event_types[i];
		}
	}
	return all_custom_event_types[0];
}

int test_condition(event_condition condition) {
	int valueType = 0;
	for (int i = 0; i <= CONDITION_VALUE_MAX_KEY; ++i) {
		if (strcmp(condition.value, all_condition_values[i].value_type_string) == 0) {
			valueType = i;
			break;
		}
	}

	if (all_condition_values[valueType].actual_value_function() >= condition.requirement) {
		return 1;
	}
	return 0;
}

int test_conditions_for_event(custom_event event) {
	for (int i = 0; i < MAX_CONDITIONS; ++i) {
		event_condition condition = event.conditions[i];
		if (!strlen(condition.value)) {
			break;
		}
		if (!test_condition(condition)) {
			return 0;
		}
	}

	return 1;
}

void custom_events_process() {
	for (int i = 0; i < MAX_CUSTOM_EVENTS; ++i) {
		if (custom_events[i].in_use && !custom_events[i].fired) {
			if (test_conditions_for_event(custom_events[i])) {
				custom_event_type event_type = get_event_type(custom_events[i].event_data);

				event_type.activation_function(custom_events[i].event_data);
				custom_events[i].fired = 1;
			}
		}
	}
}

void load_all_custom_messages() {
	int total_custom_messages = 0;
	for (int i = 0; i < MAX_CUSTOM_EVENTS; ++i) {
		if (custom_events[i].in_use && !string_is_empty(custom_events[i].event_data.text)) {
			lang_message* m = get_next_message_entry();
			custom_event_type event_type = get_event_type(custom_events[i].event_data);			
			city_message_type default_message =	event_type.city_message_type;
			m->type = TYPE_MESSAGE;
			int text_id = city_message_get_text_id(default_message);
			m->message_type = lang_get_message(text_id)->message_type;
			m->image = lang_get_message(text_id)->image;
			m->video = lang_get_message(text_id)->video;					

			if (!string_is_empty(custom_events[i].event_data.title)) {
				m->title.text = custom_events[i].event_data.title;
			}
			else {
				m->title.text = lang_get_message(text_id)->title.text;
			}
			if (!string_is_empty(custom_events[i].event_data.header)) {
				m->subtitle.text = custom_events[i].event_data.header;
				m->subtitle.x = 20;
				m->subtitle.y = 50;
			}
			if (!string_is_empty(custom_events[i].event_data.signature)) {
				m->signature.text = custom_events[i].event_data.signature;
			}
			m->content.text = custom_events[i].event_data.text;	
			m->height_blocks = ((int)string_length(m->content.text) / 20);

			if (m->height_blocks < 16) {
				m->height_blocks = 16;
			}

			m->width_blocks = 42;			

			if (strlen(custom_events[i].event_data.sound) > 0) {
				m->custom_sound_filename = custom_events[i].event_data.sound;
			}

			custom_events[i].event_data.message_id = MESSAGE_MAX_KEY + total_custom_messages;
			total_custom_messages++;
		}
	}	
}
