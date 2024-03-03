#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "system/xml/xml.h"
#include "system/io/io.h"

#include "svg.h"

int attributeCallback(xml_element_t *element, xml_attribute_t *attribute)
{
	char *path = getElementPath(element);
	printf("%s/@%s = '%s'\n", path, attribute->name, attribute->value);
	free(path);
	return 1;
}
int elementCallback(xml_element_t *element)
{
	char *path = getElementPath(element);
	printf("%s (%s) = '%s'\n", path, element->name, element->value);

	int i;
	for(i = 0; i < element->attributesSize; i++)
	{
		xml_attribute_t *attribute = element->attributes[i];
		printf("\t%s/@%s = '%s'\n", path, attribute->name, attribute->value);
	}

	free(path);

	return 1;
}

void printXmlElement(xml_element_t *element)
{
	char *path = getElementPath(element);
	printf("%s (%s) = '%s'\n", path, element->name, element->value);

	int i;
	for(i = 0; i < element->attributesSize; i++)
	{
		xml_attribute_t *attribute = element->attributes[i];
		printf("\t%s/@%s = '%s'\n", path, attribute->name, attribute->value);
	}

	free(path);	

	for(i = 0; i < element->elementsSize; i++)
	{
		printXmlElement(element->elements[i]);
	}
}

#define SVG_COLOR_KEYWORDS_ALL
#ifdef SVG_COLOR_KEYWORDS_ALL
#	define SVG_COLOR_KEYWORDS_SIZE 147
#else //basic color keywords
#	define SVG_COLOR_KEYWORDS_SIZE 15
#endif //SVG_COLOR_KEYWORDS_ALL

typedef struct svg_color {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} svg_color;

typedef struct svg_color_keyword {
	const char *name;
	svg_color color;
} svg_color_keyword;

