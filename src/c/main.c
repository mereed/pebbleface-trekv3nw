/*
Copyright (C) 2017 Mark Reed / Little Gem Software

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), 
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include <pebble.h>
#include "main.h"
#include "languages.h"
#include "effect_layer.h"
	


// All UI elements
static Window         *window;
static Layer 		  *window_layer;

EffectLayer  		  *effect_layer;
EffectLayer 		  *effect_layer2;

static GBitmap        *battery_image;
static GBitmap        *bluetooth_image;
static GBitmap 		  *icon_bitmap = NULL;

static BitmapLayer    *battery_layer;
static BitmapLayer    *bluetooth_layer;
static BitmapLayer    *icon_layer;

static TextLayer      *text_time_layer;
static TextLayer      *text_ampm_layer;
static TextLayer      *text_secs_layer;
static TextLayer      *text_days_layer;
static TextLayer      *text_date_layer;
static TextLayer      *text_week_layer;
static TextLayer	  *battery_text_layer;
static TextLayer 	  *temp_layer;

//static GFont          font_round_time;
static GFont          font_time;
static GFont          font_days;
static GFont          font_date;
static GFont          font_other;
static GFont          small_batt;
static GFont          small_batt2;

int charge_percent = 0;

static GBitmap *block24_img;
static BitmapLayer *block24_layer;

static GBitmap *background_image;
static BitmapLayer *background_layer;


// Define layer rectangles (x, y, width, height)

#ifdef PBL_PLATFORM_CHALK
GRect TIME_RECT      = ConstantGRect(  36,   11, 160,  72 );
GRect AMPM_RECT      = ConstantGRect( 134,  25,  30,  21 );
GRect SECS_RECT      = ConstantGRect( 132,  27,  30,  21 );
GRect DATE_RECT      = ConstantGRect(  0, 0, 0,  0 );
GRect WEEK_RECT      = ConstantGRect(  40, 132, 180,  50 );
GRect DAYS_RECT      = ConstantGRect(  38,  104, 140,  30 );
GRect BATT_RECT      = ConstantGRect(  72,  83,  60,  11 );
GRect BT_RECT        = ConstantGRect( 155, 83,  28,  21 );
GRect EMPTY_RECT     = ConstantGRect(   0,   0,   0,   0 );
GRect OFF_DATE_RECT  = ConstantGRect(  0, 0, 0,  0 );
GRect OFF_WEEK_RECT  = ConstantGRect(  38, 132, 180,  70 );
GRect TEMP_RECT      = ConstantGRect(  140,  57, 40,  40 );
GRect ICON_RECT      = ConstantGRect(  0,  0, 0,  0 );

#else
GRect TIME_RECT      = ConstantGRect(  29,   5, 115,  72 );
GRect AMPM_RECT      = ConstantGRect( 123,   0,  25,  21 );
GRect SECS_RECT      = ConstantGRect( 123,   0,  23,  21 );
GRect DATE_RECT      = ConstantGRect(  11, 134,  80,  50 );
GRect WEEK_RECT      = ConstantGRect(  0, 132,  143,  50 );
GRect DAYS_RECT      = ConstantGRect(  17,  95, 140,  30 );
GRect BATT_RECT      = ConstantGRect(  83,  79,  48,  17 );
GRect BT_RECT        = ConstantGRect( 131,  79,  14,  17 );
GRect EMPTY_RECT     = ConstantGRect(   0,   0,   0,   0 );
GRect OFF_DATE_RECT  = ConstantGRect(  11, 132, 80,  50 );
GRect OFF_WEEK_RECT  = ConstantGRect(  0, 132, 143,  50 );
GRect TEMP_RECT      = ConstantGRect(   18,  53, 40,  40 );
GRect ICON_RECT      = ConstantGRect(   16,  21, 20,  20 );
#endif

// Define placeholders for time and date
static char time_text[] = "00:00";
static char ampm_text[] = "XXX";
static char secs_text[] = "00";
 
// Previous bluetooth connection status
static bool prev_bt_status = false;

#ifdef LANGUAGE_TESTING
  static int ct = 1;
  static int speed = 5;
#endif

// A struct for our specific settings (see main.h)
ClaySettings settings;

int stringToInt(char *str);


// Initialize the default settings
static void prv_default_settings() {	
  settings.current_background = 0;
  settings.bluetoothvibe_status = false;
  settings.ampmsecs = false;
  settings.textcol_format = false;
  settings.steps_status = false;
  settings.invert = false;
  settings.format = 0;
  settings.current_status = true;
  settings.current_language = 0;
  settings.startday_status = false;
}

#ifdef PBL_HEALTH
static TextLayer *steps_label;
static GBitmap *footprint_icon;
static BitmapLayer *footprint_layer;

static void health_handler(HealthEventType event, void *context) {
  static char s_value_buffer[20];
  if (event == HealthEventMovementUpdate) {
    // display the step count
    snprintf(s_value_buffer, sizeof(s_value_buffer), "%d", (int)health_service_sum_today(HealthMetricStepCount));	  
    text_layer_set_text(steps_label, s_value_buffer);
  }
}
#endif


/*
  Setup new TextLayer
*/
static TextLayer * setup_text_layer( GRect rect, GTextAlignment align , GFont font ) {
  TextLayer *newLayer = text_layer_create( rect );
  text_layer_set_text_color( newLayer, GColorWhite );
  text_layer_set_background_color( newLayer, GColorClear );
  text_layer_set_text_alignment( newLayer, align );
  text_layer_set_font( newLayer, font );

  return newLayer;
}


