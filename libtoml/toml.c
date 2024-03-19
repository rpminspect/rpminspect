#include "toml.h"
#include "toml_private.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <string.h>

int
toml_init(struct toml_node **toml_root)
{
	struct toml_node *toml_node;

	toml_node = malloc(sizeof(*toml_node));
	if (!toml_node) {
		return -1;
	}

	toml_node->type = TOML_ROOT;
	toml_node->name = NULL;
	list_head_init(&toml_node->value.map);

	*toml_root = toml_node;
	return 0;
}

struct toml_node *
toml_get(struct toml_node *toml_root, char *key)
{
	char *ancestor, *tofree, *name;
	struct toml_node *node = toml_root;

	tofree = name = strdup(key);

	while ((ancestor = strsep(&name, "."))) {
		struct toml_table_item *item = NULL;
		int found = 0;

		list_for_each(&node->value.map, item, map) {
			if (!item->node.name)
				continue;
							
			if (strcmp(item->node.name, ancestor) == 0) {
				node = &item->node;
				found = 1;
				break;
			}
		}

		if (!found) {
			node = NULL;
			break;
		}
	}

	free(tofree);

	return node;
}

static void
_toml_dump(struct toml_node *toml_node, FILE *output, char *bname, int indent,
																	int newline)
{
	int		i;
	char*	value;

	for (i = 0; i < indent - 1; i++)
		fprintf(output, "\t");

	switch (toml_node->type) {
	case TOML_ROOT: {
		struct toml_table_item *item = NULL;

		list_for_each(&toml_node->value.map, item, map) {
			_toml_dump(&item->node, output, toml_node->name, indent, 1);
		}
		break;
	}

	case TOML_INLINE_TABLE:
	case TOML_TABLE: {
		struct toml_table_item *item = NULL;
		char name[100];

		if (toml_node->name) {
			sprintf(name, "%s%s%s", bname ? bname : "", bname ? "." : "",
															toml_node->name);
			fprintf(output, "%s[%s]\n", indent ? "\t": "", name);
		}
		list_for_each(&toml_node->value.map, item, map)
			_toml_dump(&item->node, output, name, indent+1, 1);
		fprintf(output, "\n");
		break;
	}

	case TOML_LIST: {
		struct toml_list_item *item = NULL;
		struct toml_list_item *tail =
				list_tail(&toml_node->value.list, struct toml_list_item, list);

		if (toml_node->name)
			fprintf(output, "%s = ", toml_node->name);
		fprintf(output, "[ ");
		list_for_each(&toml_node->value.list, item, list) {
			_toml_dump(&item->node, output, toml_node->name, 0, 0);
			if (item != tail)
				fprintf(output, ", ");
		}
		fprintf(output, " ]%s", newline ? "\n" : "");

		break;
	}

	case TOML_INT:
	case TOML_FLOAT:
	case TOML_STRING:
	case TOML_DATE:
	case TOML_BOOLEAN:
		if (toml_node->name)
			fprintf(output, "%s = ", toml_node->name);
		value = toml_value_as_string(toml_node);
		fprintf(output, "%s%s%s%s",
				toml_node->type == TOML_STRING ? "\"" : "",
				value, 
				toml_node->type == TOML_STRING ? "\"" : "",
				newline ? "\n" : "");
		free(value);
		break;

	case TOML_TABLE_ARRAY: {
		struct toml_list_item *item = NULL;

		list_for_each(&toml_node->value.list, item, list) {
			fprintf(output, "[[%s]]\n", toml_node->name);
			_toml_dump(&item->node, output, toml_node->name, indent, 1);
		}

		break;
	}

	default:
		fprintf(stderr, "unknown toml type %d\n", toml_node->type);
		/* assert(toml_node->type); */
	}
}

enum order {
	kOrderWalk = 1,
	kOrderDive
};

static void
_toml_process(struct toml_node *node, toml_node_walker fn, enum order order, void *ctx)
{
	if (order == kOrderWalk)
		fn(node, ctx);

	switch (node->type) {
	case TOML_ROOT:
	case TOML_INLINE_TABLE:
	case TOML_TABLE: {
		struct toml_table_item *item = NULL, *next = NULL;

		list_for_each_safe(&node->value.map, item, next, map)
			_toml_process(&item->node, fn, order, ctx);
		break;
	}

	case TOML_TABLE_ARRAY:
	case TOML_LIST: {
		struct toml_list_item *item = NULL, *next = NULL;

		list_for_each_safe(&node->value.list, item, next, list) {
			_toml_process(&item->node, fn, order, ctx);
		}
		break;
	}

	default:
		break;
	}

	if (order == kOrderDive)
		fn(node, ctx);
}