const struct
{
	size_t size;
	svg_color_keyword colors[];
} svg_color_keywords = {
	SVG_COLOR_KEYWORDS_SIZE,
	{
		//BASIC COLOR KEYWORDS
		 {"black", { 0, 0, 0} }
		,{"blue", { 0, 0, 255} }
		,{"brown", {165, 42, 42} }
		,{"cyan", { 0, 255, 255} }
		,{"gray", {128, 128, 128} }
		,{"grey", {128, 128, 128} }
		,{"green", { 0, 128, 0} }
		,{"magenta", {255, 0, 255} }
		,{"orange", {255, 165, 0} }
		,{"pink", {255, 192, 203} }
		,{"purple", {128, 0, 128} }
		,{"red", {255, 0, 0} }
		,{"violet", {238, 130, 238} }
		,{"white", {255, 255, 255} }
		,{"yellow", {255, 255, 0} }
#ifdef SVG_COLOR_KEYWORDS_ALL
		,{"aliceblue", {240, 248, 255} }
		,{"antiquewhite", {250, 235, 215} }
		,{"aqua", { 0, 255, 255} }
		,{"aquamarine", {127, 255, 212} }
		,{"azure", {240, 255, 255} }
		,{"beige", {245, 245, 220} }
		,{"bisque", {255, 228, 196} }
		,{"blanchedalmond", {255, 235, 205} }
		,{"blueviolet", {138, 43, 226} }
		,{"burlywood", {222, 184, 135} }
		,{"cadetblue", { 95, 158, 160} }
		,{"chartreuse", {127, 255, 0} }
		,{"chocolate", {210, 105, 30} }
		,{"coral", {255, 127, 80} }
		,{"cornflowerblue", {100, 149, 237} }
		,{"cornsilk", {255, 248, 220} }
		,{"crimson", {220, 20, 60} }
		,{"darkblue", { 0, 0, 139} }
		,{"darkcyan", { 0, 139, 139} }
		,{"darkgoldenrod", {184, 134, 11} }
		,{"darkgray", {169, 169, 169} }
		,{"darkgreen", { 0, 100, 0} }
		,{"darkgrey", {169, 169, 169} }
		,{"darkkhaki", {189, 183, 107} }
		,{"darkmagenta", {139, 0, 139} }
		,{"darkolivegreen", { 85, 107, 47} }
		,{"darkorange", {255, 140, 0} }
		,{"darkorchid", {153, 50, 204} }
		,{"darkred", {139, 0, 0} }
		,{"darksalmon", {233, 150, 122} }
		,{"darkseagreen", {143, 188, 143} }
		,{"darkslateblue", { 72, 61, 139} }
		,{"darkslategray", { 47, 79, 79} }
		,{"darkslategrey", { 47, 79, 79} }
		,{"darkturquoise", { 0, 206, 209} }
		,{"darkviolet", {148, 0, 211} }
		,{"deeppink", {255, 20, 147} }
		,{"deepskyblue", { 0, 191, 255} }
		,{"dimgray", {105, 105, 105} }
		,{"dimgrey", {105, 105, 105} }
		,{"dodgerblue", { 30, 144, 255} }
		,{"firebrick", {178, 34, 34} }
		,{"floralwhite", {255, 250, 240} }
		,{"forestgreen", { 34, 139, 34} }
		,{"fuchsia", {255, 0, 255} }
		,{"gainsboro", {220, 220, 220} }
		,{"ghostwhite", {248, 248, 255} }
		,{"gold", {255, 215, 0} }
		,{"goldenrod", {218, 165, 32} }
		,{"greenyellow", {173, 255, 47} }
		,{"honeydew", {240, 255, 240} }
		,{"hotpink", {255, 105, 180} }
		,{"indianred", {205, 92, 92} }
		,{"indigo", { 75, 0, 130} }
		,{"ivory", {255, 255, 240} }
		,{"khaki", {240, 230, 140} }
		,{"lavender", {230, 230, 250} }
		,{"lavenderblush", {255, 240, 245} }
		,{"lawngreen", {124, 252, 0} }
		,{"lemonchiffon", {255, 250, 205} }
		,{"lightblue", {173, 216, 230} }
		,{"lightcoral", {240, 128, 128} }
		,{"lightcyan", {224, 255, 255} }
		,{"lightgoldenrodyellow", {250, 250, 210} }
		,{"lightgray", {211, 211, 211} }
		,{"lightgreen", {144, 238, 144} }
		,{"lightgrey", {211, 211, 211} }
		,{"lightpink", {255, 182, 193} }
		,{"lightsalmon", {255, 160, 122} }
		,{"lightseagreen", { 32, 178, 170} }
		,{"lightskyblue", {135, 206, 250} }
		,{"lightslategray", {119, 136, 153} }
		,{"lightslategrey", {119, 136, 153} }
		,{"lightsteelblue", {176, 196, 222} }
		,{"lightyellow", {255, 255, 224} }
		,{"lime", { 0, 255, 0} }
		,{"limegreen", { 50, 205, 50} }
		,{"linen", {250, 240, 230} }
		,{"maroon", {128, 0, 0} }
		,{"mediumaquamarine", {102, 205, 170} }
		,{"mediumblue", { 0, 0, 205} }
		,{"mediumorchid", {186, 85, 211} }
		,{"mediumpurple", {147, 112, 219} }
		,{"mediumseagreen", { 60, 179, 113} }
		,{"mediumslateblue", {123, 104, 238} }
		,{"mediumspringgreen", { 0, 250, 154} }
		,{"mediumturquoise", { 72, 209, 204} }
		,{"mediumvioletred", {199, 21, 133} }
		,{"midnightblue", { 25, 25, 112} }
		,{"mintcream", {245, 255, 250} }
		,{"mistyrose", {255, 228, 225} }
		,{"moccasin", {255, 228, 181} }
		,{"navajowhite", {255, 222, 173} }
		,{"navy", { 0, 0, 128} }
		,{"oldlace", {253, 245, 230} }
		,{"olive", {128, 128, 0} }
		,{"olivedrab", {107, 142, 35} }
		,{"orangered", {255, 69, 0} }
		,{"orchid", {218, 112, 214} }
		,{"palegoldenrod", {238, 232, 170} }
		,{"palegreen", {152, 251, 152} }
		,{"paleturquoise", {175, 238, 238} }
		,{"palevioletred", {219, 112, 147} }
		,{"papayawhip", {255, 239, 213} }
		,{"peachpuff", {255, 218, 185} }
		,{"peru", {205, 133, 63} }
		,{"plum", {221, 160, 221} }
		,{"powderblue", {176, 224, 230} }
		,{"rosybrown", {188, 143, 143} }
		,{"royalblue", { 65, 105, 225} }
		,{"saddlebrown", {139, 69, 19} }
		,{"salmon", {250, 128, 114} }
		,{"sandybrown", {244, 164, 96} }
		,{"seagreen", { 46, 139, 87} }
		,{"seashell", {255, 245, 238} }
		,{"sienna", {160, 82, 45} }
		,{"silver", {192, 192, 192} }
		,{"skyblue", {135, 206, 235} }
		,{"slateblue", {106, 90, 205} }
		,{"slategray", {112, 128, 144} }
		,{"slategrey", {112, 128, 144} }
		,{"snow", {255, 250, 250} }
		,{"springgreen", { 0, 255, 127} }
		,{"steelblue", { 70, 130, 180} }
		,{"tan", {210, 180, 140} }
		,{"teal", { 0, 128, 128} }
		,{"thistle", {216, 191, 216} }
		,{"tomato", {255, 99, 71} }
		,{"turquoise", { 64, 224, 208} }
		,{"wheat", {245, 222, 179} }
		,{"whitesmoke", {245, 245, 245} }
		,{"yellowgreen", {154, 205, 50} }
#endif //SVG_COLOR_KEYWORDS_ALL
	}
};