/*
  Handle tick events
*/
void handle_tick( struct tm *tick_time, TimeUnits units_changed ) {
	
  // Update text layer for current day
	
  if (settings.startday_status) {
  int today = tick_time->tm_wday ; if ( today < 0 ) { today = 6; }
  layer_set_frame( effect_layer_get_layer(effect_layer2), highlight_rect2[settings.current_language][today] );
	} else {
  int today = tick_time->tm_wday - 1; if ( today < 0 ) { today = 6; }
  layer_set_frame( effect_layer_get_layer(effect_layer2), highlight_rect[settings.current_language][today] );		
	}

  #ifdef LANGUAGE_TESTING
  layer_set_frame( effect_layer_get_layer(effect_layer2), hightlight_rect[settings.current_language][ct] );
    if ( tick_time->tm_sec % speed == 0 ) { ct++; }
    if ( ct == 7 ) { ct = 0; }
  #endif

  strftime( date_text, sizeof( date_text ), date_formats[settings.current_language], tick_time );
  if ( settings.current_language == 0 ) {
    strcat( date_text, ordinal_numbers[tick_time->tm_mday - 1] );
  } else {
    strcat( date_text, month_names_arr[settings.current_language][tick_time->tm_mon] );
  }
  text_layer_set_text( text_date_layer, date_text );

  // Update week or day of the year (i.e. Week 15 or 2013-118)
  if ( settings.format == 0 ) {
    strftime( week_text, sizeof( week_text ), week_formats[settings.current_language], tick_time );
    text_layer_set_text( text_week_layer, week_text );
  } else {
    strftime( alt_text, sizeof( alt_text ), alt_formats[settings.format], tick_time );
    text_layer_set_text( text_week_layer, alt_text );
  }

  // Display hours (i.e. 18 or 06)
  strftime( time_text, sizeof( time_text ), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time );

  // Remove leading zero (only in 12h-mode)
  if ( !clock_is_24h_style() && (time_text[0] == '0') ) {
    memmove( time_text, &time_text[1], sizeof( time_text ) - 1 );
  }
  text_layer_set_text( text_time_layer, time_text );

  // Update AM/PM indicator (i.e. AM or PM or nothing when using 24-hour style)
  strftime( ampm_text, sizeof( ampm_text ), clock_is_24h_style() ? "" : "%p", tick_time );
  text_layer_set_text( text_ampm_layer, ampm_text );

  // Display seconds 
  strftime( secs_text, sizeof( secs_text ), "%S", tick_time );
  text_layer_set_text( text_secs_layer, secs_text );
	
}

