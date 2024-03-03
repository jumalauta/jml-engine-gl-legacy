#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <yxml.h>

#include "system/debug/debug.h"
#include "system/io/io.h"

#include "xml.h"

#define YXML_BUFFER_SIZE 4096

static char *xmlAppendToString(char *content, const char *string);

xml_attribute_t* setAttribute(xml_attribute_t *attribute, const char* name, const char* value)
{
	assert(name);

	if (attribute == NULL)
	{
		attribute = (xml_attribute_t*)malloc(sizeof(xml_attribute_t));
		assert(attribute);
	}

	attribute->name = strdup(name);
	attribute->value = strdup(value);

	return attribute;
}

void freeAttribute(xml_attribute_t **attribute)
{
	assert(*attribute);

	freeAttributeData(*attribute);
	free(*attribute);
	*attribute = NULL;
}

void freeAttributeData(xml_attribute_t *attribute)
{
	assert(attribute);

	free(attribute->name);
	free(attribute->value);
}

xml_element_t* initElement()
{
	xml_element_t *element = (xml_element_t*)malloc(sizeof(xml_element_t));
	assert(element);

	element->name = NULL;
	element->value = NULL;

	element->attributes = NULL;
	element->attributesSize = 0;

	element->elements = NULL;
	element->elementsSize = 0;

	element->parentElement = NULL;

	return element;
}

xml_element_t* setElement(xml_element_t *element, const char* name)
{
	assert(name);

	if (element == NULL)
	{
		element = initElement();
	}

	element->name = strdup(name);

	return element;
}

void setElementValue(xml_element_t *element, const char *value)
{
	assert(element);

	element->value = strdup(value);
}

char* getElementPath(xml_element_t *element)
{
	//<moi><test><jee/></test></moi>
// /test

	char *path = xmlAppendToString(NULL, "/");
	path = xmlAppendToString(path, element->name);
	element = element->parentElement;
	while(element != NULL)
	{
		char *newPath = xmlAppendToString(NULL, "/");
		newPath = xmlAppendToString(newPath, element->name);
		newPath = xmlAppendToString(newPath, path);
		free(path);
		path = newPath;
		element = element->parentElement;
	}

	return path;
}

xml_element_t* elementAddElement(xml_element_t *parentElement, xml_element_t *element)
{
	assert(parentElement);
	assert(element);

	parentElement->elementsSize++;
	parentElement->elements = (xml_element_t**)realloc(parentElement->elements, sizeof(xml_element_t*)*parentElement->elementsSize);
	assert(parentElement->elements);

	parentElement->elements[parentElement->elementsSize-1] = element;
	element->parentElement = parentElement;

	return (xml_element_t*)parentElement->elements[parentElement->elementsSize-1];
}

xml_attribute_t* elementAddAttribute(xml_element_t *element, const char* name, const char* value)
{
	assert(element);
	assert(name);

	element->attributesSize++;
	element->attributes = (xml_attribute_t**)realloc(element->attributes, sizeof(xml_attribute_t*)*element->attributesSize);
	assert(element->attributes);

	element->attributes[element->attributesSize-1] = setAttribute(NULL, name, value);

	return element->attributes[element->attributesSize-1];
}

const char* elementGetAttributeValue(xml_element_t *element, const char* name)
{
	assert(element);
	assert(name);

	int i;
	for(i = 0; i < element->attributesSize; i++)
	{
		if (!strcmp(name, element->attributes[i]->name))
		{
			return (const char*)element->attributes[i]->value;
		}
	}

	return NULL;
}

void freeElement(xml_element_t **element)
{
	assert(*element);

	int i;
	for(i = 0; i < (*element)->attributesSize; i++)
	{
		freeAttribute(&(*element)->attributes[i]);
	}
	free((*element)->attributes);

	for(i = 0; i < (*element)->elementsSize; i++)
	{
		freeElement(&((*element)->elements[i]));
	}
	free((*element)->elements);

	free((*element)->name);
	free((*element)->value);
	free(*element);
	*element = NULL;
}

int xmlInit(xml_t *xml)
{
	assert(xml);

	xml->parser = malloc(sizeof(yxml_t));
	assert(xml->parser);

	xml->buffer = NULL;
	xml->bufferSize = 0;

	xml->parserBuffer = malloc(YXML_BUFFER_SIZE);
	assert(xml->parserBuffer);

	xml->elementCallback = NULL;
	xml->attributeCallback = NULL;

	xml->rootElement = NULL;

	yxml_init(xml->parser, xml->parserBuffer, YXML_BUFFER_SIZE);

	return 1;
}

static int xmlFree(xml_t *xml)
{
	if (xml->buffer)
	{
		free(xml->buffer);
		xml->buffer = NULL;
	}

	if (xml->parserBuffer)
	{
		free(xml->parserBuffer);
		xml->parserBuffer = NULL;
	}

	if (xml->parser)
	{
		free(xml->parser);
		xml->parser = NULL;
	}

	return 1;
}

