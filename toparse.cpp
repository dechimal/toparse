// Author: dechimal (http://libdechimal.so/)
// distribute under the WTFPL. See file LICENSE.

#include <cstring>
#include <algorithm>
#include <boost/optional/optional.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "toparse.hpp"

namespace toparse { namespace detail {

option::option(std::string name, b::optional<char> short_name, arg arg_req)
    : name(name.empty() || name.find('=') != name.npos ? (throw invalid_option_name(std::move(name))), "" : std::move(name)),
      short_name(std::move(short_name)),
      arg_req(arg_req)
{}
option::option(std::string name, char short_name, arg arg_req)
    : option(name, b::make_optional(short_name), arg_req)
{}
option::option(std::string name, arg arg_req)
    : option(name, b::none, arg_req)
{}

result parse(int argc, char * const argv[], description const & desc) {
    std::vector<std::string> pos_params;
    std::vector<std::tuple<std::string, b::optional<std::string>>> opt_params;

    auto cont = [&] {
            return argc > 0;
        };
    auto next = [&] {
            --argc;
            ++argv;
        };

    while (next(), cont()) {
        char const * str = *argv;
        if (std::strcmp(str, "--") == 0) {
            while (next(), cont()) pos_params.push_back(str);
            break;
        } else if (b::starts_with(str, "--")) {
            str += 2;
            std::size_t size = std::strlen(str);
            auto end = str+size;
            auto eq_pos = std::find(str, end, '=');
            auto i = std::find_if(desc.begin(), desc.end(), [&] (option const & opt) {
                    return std::search(str, end, opt.name.begin(), opt.name.end()) == str;
                });
            if (i == desc.end()) throw here::unrecognized_option("--" + std::string(str, eq_pos));
            auto arg_req = i->arg_req;
            auto const & long_name = i->name;
            switch (arg_req) {
            case arg::none:
                if (eq_pos != end) throw unnecessary_argument("--" + std::string(str, eq_pos));
                opt_params.emplace_back(long_name, b::none);
                break;
            case arg::mandatory:
                if (eq_pos == end) {
                    next();
                    if (!cont()) throw here::argument_not_found("--");
                    str = *argv;
                } else {
                    str = eq_pos+1;
                }
                opt_params.emplace_back(long_name, b::make_optional(std::string(str)));
                break;
            default:
                opt_params.emplace_back(long_name, eq_pos == end ? b::none : b::make_optional(std::string(eq_pos+1)));
            }
        } else if (str[0] == '-' && str[1] != '\0') {
            while (++str, *str != '\0') {
                char short_name = *str;
                auto i = std::find_if(desc.begin(), desc.end(), [&] (option const & opt) {
                        return opt.short_name == short_name;
                    });
                if (i == desc.end()) throw here::unrecognized_option(std::string("-")+*str);
                auto arg_req = i->arg_req;
                auto const & long_name = i->name;
                switch (arg_req) {
                case arg::none:
                    opt_params.emplace_back(long_name, b::none);
                    continue;
                case arg::mandatory:
                    ++str;
                    if (*str == '\0') {
                        next();
                        if (!cont()) throw here::argument_not_found(std::string("-")+*str);
                        str = *argv;
                    }
                    opt_params.emplace_back(long_name, b::make_optional(std::string(str)));
                    goto finish;
                default:
                    ++str;
                    opt_params.emplace_back(long_name, *str == '\0' ? b::none : b::make_optional(std::string(str)));
                    goto finish;
                }
            }
            finish:;
        } else {
            pos_params.push_back(str);
        }
    }

    return std::make_tuple(std::move(pos_params), std::move(opt_params));
}

}}