char* stripWhitespaces(const char *src)
{
	if (src == NULL)
	{
		return NULL;
	}

	char *dst_tmp = strdup(src);
	assert(dst_tmp);

	size_t src_i;
	size_t dst_tmp_i;
	size_t src_length = strlen(src);
	for(src_i = 0, dst_tmp_i = 0; src_i < src_length; src_i++)
	{
		char current_character = src[src_i];
		if (!isspace((int)current_character))
		{
			dst_tmp[dst_tmp_i++] = current_character;
		}
	}
	dst_tmp[dst_tmp_i++] = '\0';

	char *dst = (char*)realloc(dst_tmp, dst_tmp_i);
	assert(dst);

	return dst;
}
//Color data type: https://www.w3.org/TR/SVG/types.html#DataTypeColor
//Color keywords: https://www.w3.org/TR/SVG/types.html#ColorKeywords
void parseColor(const char *colorValueUnstripped)
{
	/*
		color    ::= "#" hexdigit hexdigit hexdigit (hexdigit hexdigit hexdigit)?
		             | "rgb(" wsp* integer comma integer comma integer wsp* ")"
		             | "rgb(" wsp* integer "%" comma integer "%" comma integer "%" wsp* ")"
		             | color-keyword
		hexdigit ::= [0-9A-Fa-f]
		comma    ::= wsp* "," wsp*
	*/

	char *colorValue = stripWhitespaces(colorValueUnstripped);

	printf("input:'%s' (original:'%s')\n", colorValue, colorValueUnstripped);

	unsigned int r=0,g=0,b=0;
	if (colorValue[0] == '#')
	{
		unsigned int hashColorLength = strlen(colorValue);
		switch(hashColorLength)
		{
			case 7: //#RRGGBB
				sscanf(colorValue, "#%2x%2x%2x", &r, &g, &b);
				break;

			case 4: //#RGB
				sscanf(colorValue, "#%1x%1x%1x", &r, &g, &b);
				//For example, #fb0 expands to #ffbb00
				//HEX rgb short notation is converted like this: 0xF => 0xFF
				r = r*0x10+r;
				g = g*0x10+g;
				b = b*0x10+b;
				break;

			default: //not supported / invalid format
				printf("ERROR: Invalid color format! value:'%s'\n", colorValue);
				break;
		}
	}
	//An RGB start-function is the case-insensitive string "rgb(", {for example "RGB(" or "rGb(". For compatibility, the all-lowercase form "rgb(" is preferred.
	else if (strncasecmp(colorValue, "rgb(", 4) == 0)
	{
		//TODO: Handle whitespaces between commas
		if (strchr(colorValue, '%') == NULL)
		{
			//parse 0 - 255 integer RGB values
			sscanf(colorValue + 3, "(%d,%d,%d)", &r, &g, &b);

		}
		else
		{
			//parse 0% - 100% integer percent RGB values
			sscanf(colorValue + 3, "(%d%%,%d%%,%d%%)", &r, &g, &b);
			r = r/100.0 * 0xFF;
			g = g/100.0 * 0xFF;
			b = b/100.0 * 0xFF;
		}

	}
	else //assume that there are defined color keywords in use
	{
		int match = 0;
		size_t kwyword_i;
		for (kwyword_i = 0; kwyword_i < svg_color_keywords.size; kwyword_i++)
		{
			const svg_color_keyword *color_keyword = &svg_color_keywords.colors[kwyword_i];
			if (!strcasecmp(color_keyword->name, colorValue))
			{
				match = 1;
				r = color_keyword->color.r;
				g = color_keyword->color.g;
				b = color_keyword->color.b;
				break;
			}
		}

		if (match == 0)
		{
			printf("ERROR: Invalid color format! value:'%s'\n", colorValue);
		}
	}

	printf("output red: %X (%d), green:%X (%d), blue:%X (%d)\n", r,r,g,g,b,b);
	free(colorValue);
}

