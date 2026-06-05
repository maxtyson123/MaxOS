//
// Created by 98max on 18/04/2026.
//

#ifndef MAXOS_COMMON_JSON_H
#define MAXOS_COMMON_JSON_H

#include <string.h>
#include <map.h>
#include <math.h>

namespace MaxOS::common {
	enum JSONNodeType {
		UNKNOWN,
		OBJECT,
		ARRAY,
		STRING,
		NUMBER,
		BOOL,
	};

	class JSONNode {
		private:
			string       m_key;
			void*        m_value;
			JSONNodeType m_type;

		public:
			JSONNode(string key, void* value, JSONNodeType type);
			~JSONNode();

			static JSONNode NullNode;

			// Simple types
			operator int64_t();
			operator bool();

			// Complex types
			operator string();
			JSONNode &operator [](int index);
			JSONNode &operator [](string key);

			virtual string to_string(int level = 0);

			size_t array_size();
			bool is_null();
	};

	class JSONObject : public JSONNode {
		private:
			Map<string, JSONNode*> m_values;
			string                 m_self_key;

		public:
			JSONObject(string key, Map<string, JSONNode*> values);
			~JSONObject();

			JSONNode &get_value(string key);
			void      set_value(string key, JSONNode* value);

			JSONNode &operator[](string key);

			string to_string(int level = 0) final;
	};

	class JSONParser {

		private:
			string* m_src;
			size_t m_pos = 0;

			void skip_whitespace();

			char peek();
			char next();

			JSONNode*   parse_value(string key = "");
			int64_t     parse_number();
			string      parse_string();
			JSONNode*	parse_array(string key);
			JSONObject* parse_object(string key);

			JSONNode* m_root;

		public:
			JSONParser(string* source);
			~JSONParser();

			JSONNode* root();
	};
}

#endif //MAXOS_COMMON_JSON_H
