/**
 * @brief Implementation of JSON document functions.
 *
 * @file
 * @ingroup utils
 */

#include "utils/io/json/document.hpp"

#include "utils/core/fs.hpp"
#include "utils/core/logging.hpp"

namespace genesis {
namespace utils {

// =============================================================================
//     JsonDocument
// =============================================================================

// TODO write a validate json doc function that checks if there are no recurrencies in the values,
// TODO meaning that the pointers in an object or array need to point to unique values, and not for
// TODO example to themselves or their parent objects.
/*
bool JsonDocument::validate()
{
    std::deque<JsonValue*> pointers;
    std::stack<JsonValue*> callstack;

    for (ObjectData::value_type v : value) {
        callstack.push(v.second);
    }

    while (!callstack.empty()) {
        break;
    }
    return true;
}
*/

// =============================================================================
//     Converter Functions
// =============================================================================

/**
 * @brief Converts a pointer to a JsonValue to JsonValueNull if appropriate.
 *
 * Triggers a warning and returns a nullptr if the dynamic type of the object is not actually
 * a JsonValueNull object.
 */
JsonValueNull* json_value_to_null (const JsonValue* v)
{
    if (v->type() != JsonValue::kNull) {
        LOG_WARN << "Invalid conversion from JsonValue::" << v->type_to_string()
                 << " to JsonValue::Null.";
        return nullptr;
    }
    return static_cast<JsonValueNull*> (const_cast<JsonValue*> (v));
}

/**
 * @brief Converts a pointer to a JsonValue to JsonValueBool if appropriate.
 *
 * Triggers a warning and returns a nullptr if the dynamic type of the object is not actually
 * a JsonValueBool object.
 */
JsonValueBool* json_value_to_bool (const JsonValue* v)
{
    if (v->type() != JsonValue::kBool) {
        LOG_WARN << "Invalid conversion from JsonValue::" << v->type_to_string()
                 << " to JsonValue::Bool.";
        return nullptr;
    }
    return static_cast<JsonValueBool*> (const_cast<JsonValue*> (v));
}

/**
 * @brief Converts a pointer to a JsonValue to JsonValueNumber if appropriate.
 *
 * Triggers a warning and returns a nullptr if the dynamic type of the object is not actually
 * a JsonValueNumber object.
 */
JsonValueNumber* json_value_to_number (const JsonValue* v)
{
    if (v->type() != JsonValue::kNumber) {
        LOG_WARN << "Invalid conversion from JsonValue::" << v->type_to_string()
                 << " to JsonValue::Number.";
        return nullptr;
    }
    return static_cast<JsonValueNumber*> (const_cast<JsonValue*> (v));
}

/**
 * @brief Converts a pointer to a JsonValue to JsonValueString if appropriate.
 *
 * Triggers a warning and returns a nullptr if the dynamic type of the object is not actually
 * a JsonValueString object.
 */
JsonValueString* json_value_to_string (const JsonValue* v)
{
    if (v->type() != JsonValue::kString) {
        LOG_WARN << "Invalid conversion from JsonValue::" << v->type_to_string()
                 << " to JsonValue::String.";
        return nullptr;
    }
    return static_cast<JsonValueString*> (const_cast<JsonValue*> (v));
}

/**
 * @brief Converts a pointer to a JsonValue to JsonValueArray if appropriate.
 *
 * Triggers a warning and returns a nullptr if the dynamic type of the object is not actually
 * a JsonValueArray object.
 */
JsonValueArray* json_value_to_array (const JsonValue* v)
{
    if (v->type() != JsonValue::kArray) {
        LOG_WARN << "Invalid conversion from JsonValue::" << v->type_to_string()
                 << " to JsonValue::Array.";
        return nullptr;
    }
    return static_cast<JsonValueArray*> (const_cast<JsonValue*> (v));
}

/**
 * @brief Converts a pointer to a JsonValue to JsonValueObject if appropriate.
 *
 * Triggers a warning and returns a nullptr if the dynamic type of the object is not actually
 * a JsonValueObject object.
 */
JsonValueObject* json_value_to_object (const JsonValue* v)
{
    if (v->type() != JsonValue::kObject) {
        LOG_WARN << "Invalid conversion from JsonValue::" << v->type_to_string()
                 << " to JsonValue::Object.";
        return nullptr;
    }
    return static_cast<JsonValueObject*> (const_cast<JsonValue*> (v));
}

} // namespace utils
} // namespace genesis