void
toml_walk(struct toml_node *root, toml_node_walker fn, void *ctx)
{
	_toml_process(root, fn, kOrderWalk, ctx);
}

void
toml_dive(struct toml_node *root, toml_node_walker fn, void *ctx)
{
	_toml_process(root, fn, kOrderDive, ctx);
}

void
toml_dump(struct toml_node *toml_root, FILE *output)
{
	_toml_dump(toml_root, output, NULL, 0, 1);
}

static char*
_json_string_encode(const char* string)
{
	char*	ret;
	int		j = 0;
	int		i = 0;

	for (i = 0; string[i]; i++)
	{
		switch (string[i]) {
		case '"':
		case '\\':
		case '/':
		case '\b':
		case '\f':
		case '\n':
		case '\r':
		case '\t':
			j++;

		default:
			break;
		}
	}

	ret = malloc(i + j + 1);
	for (i = 0, j = 0; string[i]; i++)
	{
		switch (string[i]) {
		case '"':
			ret[i+j++] = '\\';
			ret[i+j] = '"';
			break;

		case '\\':
			ret[i+j++] = '\\';
			ret[i+j] = '\\';
			break;

		case '/':
			ret[i+j++] = '\\';
			ret[i+j] = '/';
			break;

		case '\b':
			ret[i+j++] = '\\';
			ret[i+j] = 'b';
			break;

		case '\f':
			ret[i+j++] = '\\';
			ret[i+j] = 'f';
			break;

		case '\n':
			ret[i+j++] = '\\';
			ret[i+j] = 'n';
			break;

		case '\r':
			ret[i+j++] = '\\';
			ret[i+j] = 'r';
			break;

		case '\t':
			ret[i+j++] = '\\';
			ret[i+j] = 't';
			break;

		default:
			ret[i+j] = string[i];
			break;
		}
	}

	ret[i+j] = 0;

	return ret;
}

static void
_output_name(struct toml_node* node, FILE* output)
{
	char* name;

	if (!node->name)
		return;

	name = _json_string_encode(node->name);
	fprintf(output, "\"%s\": ", name);
	free(name);
}

static void
_toml_tojson(struct toml_node *toml_node, FILE *output, int indent)
{
	int		i;
	char*	value;

	char*	toml_json_types[TOML_MAX];

	toml_json_types[TOML_INT]		= "integer";
	toml_json_types[TOML_FLOAT]		= "float";
	toml_json_types[TOML_STRING]	= "string";
	toml_json_types[TOML_DATE]		= "datetime";
	toml_json_types[TOML_BOOLEAN]	= "bool";

	for (i = 0; i < indent - 1; i++)
		fprintf(output, "\t");

	switch (toml_node->type) {
	case TOML_ROOT: {
		struct toml_table_item *item = NULL;
		struct toml_table_item *tail =
			list_tail(&toml_node->value.map, struct toml_table_item, map);

		list_for_each(&toml_node->value.map, item, map) {
			_toml_tojson(&item->node, output, indent+1);
			fprintf(output, "%s\n", item != tail ? "," : "");
		}
		break;
	}

	case TOML_INLINE_TABLE:
	case TOML_TABLE: {
		struct toml_table_item *item = NULL;
		struct toml_table_item *tail =
			list_tail(&toml_node->value.map, struct toml_table_item, map);

		_output_name(toml_node, output);

		fprintf(output, "{\n");

		list_for_each(&toml_node->value.map, item, map) {
			_toml_tojson(&item->node, output, indent+1);
			fprintf(output, "%s\n", item != tail ? "," : "");
		}

		for (i = 0; i < indent - 1; i++)
			fprintf(output, "\t");
		fprintf(output, "}");
		break;
	}

	case TOML_LIST: {
		struct toml_list_item *item = NULL;
		struct toml_list_item *tail =
			list_tail(&toml_node->value.map, struct toml_list_item, list);

		_output_name(toml_node, output);
		fprintf(output, "{ \"type\": \"array\", \"value\": [\n");

		list_for_each(&toml_node->value.list, item, list) {
			_toml_tojson(&item->node, output, indent+1);
			fprintf(output, "%s\n", item != tail ? "," : "");
		}

		for (i = 0; i < indent - 1; i++)
			fprintf(output, "\t");
		fprintf(output, " ] }");
		break;
	}

	case TOML_INT:
	case TOML_FLOAT:
	case TOML_STRING:
	case TOML_DATE:
	case TOML_BOOLEAN:
		value = toml_value_as_string(toml_node);
		_output_name(toml_node, output);
		fprintf(output, "{ \"type\": \"%s\", \"value\": \"%s\" }",
									toml_json_types[toml_node->type], value);
		free(value);
		break;

	case TOML_TABLE_ARRAY: {
		struct toml_list_item *item = NULL;
		struct toml_list_item *tail =
			list_tail(&toml_node->value.list, struct toml_list_item, list);

		_output_name(toml_node, output);
		fprintf(output, "[\n");

		list_for_each(&toml_node->value.list, item, list) {
			_toml_tojson(&item->node, output, indent+1);
			fprintf(output, "%s\n", item != tail ? "," : "");
		}

		for (i = 0; i < indent - 1; i++)
			fprintf(output, "\t");
		fprintf(output, "]");
		break;
	}

	default:
		fprintf(stderr, "unknown toml type %d\n", toml_node->type);
		/* assert(toml_node->type); */
	}
}