// Read settings from persistent storage
static void prv_load_settings() {
  // Load the default settings
  prv_default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  // Update the display based on new settings
  prv_update_display();
}

/*
  Force update of time
*/
void update_time() {
	
	 if (settings.startday_status) {
     	  	layer_mark_dirty( text_layer_get_layer( text_days_layer));
			text_layer_set_text( text_days_layer, day_lines2[settings.current_language] );
	  		} else {
		  	layer_mark_dirty( text_layer_get_layer( text_days_layer));
			text_layer_set_text( text_days_layer, day_lines[settings.current_language] );
	  		}
	
  // Get current time
  time_t now = time( NULL );
  struct tm *tick_time = localtime( &now );

  // Force update to avoid a blank screen at startup of the watchface
  handle_tick( tick_time, MONTH_UNIT + DAY_UNIT + HOUR_UNIT + MINUTE_UNIT + SECOND_UNIT );

}


// Update the display elements
static void prv_update_display() {
	  
	if (settings.ampmsecs) {
			layer_set_hidden(text_layer_get_layer(text_secs_layer), false);
	 		layer_set_hidden(text_layer_get_layer(text_ampm_layer), true);
	    	tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
   	   }
       else {
		   	layer_set_hidden(text_layer_get_layer(text_secs_layer), true);
	    	layer_set_hidden(text_layer_get_layer(text_ampm_layer), false);
	    	tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
      }	
	

	  if (settings.textcol_format) {	  
		  	text_layer_set_text_color(text_time_layer, GColorWhite);
			text_layer_set_text_color(text_week_layer, GColorWhite);
		#ifdef PBL_COLOR	
		 text_layer_set_text_color(text_time_layer, GColorChromeYellow);
		 text_layer_set_text_color(text_week_layer, GColorChromeYellow);
		#endif		

	  } else {
		   text_layer_set_text_color(text_time_layer, GColorWhite);
		   text_layer_set_text_color(text_week_layer, GColorWhite);
	  }	


	  if (settings.steps_status) {
#ifdef PBL_HEALTH
		  
		health_service_events_subscribe(health_handler, NULL);

    // force initial steps display
		health_handler(HealthEventMovementUpdate, NULL);
 
		layer_set_hidden(text_layer_get_layer(steps_label), false); 
		layer_set_hidden(bitmap_layer_get_layer(footprint_layer), false); 
            
        } else {
		  
        health_service_events_unsubscribe();
				
		layer_set_hidden(text_layer_get_layer(steps_label), true); 
		layer_set_hidden(bitmap_layer_get_layer(footprint_layer), true); 
#endif

	  }

	
if (settings.invert) {
    // Add inverter layer
    Layer *window_layer = window_get_root_layer(window);

	//creating effect layer with inverter effect
#ifdef PBL_PLATFORM_CHALK
  effect_layer = effect_layer_create(GRect(0,0,180,180));
#else
  effect_layer = effect_layer_create(GRect(0,0,144,168));
#endif
  effect_layer_add_effect(effect_layer, effect_invert, NULL);
  layer_add_child((window_layer), effect_layer_get_layer(effect_layer));
    
  } else if (effect_layer != NULL) {
    // hide Inverter layer
    layer_remove_from_parent(effect_layer_get_layer(effect_layer));
    effect_layer_destroy(effect_layer);
    effect_layer = NULL;
  } 
	

	
	switch (settings.current_background) {

		case 0:
		
		if (background_image) {
		gbitmap_destroy(background_image);
		background_image = NULL;
    	}
		background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND1);
		
	    if (background_image != NULL) {
		bitmap_layer_set_bitmap(background_layer, background_image);
		layer_set_hidden(bitmap_layer_get_layer(background_layer), false);
		layer_mark_dirty(bitmap_layer_get_layer(background_layer));
    	}		
		break;

		case 1:
		
		if (background_image) {
		gbitmap_destroy(background_image);
		background_image = NULL;
    	}
		background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND2);
		
		if (background_image != NULL) {
		bitmap_layer_set_bitmap(background_layer, background_image);
		layer_set_hidden(bitmap_layer_get_layer(background_layer), false);
		layer_mark_dirty(bitmap_layer_get_layer(background_layer));
		}			   
		break;

		case 2:
		
		if (background_image) {
		gbitmap_destroy(background_image);
		background_image = NULL;
    	}
		background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND3);
		
		if (background_image != NULL) {
		bitmap_layer_set_bitmap(background_layer, background_image);
		layer_set_hidden(bitmap_layer_get_layer(background_layer), false);
		layer_mark_dirty(bitmap_layer_get_layer(background_layer));
		}	   
		break;

		case 3:
		if (background_image) {
		gbitmap_destroy(background_image);
		background_image = NULL;
    	}
		background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND4);
		
		if (background_image != NULL) {
		bitmap_layer_set_bitmap(background_layer, background_image);
		layer_set_hidden(bitmap_layer_get_layer(background_layer), false);
		layer_mark_dirty(bitmap_layer_get_layer(background_layer));
		}
		break;
		
		case 4:
		if (background_image) {
		gbitmap_destroy(background_image);
		background_image = NULL;
    	}
		background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND5);
		
		if (background_image != NULL) {
		bitmap_layer_set_bitmap(background_layer, background_image);
		layer_set_hidden(bitmap_layer_get_layer(background_layer), false);
		layer_mark_dirty(bitmap_layer_get_layer(background_layer));
		}
		break;

		case 5:
		if (background_image) {
		gbitmap_destroy(background_image);
		background_image = NULL;
    	}
		background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND6);
		
		if (background_image != NULL) {
		bitmap_layer_set_bitmap(background_layer, background_image);
		layer_set_hidden(bitmap_layer_get_layer(background_layer), false);
		layer_mark_dirty(bitmap_layer_get_layer(background_layer));
		}	   
		break;

		case 6:
		if (background_image) {
		gbitmap_destroy(background_image);
		background_image = NULL;
    	}
		background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND7);
		
		if (background_image != NULL) {
		bitmap_layer_set_bitmap(background_layer, background_image);
		layer_set_hidden(bitmap_layer_get_layer(background_layer), false);
		layer_mark_dirty(bitmap_layer_get_layer(background_layer));
		}	   
		break;

		case 7: // blue/yellows
		if (background_image) {
		gbitmap_destroy(background_image);
		background_image = NULL;
    	}
		background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND8);
		
		if (background_image != NULL) {
		bitmap_layer_set_bitmap(background_layer, background_image);
		layer_set_hidden(bitmap_layer_get_layer(background_layer), false);
		layer_mark_dirty(bitmap_layer_get_layer(background_layer));
		}	   
		break;
		
		case 8:  // blue-red-orange
		if (background_image) {
		gbitmap_destroy(background_image);
		background_image = NULL;
    	}
		background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND9);
		
		if (background_image != NULL) {
		bitmap_layer_set_bitmap(background_layer, background_image);
		layer_set_hidden(bitmap_layer_get_layer(background_layer), false);
		layer_mark_dirty(bitmap_layer_get_layer(background_layer));
		}	   
		break;

		case 9:
		if (background_image) {
		gbitmap_destroy(background_image);
		background_image = NULL;
    	}
		background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND10);
		
		if (background_image != NULL) {
		bitmap_layer_set_bitmap(background_layer, background_image);
		layer_set_hidden(bitmap_layer_get_layer(background_layer), false);
		layer_mark_dirty(bitmap_layer_get_layer(background_layer));
		}	   
		break;
		
		case 10:
		if (background_image) {
		gbitmap_destroy(background_image);
		background_image = NULL;
    	}
		background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND11);
		
		if (background_image != NULL) {
		bitmap_layer_set_bitmap(background_layer, background_image);
		layer_set_hidden(bitmap_layer_get_layer(background_layer), false);
		layer_mark_dirty(bitmap_layer_get_layer(background_layer));
		}	   
		break;

    }
		
