module.exports = [
  {
    "type": "heading",
    "defaultValue": "TrekV3 (no weather) Config"
  },
     {
        "type": "text",
        "defaultValue": "<h6>A white button = OFF, an orange button = ON</h6>",
      },
	{
    "type": "section",
    "items": [
 	{
        "type": "heading",
        "defaultValue": "<h5>Display</h5>"
	},
     {
		"type": "toggle",
        "messageKey": "startday_status",
		"label": "Week start day: Mon/Sun",
        "defaultValue": false
      },
      {
		"type": "toggle",
        "messageKey": "steps_status",
        "label": "Show steps",
        "defaultValue": false
      },
	  {
        "type": "toggle",
        "messageKey": "ampmsecs",
        "label": " AM/PM or Seconds",
        "defaultValue": false
      },
	  {
        "type": "toggle",
        "messageKey": "invert",
        "label": "Invert",
        "defaultValue": false
      },
	  {
        "type": "toggle",
        "messageKey": "textcol_format",
        "label": "White or Orange time text",
        "defaultValue": false
      },
		
		{
  "type": "select",
  "messageKey": "current_background",
  "defaultValue": 0,
  "label": "BG Colour Scheme",
  "options": [
    { 
      "label": "Default Blue or B&W",
      "value": 0
    },
    { 
      "label": "Light Blue",
      "value": 1
    },
	  { 
      "label": "Purple",
      "value": 2
    },
	{ 
      "label": "Yellow",
      "value": 3
    },
	{ 
      "label": "White",
      "value": 4
    },
	{ 
      "label": "Grey",
      "value": 5
    },
    { 
      "label": "Red",
      "value": 6
    },
	{ 
      "label": "Yellows/Blue",
      "value": 7
    },
	{ 
      "label": "Blues/Red/Orange",
      "value": 8
    },
	{ 
      "label": "Dark Green",
      "value": 9
    },
	{ 
      "label": "Dark Blue",
      "value": 10
    } 
  ]
},
		{
  "type": "select",
  "messageKey": "current_language",
  "defaultValue": 0,
  "label": "Language",
  "options": [
    { 
      "label": "English",
      "value": 0
    },
    { 
      "label": "Dutch",
      "value": 1
    },
	  { 
      "label": "German",
      "value": 2
    },
	{ 
      "label": "French",
      "value": 3
    },
	{ 
      "label": "Croation",
      "value": 4
    },
	{ 
      "label": "Spanish",
      "value": 5
    },
    { 
      "label": "Italian",
      "value": 6
    },
	{ 
      "label": "Norweigan",
      "value": 7
    },
	{ 
      "label": "Swedish",
      "value": 8
    },
	{ 
      "label": "Finnish",
      "value": 9
    },
	{ 
      "label": "Danish",
      "value": 10
    },
	{ 
      "label": "Turkish",
      "value": 11
    },
	{ 
      "label": "Catalan",
      "value": 12
    },
	{ 
      "label": "Slovak",
      "value": 13
    },
	{ 
      "label": "Portugese",
      "value": 14
    },
	{ 
      "label": "Hungarian",
      "value": 15
    }
  ]
},
	  
	  {
  "type": "select",
  "messageKey": "format",
  "defaultValue": 0,
  "label": "Date Format",
  "options": [
    { 
      "label": "Week",
      "value": 0
    },
    { 
		"label": "Day of the year (stardate)",
      "value": 1
    },
	  { 
      "label": "DD/MM/YY",
      "value": 2
    },
	{ 
      "label": "MM/DD/YY",
      "value": 3
    },
	{ 
      "label": "Wxxx Dxxx",
      "value": 4
    },
	{ 
      "label": "YYYY MM DD",
      "value": 5
    },
    { 
      "label": "DD.MM.YYYY",
      "value": 6
    },
	{ 
      "label": "YY.WW.DDD",
      "value": 7
    }
	  ]
	  },

	{
    "type": "section",
    "items": [
 	{
        "type": "heading",
        "defaultValue": "<h5>Vibration</h5>"
	},	
     {
		"type": "toggle",
        "messageKey": "bluetoothvibe_status",
        "label": "Bluetooth vibration",
        "defaultValue": false
	  },
   ]
},
]
},
		
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  },
		  {
        "type": "text",
        "defaultValue": "<h6>If you find this watchface useful, please consider making a <a href='https://www.paypal.me/markchopsreed'>small donation</a>. Thankyou.</h6>",
  },
];