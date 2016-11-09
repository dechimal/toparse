// Author: dechimal (http://libdechimal.so/)
// distribute under the WTFPL. See file LICENSE.

#include <tuple>
#include <iostream>
#include <vector>
#include <iterator>
#include "boost/optional/optional.hpp"
#include "toparse.hpp"

using toparse::arg;
using toparse::description;
using toparse::option;
namespace b = boost;

void test(std::vector<char const *> const & data, toparse::description const & desc,
          std::vector<std::string> const & expected_pos_params,
          std::vector<std::tuple<option, b::optional<std::string>>> const & expected_opt_params,
          std::vector<std::tuple<b::optional<option>, b::optional<std::string>>> const & expected_merged_params) {
    std::vector<char const *> args{nullptr};
    for (auto p : data) {
        args.push_back(p);
    }
    auto r = toparse::parse(args.size(), args.data(), desc);
    auto const & pos_params = std::get<0>(r);
    auto const & opt_params = std::get<1>(r);
    assert(pos_params == expected_pos_params);
    assert(opt_params == expected_opt_params);

    auto merged_params = toparse::parse_merged(args.size(), args.data(), desc);
    assert(merged_params == expected_merged_params);
}

int main(int argc, char * argv[]) {
    auto expected_pos_params = std::vector<std::string> {
        "value1", "value2", "value3", "value4", "value5", "-f1", "--mandatory", "-s1",
    };
    auto desc = description{ { "flag", 'f', arg::none },
        { "mandatory", 'm', arg::mandatory },
        { "optional", 'o', arg::optional },
        { 's', arg::mandatory },
    };
    auto expected_opt_params = std::vector<std::tuple<option, b::optional<std::string>>> {
        std::make_tuple(desc[0], b::none),
        std::make_tuple(desc[0], b::none),
        std::make_tuple(desc[1], std::string("1")),
        std::make_tuple(desc[1], std::string("1")),
        std::make_tuple(desc[1], std::string("1")),
        std::make_tuple(desc[1], std::string("1")),
        std::make_tuple(desc[1], std::string("")),
        std::make_tuple(desc[3], std::string("1")),
        std::make_tuple(desc[1], std::string("")),
        std::make_tuple(desc[1], std::string("")),
        std::make_tuple(desc[2], std::string("1")),
        std::make_tuple(desc[2], std::string("1")),
        std::make_tuple(desc[2], b::none),
        std::make_tuple(desc[2], b::none),
        std::make_tuple(desc[2], std::string("")),
        std::make_tuple(desc[0], b::none),
        std::make_tuple(desc[0], b::none),
        std::make_tuple(desc[1], std::string("1"))
    };
    auto expected_merged_params = std::vector<std::tuple<b::optional<option>, b::optional<std::string>>>{
        std::make_tuple(desc[0], b::none),
        std::make_tuple(desc[0], b::none),
        std::make_tuple(b::none, std::string("value1")),
        std::make_tuple(desc[1], std::string("1")),
        std::make_tuple(desc[1], std::string("1")),
        std::make_tuple(b::none, std::string("value2")),
        std::make_tuple(desc[1], std::string("1")),
        std::make_tuple(desc[1], std::string("1")),
        std::make_tuple(desc[1], std::string("")),
        std::make_tuple(desc[3], std::string("1")),
        std::make_tuple(b::none, std::string("value3")),
        std::make_tuple(desc[1], std::string("")),
        std::make_tuple(desc[1], std::string("")),
        std::make_tuple(desc[2], std::string("1")),
        std::make_tuple(desc[2], std::string("1")),
        std::make_tuple(b::none, std::string("value4")),
        std::make_tuple(desc[2], b::none),
        std::make_tuple(desc[2], b::none),
        std::make_tuple(b::none, std::string("value5")),
        std::make_tuple(desc[2], std::string("")),
        std::make_tuple(desc[0], b::none),
        std::make_tuple(desc[0], b::none),
        std::make_tuple(desc[1], std::string("1")),
        std::make_tuple(b::none, std::string("-f1")),
        std::make_tuple(b::none, std::string("--mandatory")),
        std::make_tuple(b::none, std::string("-s1")),
    };
    ::test({ "--flag", "-f", "value1", "--mandatory=1", "-m1", "value2", "-m", "1", "--mandatory", "1",
             "-m", "", "-s1", "value3", "--mandatory", "", "--mandatory=", "--optional=1", "-o1", "value4", "--optional",
             "-o", "value5", "--optional=", "-ffm1", "--", "-f1", "--mandatory", "-s1", },
           desc,
           expected_pos_params,
           expected_opt_params,
           expected_merged_params);
    try {
        ::test({ "--foo" }, { { "flag", arg::none } },
               {}, {}, {});
        assert(false);
    } catch (toparse::unrecognized_option const &) {}
    try {
        ::test({ "--foo=1" }, { { "foo", arg::none } },
               {}, {}, {});
        assert(false);
    } catch (toparse::unnecessary_argument const &) {}
    try {
        ::test({ "--foo" }, { { "foo", arg::mandatory } },
               {}, {}, {});
        assert(false);
    } catch (toparse::argument_not_found const &) {}
    try {
        ::test({ "--foo=" }, { { "foo=", arg::none } },
        {}, {}, {});
        assert(false);
    } catch (toparse::invalid_option_name const &) {}
    {
        auto desc = toparse::detail::description_from_string("fm:o::a:::b", "flags,mandatory:,optional::,args:::");
        auto expected_desc = description{
            { 'f', arg::none },
            { 'm', arg::mandatory },
            { 'o', arg::optional },
            { 'a', arg::optional },
            { 'b', arg::none },
            { std::string("flags"), arg::none },
            { std::string("mandatory"), arg::mandatory },
            { std::string("optional"), arg::optional },
            { std::string("args:"), arg::optional },
        };
        assert(desc == expected_desc);
    }
}