void
toml_tojson(struct toml_node *toml_root, FILE *output)
{
	fprintf(output, "{\n");
	_toml_tojson(toml_root, output, 1);
	fprintf(output, "}\n");
}

static void
toml_node_walker_free(struct toml_node* node, __attribute__((unused)) void* ctx)
{
	if (node->name)
		free(node->name);

	switch (node->type) {
	case TOML_ROOT:
	case TOML_INLINE_TABLE:
	case TOML_TABLE: {
		struct toml_table_item *item = NULL, *next = NULL;

		list_for_each_safe(&node->value.map, item, next, map) {
			list_del(&item->map);
			free(item);
		}
		break;
	}

	case TOML_TABLE_ARRAY:
	case TOML_LIST: {
		struct toml_list_item *item = NULL, *next = NULL;

		list_for_each_safe(&node->value.list, item, next, list) {
			list_del(&item->list);
			free(item);
		}
		break;
	}

	case TOML_STRING:
		free(node->value.string);
		break;

	default:
		break;
	}
}

void
toml_free(struct toml_node *toml_root)
{
	assert(toml_root->type == TOML_ROOT);
	toml_dive(toml_root, toml_node_walker_free, NULL);
	free(toml_root);
}

char*
toml_value_as_string(struct toml_node* node)
{
	char* ret = NULL;

	switch (node->type)
	{
	case TOML_INT:
		asprintf(&ret, "%" PRId64, node->value.integer);
		break;

	case TOML_FLOAT:
		asprintf(&ret, "%.*f", node->value.floating.precision,
								node->value.floating.value);
		break;

	case TOML_STRING:
		ret = _json_string_encode(node->value.string);
		break;

	case TOML_DATE: {
		struct tm	tm;
		char		offset_string[7] = "Z";
		char		sec_frac[1024] = "";

		if (!node->value.rfc3339_time.offset_is_zulu)
			sprintf(offset_string, "%s%02d:%02d",
				node->value.rfc3339_time.offset_sign_negative ? "-" : "+",
				node->value.rfc3339_time.offset / 60,
				node->value.rfc3339_time.offset % 60);

		if (node->value.rfc3339_time.sec_frac != -1)
			snprintf(sec_frac, sizeof(sec_frac), ".%d",
											node->value.rfc3339_time.sec_frac);

		if (!gmtime_r(&node->value.rfc3339_time.epoch, &tm))
			break;

		asprintf(&ret, "%d-%02d-%02dT%02d:%02d:%02d%s%s",
			1900 + tm.tm_year,
			tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
			sec_frac, offset_string);
		break;
	}

	case TOML_BOOLEAN:
		asprintf(&ret, "%s", node->value.integer ? "true" : "false");
		break;

	default:
		break;
	}

	return ret;
}

enum toml_type
toml_type(struct toml_node* node)
{
	return node->type;
}

char*
toml_name(struct toml_node* node)
{
	return _json_string_encode(node->name);
}
