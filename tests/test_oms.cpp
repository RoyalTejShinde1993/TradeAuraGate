
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>

#include "oms/RiskManager.hpp"

using namespace TradeAuraGate::oms;

ManagedOrder make_order(
    int32_t quantity = 100,
    double price = 100.0,
    std::string symbol = "RELIANCE",
    std::string exchange = "NSE") {
    ManagedOrder order{};
    order.total_qty = quantity;
    order.price = price;
    order.trading_symbol = std::move(symbol);
    order.exchange = std::move(exchange);
    return order;
}

void test_valid_order() {
    RiskManager risk;
    std::string reason;

    auto order = make_order();
    assert(risk.validate_order(order, reason));

    std::cout << "test_valid_order passed.\n";
}

void test_quantity_limits() {
    RiskLimits limits;
    limits.max_quantity_per_order = 100;

    RiskManager risk(limits);
    std::string reason;

    // Boundary: quantity exactly at the maximum is allowed.
    assert(risk.validate_order(make_order(100), reason));

    // Above the maximum must be rejected.
    assert(!risk.validate_order(make_order(101), reason));
    assert(!reason.empty());

    assert(!risk.validate_order(make_order(0), reason));
    assert(!risk.validate_order(make_order(-1), reason));

    std::cout << "test_quantity_limits passed.\n";
}

void test_order_value_limits() {
    RiskLimits limits;
    limits.max_value_per_order = 10000.0;

    RiskManager risk(limits);
    std::string reason;

    // 100 * 100 = 10,000: exactly at the limit.
    assert(risk.validate_order(make_order(100, 100.0), reason));

    // 101 * 100 = 10,100: above the limit.
    assert(!risk.validate_order(make_order(101, 100.0), reason));
    assert(!reason.empty());

    std::cout << "test_order_value_limits passed.\n";
}

void test_required_fields() {
    RiskManager risk;
    std::string reason;

    assert(!risk.validate_order(
        make_order(100, 100.0, "", "NSE"), reason));
    assert(!reason.empty());

    reason.clear();
    assert(!risk.validate_order(
        make_order(100, 100.0, "RELIANCE", ""), reason));
    assert(!reason.empty());

    std::cout << "test_required_fields passed.\n";
}

void test_invalid_prices() {
    RiskManager risk;
    std::string reason;

    // These tests should pass after the validation fix below.
    assert(!risk.validate_order(
        make_order(10, -1.0), reason));

    reason.clear();
    assert(!risk.validate_order(
        make_order(10, std::numeric_limits<double>::quiet_NaN()),
        reason));

    reason.clear();
    assert(!risk.validate_order(
        make_order(10, std::numeric_limits<double>::infinity()),
        reason));

    std::cout << "test_invalid_prices passed.\n";
}

int main() {
    test_valid_order();
    test_quantity_limits();
    test_order_value_limits();
    test_required_fields();
    test_invalid_prices();

    std::cout << "All OMS risk tests passed!\n";
    return 0;
}
