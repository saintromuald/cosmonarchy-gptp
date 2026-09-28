#include "utils/enum.h"

#include <cctype>

std::string enum_name_to_snake_case(std::string_view name) {
    std::string result;
    result.reserve(name.size());
    for (size_t i = 0; i < name.size(); ++i) {
        const auto current = name[i];
        const auto upper = isupper(current);
        const auto previous_lower = i > 0 && islower(name[i - 1]);
        const auto next_lower = i + 1 < name.size() && islower(name[i + 1]);
        if (upper && (previous_lower || (i > 0 && next_lower))) {
            result += '_';
        }
        result += char(tolower(current));
    }
    return result;
}