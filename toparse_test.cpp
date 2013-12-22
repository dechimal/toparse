// Author: dechimal (http://libdechimal.so/)
// distribute under the WTFPL. See file LICENSE.

#include <tuple>
#include <iostream>
#include <vector>
#include <iterator>
#include "boost/optional/optional.hpp"
#include "toparse.hpp"

using toparse::arg;
namespace b = boost;

void test(std::vector<char const *> const & data, toparse::description const & desc,
          std::vector<std::string> const & expected_pos_params,
          std::vector<std::tuple<std::string, b::optional<std::string>>> const & expected_opt_params) {
    std::vector<char*> args{nullptr};
    for (auto p : data) {
        args.push_back(const_cast<char *>(p));
    }
    auto r = toparse::parse(args.size(), args.data(), desc);
    auto const & pos_params = std::get<0>(r);
    auto const & opt_params = std::get<1>(r);
    assert(pos_params == expected_pos_params);
    assert(opt_params == expected_opt_params);
}

int main() {
    auto expected_pos_params = std::vector<std::string> {
            "value1", "value2", "value3", "value4", "value5"
        };
    auto expected_opt_params = std::vector<std::tuple<std::string, b::optional<std::string>>> {
            std::make_tuple("flag", b::none),
            std::make_tuple("flag", b::none),
            std::make_tuple("mandatory", std::string("1")),
            std::make_tuple("mandatory", std::string("1")),
            std::make_tuple("mandatory", std::string("1")),
            std::make_tuple("mandatory", std::string("1")),
            std::make_tuple("mandatory", std::string("")),
            std::make_tuple("mandatory", std::string("")),
            std::make_tuple("mandatory", std::string("")),
            std::make_tuple("optional", std::string("1")),
            std::make_tuple("optional", std::string("1")),
            std::make_tuple("optional", b::none),
            std::make_tuple("optional", b::none),
            std::make_tuple("optional", std::string("")),
            std::make_tuple("flag", b::none),
            std::make_tuple("flag", b::none),
            std::make_tuple("mandatory", std::string("1"))
        };
    ::test({ "--flag", "-f", "value1", "--mandatory=1", "-m1", "value2", "-m", "1", "--mandatory", "1",
             "-m", "", "value3", "--mandatory", "", "--mandatory=", "--optional=1", "-o1", "value4", "--optional",
             "-o", "value5", "--optional=", "-ffm1" },
           { { "flag", 'f', arg::none },
             { "mandatory", 'm', arg::mandatory },
             { "optional", 'o', arg::optional } },
             expected_pos_params,
             expected_opt_params);
    try {
        ::test({ "--foo" }, { { "flag", arg::none } },
               {}, {});
        assert(false);
    } catch (toparse::unrecognized_option const & e) {}
    try {
        ::test({ "--foo=1" }, { { "foo", arg::none } },
               {}, {});
        assert(false);
    } catch (toparse::unnecessary_argument const & e) {}
    try {
        ::test({ "--foo" }, { { "foo", arg::mandatory } },
               {}, {});
        assert(false);
    } catch (toparse::argument_not_found const & e) {}
    try {
        ::test({ "--foo=" }, { { "foo=", arg::none } },
        {}, {});
        assert(false);
    } catch (toparse::invalid_option_name const & e) {}
}
