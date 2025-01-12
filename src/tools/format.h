#ifndef FORMAT_H
#define FORMAT_H

#define STR(x, ...) std::format(x, __VA_ARGS__).c_str()

constexpr std::string camelCaseToSpaced(std::string const &text) {
    std::string result;
    result.reserve(text.length() + 5); // Reserve some extra space for potential spaces

    for (size_t i = 0; i < text.length(); ++i) {
        if (i > 0 && std::isupper(text[i]) && !std::isupper(text[i - 1])) {
            result += ' ';
        }
        result += text[i];
    }

    return result;
}

#endif //FORMAT_H