// Force update to avoid a blank screen at startup of the watchface
  update_time();
	
}
	

int stringToInt(char *str) {
    int i=0,sum=0;
    while(str[i]!='\0'){
         if(str[i]< 48 || str[i] > 57){
            // if (DEBUG) APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to convert it into integer.");
          //   return 0;
         }
         else{
             sum = sum*10 + (str[i] - 48);
             i++;
         }
    }
    return sum;
}

// Handle the response from AppMessage
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
	
  // background
  Tuple *bg_t = dict_find(iter, MESSAGE_KEY_current_background);
  if (bg_t) {
    settings.current_background = stringToInt((char*) bg_t->value->data);
  }
 // day
  Tuple *hands_t = dict_find(iter, MESSAGE_KEY_textcol_format);
  if (hands_t) {
    settings.textcol_format = hands_t->value->int32 == 1;
  }
 // date
  Tuple *date_t = dict_find(iter, MESSAGE_KEY_steps_status);
  if (date_t) {
    settings.steps_status = date_t->value->int32 == 1;
  }
// secs
  Tuple *secs_t = dict_find(iter, MESSAGE_KEY_ampmsecs);
  if (secs_t) {
    settings.ampmsecs = secs_t->value->int32 == 1;
  }
 // batt
  Tuple *batt_t = dict_find(iter, MESSAGE_KEY_startday_status);
  if (batt_t) {
    settings.startday_status = batt_t->value->int32 == 1;
  }
 // Hour hand Color
  Tuple *hr_color_t = dict_find(iter, MESSAGE_KEY_invert);
  if (hr_color_t) {
    settings.invert = hr_color_t->value->int32 == 1;
  }
 // Minute hand Color
  Tuple *min_color_t = dict_find(iter, MESSAGE_KEY_format);
  if (min_color_t) {
    settings.format = stringToInt((char*) min_color_t->value->data);
  }
 // Second hand Color
  Tuple *sec_color_t = dict_find(iter, MESSAGE_KEY_current_status);
  if (sec_color_t) {
    settings.current_status = sec_color_t->value->int32 == 1;
  }	  
	  // center dot Color
  Tuple *cx_color_t = dict_find(iter, MESSAGE_KEY_current_language);
  if (cx_color_t) {
    settings.current_language = stringToInt((char*) cx_color_t->value->data);
  }

