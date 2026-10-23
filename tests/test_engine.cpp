
#include <cassert>
#include <cmath>
#include <iostream>
#include <set>
#include <string>

#include "engine/SubscriptionManager.hpp"
#include "greek/Greek.hpp"

using namespace TradeAuraGate::engine;
using namespace TradeAuraGate::greek;

class MockSubClient : public ISubscriptionClient {
public:
    std::set<std::string> subbed;

    void subscribe(const std::string& inst) override {
        subbed.insert(inst);
    }

    void unsubscribe(const std::string& inst) override {
        subbed.erase(inst);
    }
};

void test_atm() {
    MockSubClient client;
    SubscriptionManager mgr(&client, 50, 5, 10);

    assert(mgr.calculate_atm(25000.0) == 25000);
    assert(mgr.calculate_atm(25001.0) == 25000);
    assert(mgr.calculate_atm(25024.0) == 25000);
    assert(mgr.calculate_atm(25026.0) == 25050);
    assert(mgr.calculate_atm(24976.0) == 25000);
    assert(mgr.calculate_atm(24974.0) == 24950);

    // Exact strike boundaries.
    assert(mgr.calculate_atm(25025.0) == 25050);
    assert(mgr.calculate_atm(24975.0) == 25000);

    std::cout << "test_atm passed.\n";
}

void test_subscriptions() {
    MockSubClient client;
    SubscriptionManager mgr(&client, 50, 5, 10);

    mgr.set_instrument_mapper(
        [](double strike, const std::string& type) {
            return std::to_string(static_cast<int>(strike)) + type;
        });

    mgr.on_underlying_tick(25000.0);

    // 21 strikes * 2 option types = 42 contracts.
    assert(client.subbed.size() == 42);
    assert(client.subbed.count("25000CE") == 1);
    assert(client.subbed.count("24500CE") == 1);
    assert(client.subbed.count("25500PE") == 1);

    // A tick that leaves ATM unchanged should not change subscriptions.
    const auto before = client.subbed;
    mgr.on_underlying_tick(25010.0);
    assert(client.subbed == before);

    // Move to ATM 25050; the subscription window should roll.
    mgr.on_underlying_tick(25026.0);

    assert(client.subbed.size() == 42);
    assert(client.subbed.count("24500CE") == 0);
    assert(client.subbed.count("25550CE") == 1);

    std::cout << "test_subscriptions passed.\n";
}

void test_iv() {
    const double S = 25000.0;
    const double K = 25000.0;
    const double T = 30.0 / 365.0;
    const double r = 0.05;
    const double expected_iv = 0.15;

    // Call IV round-trip.
    const double call_price =
        bs_price('C', r, S, K, T, expected_iv);

    const double call_iv =
        calculate_iv('C', S, K, T, r, call_price);

    assert(std::abs(call_iv - expected_iv) < 1e-4);

    // Put IV round-trip.
    const double put_price =
        bs_price('P', r, S, K, T, expected_iv);

    const double put_iv =
        calculate_iv('P', S, K, T, r, put_price);

    assert(std::abs(put_iv - expected_iv) < 1e-4);

    // Call price below intrinsic value: invalid.
    const double invalid_iv =
        calculate_iv('C', S, 24000.0, T, r, 900.0);

    assert(invalid_iv == 0.0);

    // Invalid inputs.
    assert(calculate_iv('C', S, K, -1.0, r, 500.0) == 0.0);
    assert(calculate_iv('C', S, K, T, r, 0.0) == 0.0);

    std::cout << "test_iv passed.\n";
}

int main() {
    test_atm();
    test_subscriptions();
    test_iv();

    std::cout << "All tests passed!\n";
    return 0;
}