void parseD(const char *d)
{
	//TODO: Assuming currently that everything is tokenized with single space character.
	//Input should be sanitized to this format as standard allows other forms as well.
	//Required conversion examples: "M10,10M10,10z" => "M 10,10 M 10,10 z"
	char *d2 = strdup(d);

	const char* D_DELIMETER = " ";
	char *token;
	while((token = strtok_reentrant(d2, D_DELIMETER, &d2)))
	{
		if (strlen(token) == 1)
		{
			//controller point
			printf("Instruction: '%s'\n", token);
		}
		else
		{
			double x=0.0;
			double y=0.0;
			sscanf(token, "%lf,%lf", &x, &y);
			printf("\tx:%.2f, y:%.2f\n", x, y);
		}
	}

	free(d2);
}

//SVG Reference: https://developer.mozilla.org/en-US/docs/Web/SVG
void parseSvg(xml_element_t *element)
{
	char *path = getElementPath(element);
	int i;

	if (!strcmp(element->name, "svg"))
	{
		const char *viewBox = elementGetAttributeValue(element, "viewBox");
		double min_x = 0.0;
		double min_y = 0.0;
		double width = 0.0;
		double height = 0.0;

		sscanf(viewBox, "%lf %lf %lf %lf", &min_x, &min_y, &width, &height);
		printf("viewBox: minX:%.2f, minY:%.2f, width:%.2f, height:%.2f\n", min_x, min_y, width, height);
	}
	else if (!strcmp(element->name, "g"))
	{
		printf("layer: '%s'\n", elementGetAttributeValue(element, "id"));
	}
	else if (!strcmp(element->name, "path"))
	{
		printf("%s (%s) = '%s'\n", path, element->name, element->value);
		const char *d = elementGetAttributeValue(element, "d");
		printf("path d: '%s'\n", d);
		parseD(d);

		char *style = strdup(elementGetAttributeValue(element, "style"));
		printf("path style: '%s'\n", style);
		const char *STYLE_DELIMETER = ";";
		const char *STYLE_VALUE_DELIMETER = ":";

		char *variable;
		char *p = style;

		while((variable = strtok_reentrant(p, STYLE_DELIMETER, &p)))
		{
			char *variable2 = strdup(variable);
			char *name = strtok_reentrant(variable2, STYLE_VALUE_DELIMETER, &variable2);
			char *value = strtok_reentrant(variable2, STYLE_VALUE_DELIMETER, &variable2);

			printf("'%s'='%s'\n", name, value);
			if (!strcmp("stroke", name))
			{
				parseColor(value);
			}
		}

		free(style);
	}

	free(path);	

	for(i = 0; i < element->elementsSize; i++)
	{
		parseSvg(element->elements[i]);
	}	
}

int testSvg()
{
	xml_t xml;
	if (xmlInit(&xml))
	{
		xmlSetParseFile(&xml, "data/drawing2.svg");
		//xmlSetAttributeCallback(&xml, attributeCallback);
		//xmlSetElementCallback(&xml, elementCallback);
		xmlParse(&xml);
		//printXmlElement(xml.rootElement);
		parseSvg(xml.rootElement);
		xmlDeinit(&xml);

		/*parseColor("#1bc");
		parseColor("#dEAdFf");
		parseColor("#1"); //should fail
		parseColor("rgb(100,123,255)");
		parseColor("rGb(   100,123  , 255)");
		parseColor("rgb(50%, 10%,100%  )");
		parseColor("aliceblue");
		parseColor("alicebluE");
		parseColor("aliceblues"); //should fail*/
 
		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}