// Save the new settings to persistent storage
  prv_save_settings();
}


/*
  Handle bluetooth events
*/
void handle_bluetooth( bool connected ) {
  if ( bluetooth_image != NULL ) {
    gbitmap_destroy( bluetooth_image );
  }
	
  if ( connected ) {
    bluetooth_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BLUETOOTH );
	  
  } else {
    bluetooth_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_NO_BLUETOOTH );
	  
    if ( prev_bt_status != connected && settings.bluetoothvibe_status) {
      vibes_short_pulse();
    }
  }

  prev_bt_status = connected;
  bitmap_layer_set_bitmap( bluetooth_layer, bluetooth_image );
}

/*
  Handle battery events
*/
void handle_battery( BatteryChargeState charge_state ) {
	static char battery_text[] = "x100";

  if ( battery_image != NULL ) {
    gbitmap_destroy( battery_image );
  }
	
#ifdef PBL_PLATFORM_CHALK
  if ( charge_state.is_charging ) {
    battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_090_100 );
	snprintf(battery_text, sizeof(battery_text), "%d", charge_state.charge_percent);
#else
  if ( charge_state.is_charging ) {
    battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_CHARGING );
	snprintf(battery_text, sizeof(battery_text), "+%d", charge_state.charge_percent);	  
#endif
	  
  } else {
	snprintf(battery_text, sizeof(battery_text), "%d", charge_state.charge_percent);       

    switch ( charge_state.charge_percent ) {
      case 0 ... 10:		
		battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_000_010 );
        break;
      case 11 ... 20:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_010_020 );
        break;
      case 21 ... 30:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_020_030 );
        break;
      case 31 ... 40:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_030_040 );
        break;
      case 41 ... 50:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_040_050 );
	    break;
	  case 51 ... 60:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_050_060 );
        break;
      case 61 ... 70:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_060_070 );
        break;
      case 71 ... 80:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_070_080 );
        break;
      case 81 ... 90:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_080_090 );
        break;
      case 91 ... 100:
        battery_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BATT_090_100 );
        break;
      }
  }
	bitmap_layer_set_bitmap( battery_layer, battery_image );
	charge_percent = charge_state.charge_percent;   
	text_layer_set_text(battery_text_layer, battery_text);
}