int xmlDeinit(xml_t *xml)
{
	assert(xml);

	xmlFree(xml);

	if (xml->rootElement)
	{
		freeElement(&xml->rootElement);
	}


	return 1;
}

int xmlSetAttributeCallback(xml_t *xml, int (*callback)(xml_element_t*, xml_attribute_t*))
{
	assert(xml);
	xml->attributeCallback = callback;

	return 1;
}

int xmlSetElementCallback(xml_t *xml, int (*callback)(xml_element_t*))
{
	assert(xml);
	xml->elementCallback = callback;

	return 1;
}

int xmlSetParseFile(xml_t *xml, const char *file)
{
	assert(xml);
	assert(xml->buffer == NULL);
	assert(xml->bufferSize == 0);


	xml->buffer = ioReadFileToBuffer(file, &xml->bufferSize);
	if (xml->bufferSize == 0)
	{
		debugErrorPrintf("XML parser could not read file '%s'", file);
		return 0;
	}

	return 1;
}

static char *xmlInitializeString(char *content)
{
	if (content)
	{
		free(content);
	}

	return NULL;
}

static char *xmlAppendToString(char *content, const char *string)
{
	char *newContent = content;
	size_t sourceLength = strlen(string);
	size_t newLength = sourceLength + 1;
	if (newContent == NULL)
	{
		newContent = (char*)malloc(sizeof(char) * newLength);
		newContent[0] = '\0';
	}
	else
	{
		size_t contentLength = strlen((const char*)content);
		newLength += contentLength;

		if (newLength > contentLength+1)
		{
			newContent = (char*)realloc(content, sizeof(char) * newLength);
		}
	}	

	strncat(newContent, string, sourceLength);

	return newContent;
}

int xmlParse(xml_t *xml)
{
	assert(xml);

	if (xml->bufferSize == 0 || xml->buffer == NULL)
	{
		debugErrorPrintf("XML data not loaded!");
		return 0;
	}

	char *path = NULL;

	char *elementContent = NULL;
	char *attributeContent = NULL;

	xml_element_t *element = NULL;
	xml_element_t *parentElement = NULL;

	char *bufferIterator = NULL;
	for(bufferIterator = xml->buffer; *bufferIterator; bufferIterator++)
	{
		yxml_ret_t returnValue = yxml_parse(xml->parser, *bufferIterator);
		if (returnValue < 0)
		{
			debugErrorPrintf("XML parsing error(%d): line: %d, byte: %u, offset: %u", returnValue, ((yxml_t*)xml->parser)->line, ((yxml_t*)xml->parser)->byte, ((yxml_t*)xml->parser)->total);
			break;
		}

		char first = 0;
		char *elementName = NULL;
		char *attributeName = NULL;

		switch (returnValue)
		{
			case YXML_ELEMSTART:
				elementName = ((yxml_t*)xml->parser)->elem;
				path = xmlAppendToString(path, "/");
				path = xmlAppendToString(path, elementName);

				parentElement = element;
				element = setElement(NULL, (const char*)elementName);
				if (parentElement)
				{
					elementAddElement(parentElement, element);
				}

				if (xml->rootElement == NULL)
				{
					xml->rootElement = element;
				}

				break;

			case YXML_CONTENT:
				//strip white spaces from the begin of element during parsing
				first = ((yxml_t*)xml->parser)->data[0];
				if (elementContent == NULL && (first == ' ' || first == '\t' || first == '\n'))
				{
					break;
				}

				elementContent = xmlAppendToString(elementContent, ((yxml_t*)xml->parser)->data);
				break;

			case YXML_ELEMEND:
				if (element == NULL)
				{
					elementName = ((yxml_t*)xml->parser)->elem;
					element = setElement(element, (const char*)elementName);
				}

				setElementValue(element, (const char*)elementContent);
				if (xml->elementCallback)
				{
					xml->elementCallback(element);
				}

				element = element->parentElement;
				elementContent = xmlInitializeString(elementContent);
				path[strlen(path) - strlen(strrchr(path, '/'))] = '\0';
				break;

			case YXML_ATTRVAL:
				attributeContent = xmlAppendToString(attributeContent, ((yxml_t*)xml->parser)->data);
				break;

			case YXML_ATTREND:
				attributeName = ((yxml_t*)xml->parser)->attr;
				xml_attribute_t *attribute = elementAddAttribute(element, (const char*)attributeName, (const char*)attributeContent);

				if (xml->attributeCallback)
				{
					xml->attributeCallback(element, attribute);
				}

				attributeContent = xmlInitializeString(attributeContent);
				break;

			default:
				//out-of-scope
				break;
		}

	}

	if (path)
	{
		free(path);
	}

	if (attributeContent)
	{
		free(elementContent);
	}

	if (elementContent)
	{
		free(elementContent);
	}

	yxml_ret_t returnValue = yxml_eof(xml->parser);
	if(returnValue < 0)
	{
		debugErrorPrintf("XML parsing error(%d) in deinitialization", returnValue);
		return 0;
	}

	xmlFree(xml);

	return 1;
}
