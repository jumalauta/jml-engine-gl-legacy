#ifndef EXH_SYSTEM_XML_XML_H_
#define EXH_SYSTEM_XML_XML_H_

typedef struct xml_attribute_t
{
	char *name;
	char *value;
} xml_attribute_t;

typedef struct xml_element_t xml_element_t;

struct xml_element_t
{
	char *name;
	char *value;

	struct xml_element_t *parentElement;
	struct xml_element_t **elements;
	int elementsSize;

	xml_attribute_t **attributes;
	int attributesSize;
};

typedef struct xml_t
{
	int (*attributeCallback)(xml_element_t*, xml_attribute_t*);
	int (*elementCallback)(xml_element_t*);
	unsigned int bufferSize;
	char *buffer;
	void *parserBuffer;
	void *parser;

	xml_element_t *rootElement;
} xml_t;

extern xml_attribute_t* setAttribute(xml_attribute_t *attribute, const char* name, const char* value);
extern void freeAttribute(xml_attribute_t **attribute);
extern void freeAttributeData(xml_attribute_t *attribute);
extern xml_element_t* initElement();
extern xml_element_t* setElement(xml_element_t *element, const char* name);
extern void setElementValue(xml_element_t *element, const char *value);
extern xml_attribute_t* elementAddAttribute(xml_element_t *element, const char* name, const char* value);
extern const char* elementGetAttributeValue(xml_element_t *element, const char* name);
extern char* getElementPath(xml_element_t *element);
extern void freeElement(xml_element_t **element);

extern int xmlInit(xml_t *xml);
extern int xmlDeinit(xml_t *xml);
extern int xmlSetAttributeCallback(xml_t *xml, int (*callback)(xml_element_t*, xml_attribute_t*));
extern int xmlSetElementCallback(xml_t *xml, int (*callback)(xml_element_t*));
extern int xmlSetParseFile(xml_t *xml, const char *file);
extern int xmlParse(xml_t *xml);

#endif /*EXH_SYSTEM_XML_XML_H_*/