/*
  Update status indicators for the battery and bluetooth connection
*/
void update_status( void ) {
  handle_battery( battery_state_service_peek() );
  handle_bluetooth( bluetooth_connection_service_peek() );
}



/*
  Initialization
*/
void handle_init( void ) {
		
  prv_load_settings();
	
  // Listen for AppMessages
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(128, 128);

  // international support
  setlocale(LC_ALL, "");
	
//  memset(&background_layer, 0, sizeof(background_layer));
//  memset(&background_image, 0, sizeof(background_image));
	
//  memset(&battery_layer, 0, sizeof(battery_layer));
//  memset(&battery_image, 0, sizeof(battery_image));
	
  window = window_create();
  window_stack_push( window, true );
  Layer *window_layer = window_get_root_layer( window );

  window_set_background_color(window, GColorBlack);

  // Adjust GRect for Hours and Minutes to compensate for missing AM/PM indicator
  if ( clock_is_24h_style() ) {
    TIME_RECT.origin.y = TIME_RECT.origin.y + 1;
  }
 
  // Load fonts

  font_days = fonts_load_custom_font( resource_get_handle( RESOURCE_ID_FONT_LCARS_20 ) );
  font_date = fonts_load_custom_font( resource_get_handle( RESOURCE_ID_FONT_LCARS_22 ) );
  font_other = fonts_load_custom_font( resource_get_handle( RESOURCE_ID_FONT_LCARS_19 ) );
  small_batt = fonts_load_custom_font( resource_get_handle( RESOURCE_ID_FONT_LCARSB_19 ) );
  small_batt2 = fonts_load_custom_font( resource_get_handle( RESOURCE_ID_FONT_LCARS_18 ) );
  font_time = fonts_load_custom_font( resource_get_handle( RESOURCE_ID_FONT_LCARS_68 ) );
 // font_round_time = fonts_load_custom_font( resource_get_handle( RESOURCE_ID_FONT_LCARS_64 ) );

  // Background image
	
#ifdef PBL_PLATFORM_CHALK
  background_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BACKGROUND1 );
  background_layer = bitmap_layer_create(GRect(0, 0, 180, 180));
  bitmap_layer_set_bitmap( background_layer, background_image );
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(background_layer));
  layer_add_child( window_layer, bitmap_layer_get_layer( background_layer ) );
#else
  background_image = gbitmap_create_with_resource( RESOURCE_ID_IMAGE_BACKGROUND1 );
  background_layer = bitmap_layer_create( layer_get_frame( window_layer ) );
  bitmap_layer_set_bitmap( background_layer, background_image );
  layer_add_child( window_layer, bitmap_layer_get_layer( background_layer ) );
#endif

  // Setup battery layer
  battery_layer = bitmap_layer_create( BATT_RECT );
  layer_add_child( window_layer, bitmap_layer_get_layer( battery_layer ) );

  // Setup bluetooth layer
  bluetooth_layer = bitmap_layer_create( BT_RECT );
  layer_add_child( window_layer, bitmap_layer_get_layer( bluetooth_layer ) );
	
  // Setup time layer
	

  text_time_layer = setup_text_layer( TIME_RECT, GTextAlignmentRight, font_time );		
  layer_add_child( window_layer, text_layer_get_layer( text_time_layer ) );

  // Setup AM/PM layer
	
#ifdef PBL_PLATFORM_CHALK
 if ( !clock_is_24h_style() ) {	 
  block24_img = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLOCK24);
  block24_layer = bitmap_layer_create(GRect(130, 25, 25, 20));
  bitmap_layer_set_bitmap(block24_layer, block24_img);
  layer_add_child(window_layer, bitmap_layer_get_layer(block24_layer));	
	}		
  text_ampm_layer = setup_text_layer( AMPM_RECT, GTextAlignmentLeft, font_other );
  layer_add_child( window_layer, text_layer_get_layer( text_ampm_layer ) );	
