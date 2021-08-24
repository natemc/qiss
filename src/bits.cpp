#include <bits.h>
#include <doctest.h>

TEST_CASE("clz returns 64 for 0") {
    CHECK(64 == clz(0));
}

TEST_CASE("ctz returns 64 for 0") {
    CHECK(64 == ctz(0));
}

TEST_CASE("is_power_of_2") {
    CHECK(!is_power_of_2(0));
    CHECK(is_power_of_2(1));
    CHECK(is_power_of_2(2));
    CHECK(!is_power_of_2(3));
    CHECK(is_power_of_2(4));
}

TEST_CASE("log2") {
    CHECK(0 == log2(1));
    CHECK(1 == log2(2));
    CHECK(1 == log2(3));
    CHECK(2 == log2(4));
}

TEST_CASE("round_up_to_power_of_2") {
    CHECK(1 == round_up_to_power_of_2(0));
    CHECK(1 == round_up_to_power_of_2(1));
    CHECK(2 == round_up_to_power_of_2(2));
    CHECK(4 == round_up_to_power_of_2(3));
    CHECK(4 == round_up_to_power_of_2(4));
}
