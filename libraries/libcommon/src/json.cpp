//
// Created by 98max on 18/04/2026.
//

#include <json.h>

using namespace MaxOS;
using namespace MaxOS::common;

JSONNode JSONNode::NullNode = JSONNode("", nullptr, UNKNOWN);

JSONNode::JSONNode(string key, void* value, JSONNodeType type)
	: m_key(key),
	  m_value(value),
	  m_type(type) {
}

JSONNode::~JSONNode() {

	// Null node
	if (m_value == nullptr)
		return;

	switch (m_type) {
		case STRING:
			delete (string*)m_value;
			break;

		case ARRAY: {

			// Delete all the child nodes
			auto* vector = (Vector<JSONNode*>*)m_value;
			for (uint32_t i = 0; i < vector->size(); i++)
				delete (*vector)[i];

			// Delete the vector
			delete vector;
			break;
		}

		case OBJECT:
			// Cast to JSONObject and let its destructor handle the Map
			if (m_value != (void*)this)
				delete (JSONObject*)m_value;
			break;

		default:
			break;
	}
}

JSONNode::operator String() {

	if (m_type == STRING)
		return *(string*)m_value;

	return "";
}

JSONNode::operator int64_t() {
	if (m_type == NUMBER)
		return (int64_t)(uint64_t)m_value;

	return 0;
}

JSONNode::operator bool() {
	if (m_type == BOOL)
		return (bool)(uint64_t)m_value;

	return false;
}

JSONNode &JSONNode::operator[](int index) {

	if (m_type == ARRAY)
		return *(*(Vector<JSONNode*>*)(m_value))[index];

	return NullNode;

}

JSONNode &JSONNode::operator[](string key) {

	if (m_type == OBJECT)
		return (*(JSONObject*)m_value)[key];

	return NullNode;
}

string JSONNode::to_string(int level) {

	// Get the indent at two spaces per level
	string indent = "  "s * level;

	switch (m_type) {

		case STRING:
			return "\""s + *(string*)m_value + "\"";

		case NUMBER:
			return string((int)(uint64_t)m_value);

		case BOOL:
			return ((bool)(uint64_t)m_value) ? "true" : "false";

		case ARRAY: {

			// Get the vector
			Vector<JSONNode*>* vector = (Vector<JSONNode*>*)m_value;
			if (vector->size() == 0)
				return "[]";

			/// Add each node to the output
			string out = "[\n";
			for (uint32_t i = 0; i < vector->size(); i++) {
				out += indent + "  " + (*vector)[i]->to_string(level + 1);

				// Move to the next node
				if (i < vector->size() - 1)
					out += ",";
				out += "\n";
			}

			// Close the array
			out += indent + "]";
			return out;
		}

		case OBJECT:
			return ((JSONObject*)this)->to_string(level);

		case UNKNOWN: default:
			return "null";
	}

}

JSONObject::JSONObject(string key, Map<string, JSONNode*> values)
	: JSONNode(key, this, OBJECT),
	  m_values(values),
	  m_self_key(key) {
}

JSONObject::~JSONObject() {

	// Delete all child nodes
	for (int i = 0; i < m_values.size(); i++)
		delete m_values[i];

}

JSONNode &JSONObject::get_value(string key) {

	// Invaild key
	auto value = m_values.find(key);
	if (value == m_values.end())
		return NullNode;

	return *value->second;
}

void JSONObject::set_value(string key, JSONNode* value) {

	m_values.insert(key, value);

}

JSONNode &JSONObject::operator[](string key) {
	return get_value(key);
}

string JSONObject::to_string(int level) {

	// Empty object
	if (m_values.size() == 0)
		return "{}";

	// Get the indent at two spaces per level
	string indent = "  "s * level;

	// Add each keypair to the output
	string out = "{";
	for (auto it = m_values.begin(); it != m_values.end(); ++it) {

		// Get the value pair
		string    key   = it->first;
		JSONNode* value = it->second;

		// Add to the output
		out += indent + "  \"" + key + "\": " + value->to_string(level + 1);

		// Move to next
		auto next_it = it;
		if (++next_it != m_values.end())
			out += ",";
		out += "\n";

	}

	// Close object
	out += indent + "}";
	return out;
}

JSONParser::JSONParser(string* source)
: m_src(source) {
	m_root = parse_value("root");
}

JSONParser::~JSONParser() = default;

JSONNode* JSONParser::root() {
	return m_root;
}

void JSONParser::skip_whitespace() {

	while (m_pos < m_src->length()) {
		char c = (*m_src)[m_pos];
		if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
			m_pos++;
		}
		else {
			break;
		}
	}

}

char JSONParser::peek() {
	return (*m_src)[m_pos];
}

char JSONParser::next() {
	return (*m_src)[m_pos++];
}

JSONNode* JSONParser::parse_value(string key) {

	// Get the first char
	skip_whitespace();
	char c = peek();

	switch (c) {

		case '{': {
			return parse_object(key);
		}

		case '[': {
			return parse_array(key);
		}

		case '"': {
			return new JSONNode(key, new string(parse_string()), STRING);
		}

		case 't':
		case 'f': {

			// Parse the true/false (and skip the rest)
			bool value = next() == 't';
			m_pos      += (value ? 3 : 4);

			return new JSONNode(key, (void*)(uint64_t)value, BOOL);

		}

		default:
			break;
	}

	// Handle numbers
	if (is_digit(c))
		return new JSONNode(key, (void*)(uint64_t)parse_number(), NUMBER);

	return &JSONNode::NullNode;
}

int64_t JSONParser::parse_number() {

	skip_whitespace();

	// Skip negative sign
	size_t start = m_pos;
	if (peek() == '-')
		next();

	// Move to end of number
	while (is_digit(peek()))
		next();

	// Get the substring
	size_t length = m_pos - start;
	string result = string((uint8_t*)m_src->c_str() + start, length);

	// Conver to number
	return result.to_int();
}

string JSONParser::parse_string() {

	// Skip opening '"'
	skip_whitespace();
	next();

	// Move to the end of the string
	size_t start = m_pos;
	while (peek() != '"' && peek() != '\0') {

		//@todo handle escape characters
		if (peek() == '\\')
			next();

		next();
	}

	// Get the substring
	size_t length = m_pos - start;
	string result = string((uint8_t*)m_src->c_str() + start, length);

	// Skip closing '"'
	next();
	return result;

}

JSONNode* JSONParser::parse_array(string key) {

	// Skip opening '['
	skip_whitespace();
	next();

	// Fill the array
	Vector<JSONNode*>* vector = new Vector<JSONNode*>();
	while (peek() != ']' && peek() != '\0') {

		// Pass index as key
		skip_whitespace();
		vector->push_back(parse_value(string((int)vector->size())));

		// Move to the next element
		skip_whitespace();
		if (peek() == ',')
			next();
		skip_whitespace();

	}

	// Skip closing ']'
	next();
	return new JSONNode(key, (void*)vector, ARRAY);

}

JSONObject* JSONParser::parse_object(string key) {

	// Skip '{'
	skip_whitespace();
	next();

	// Fill the object
	Map<string, JSONNode*> values;
	while (peek() != '}' && peek() != '\0') {

		// Extract the key
		skip_whitespace();
		string child_key = parse_string();

		// Invalid JSON
		skip_whitespace();
		if (next() != ':')
			break;

		// Parse the value
		values.insert(child_key, parse_value(child_key));

		// Move to next
		skip_whitespace();
		if (peek() == ',')
			next();

	}

	// Skip closing '}'
	next();
	return new JSONObject(key, values);

}