#else
 if ( !clock_is_24h_style() ) {
  block24_img = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLOCK24);
  block24_layer = bitmap_layer_create(GRect(125, -1, 25, 20));
  bitmap_layer_set_bitmap(block24_layer, block24_img);
  layer_add_child(window_layer, bitmap_layer_get_layer(block24_layer));	
	}	
  text_ampm_layer = setup_text_layer( AMPM_RECT, GTextAlignmentCenter, font_other );
  text_layer_set_background_color(text_ampm_layer, GColorClear);
  layer_add_child( window_layer, text_layer_get_layer( text_ampm_layer ) );
#endif
	
  // set up battery text layer
	
#ifdef PBL_PLATFORM_CHALK
  battery_text_layer = text_layer_create(GRect(49, 77, 40, 30));	
  text_layer_set_background_color(battery_text_layer, GColorClear);
  text_layer_set_text_color(battery_text_layer, GColorWhite);
  text_layer_set_font(battery_text_layer, small_batt2);
  text_layer_set_text_alignment(battery_text_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(battery_text_layer));		
#else
  battery_text_layer = text_layer_create(GRect(48, 76, 34, 30));	
  text_layer_set_background_color(battery_text_layer, GColorBlack);
  text_layer_set_text_color(battery_text_layer, GColorWhite);
  text_layer_set_font(battery_text_layer, small_batt);
  text_layer_set_text_alignment(battery_text_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(battery_text_layer));	
#endif
	
  // Setup days line layer
  text_days_layer = setup_text_layer( DAYS_RECT
                                    , GTextAlignmentLeft
                                    , font_days );		
	if (settings.startday_status) {text_layer_set_text( text_days_layer, day_lines2[settings.current_language] );
  } else {
       text_layer_set_text( text_days_layer, day_lines[settings.current_language] );
  }
  layer_add_child( window_layer, text_layer_get_layer( text_days_layer ) );	

  effect_layer2 = effect_layer_create(EMPTY_RECT);
  effect_layer_add_effect(effect_layer2, effect_invert, NULL);
  layer_add_child((window_layer), effect_layer_get_layer(effect_layer2));

  // Setup date layer
	
 #ifdef PBL_PLATFORM_CHALK
 text_date_layer = setup_text_layer( ( settings.current_status == true ) ? DATE_RECT : OFF_DATE_RECT
                                    , GTextAlignmentCenter
                                    , small_batt );		
	  text_layer_set_text_color(text_date_layer, GColorBlack);
  text_layer_set_background_color(text_date_layer, GColorClear);

#else
 text_date_layer = setup_text_layer( ( settings.current_status == false ) ? DATE_RECT : OFF_DATE_RECT
                                    , GTextAlignmentLeft
                                    , font_date );		
	
#endif
  layer_add_child( window_layer, text_layer_get_layer( text_date_layer ) );

  // Setup week layer
	
 #ifdef PBL_PLATFORM_CHALK
text_week_layer = setup_text_layer( ( settings.current_status == true ) ? WEEK_RECT : OFF_WEEK_RECT
                                    , GTextAlignmentLeft
                                    , font_date );
#else
 text_week_layer = setup_text_layer( ( settings.current_status == false ) ? WEEK_RECT : OFF_WEEK_RECT
                                    , GTextAlignmentRight
                                    , font_date );
#endif
  layer_add_child( window_layer, text_layer_get_layer( text_week_layer ) );
  

  // Setup seconds name layer
  text_secs_layer = setup_text_layer( SECS_RECT, GTextAlignmentCenter, small_batt );
  text_layer_set_background_color(text_secs_layer, GColorBlack);
  layer_add_child( window_layer, text_layer_get_layer( text_secs_layer ) );

	
#ifdef PBL_HEALTH	
	
// setup health layers
  footprint_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FOOTPRINT);
  GRect footprintbounds = gbitmap_get_bounds(footprint_icon);		
#ifdef PBL_PLATFORM_CHALK
  GRect footprintframe = GRect(57, 159, footprintbounds.size.w, footprintbounds.size.h);
#else
  GRect footprintframe = GRect(122, 135, footprintbounds.size.w, footprintbounds.size.h);
#endif	
  footprint_layer = bitmap_layer_create(footprintframe);
  bitmap_layer_set_bitmap(footprint_layer, footprint_icon);
  layer_add_child(window_layer, bitmap_layer_get_layer(footprint_layer));
	
	
  steps_label = text_layer_create(PBL_IF_ROUND_ELSE(
    GRect(  71, 154, 90,  20 ),
    GRect(  68, 133, 55,  26 )));
  text_layer_set_background_color(steps_label, GColorBlack);
  text_layer_set_text_color(steps_label, GColorWhite  );

#ifdef PBL_PLATFORM_CHALK
  text_layer_set_text_alignment(steps_label, GTextAlignmentLeft);
  text_layer_set_font(steps_label, small_batt2);
#else
   text_layer_set_text_alignment(steps_label, GTextAlignmentRight);
   text_layer_set_font(steps_label, font_date);
#endif
  layer_add_child(window_layer, text_layer_get_layer(steps_label));
  layer_set_hidden(text_layer_get_layer(steps_label), true);
	
#endif

  prv_update_display();

  handle_battery( battery_state_service_peek() );
  handle_bluetooth( bluetooth_connection_service_peek() );

  // Subscribe to services
  tick_timer_service_subscribe( SECOND_UNIT, handle_tick );
  battery_state_service_subscribe(&handle_battery);
  bluetooth_connection_service_subscribe(&handle_bluetooth);
	
  // Force update to avoid a blank screen at startup of the watchface
  update_time();
	
}

/*
  Destroy GBitmap and BitmapLayer
*/
void destroy_graphics( GBitmap *image, BitmapLayer *layer ) {
  layer_remove_from_parent( bitmap_layer_get_layer( layer ) );
  bitmap_layer_destroy( layer );
  if ( image != NULL ) {
    gbitmap_destroy( image );
  }
}

/*
  dealloc
*/
void handle_deinit( void ) {
  // Unsubscribe from services
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();

  // Destroy image objects
  destroy_graphics( background_image, background_layer );
  destroy_graphics( block24_img, block24_layer );
  destroy_graphics( battery_image, battery_layer );
  destroy_graphics( bluetooth_image, bluetooth_layer );
  destroy_graphics( icon_bitmap, icon_layer );

  // Destroy text objects
  text_layer_destroy( text_time_layer );
  text_layer_destroy( text_ampm_layer );
  text_layer_destroy( text_secs_layer );
  text_layer_destroy( text_days_layer );	
  text_layer_destroy( text_date_layer );
  text_layer_destroy( text_week_layer );
  text_layer_destroy( temp_layer );
  text_layer_destroy( battery_text_layer );
	
#ifdef PBL_HEALTH
  health_service_events_unsubscribe();
  text_layer_destroy( steps_label );
  destroy_graphics( footprint_icon, footprint_layer );
#endif
	
  // other layers
 // layer_destroy(weather_holder);
	
  effect_layer_destroy(effect_layer2);
  if (effect_layer != NULL) {
	  effect_layer_destroy(effect_layer);
  }
	
  // Destroy font objects
  fonts_unload_custom_font( font_time );
 // fonts_unload_custom_font( font_round_time );
  fonts_unload_custom_font( font_days );
  fonts_unload_custom_font( font_date );
  fonts_unload_custom_font( font_other );
  fonts_unload_custom_font( small_batt );
  fonts_unload_custom_font( small_batt2 );

 

  // Destroy window
  layer_remove_from_parent(window_layer);
  layer_destroy(window_layer);
	
  window_destroy( window );
}

/*
  Main process
*/
int main( void ) {
  handle_init();
  app_event_loop();
  handle_deinit();
}