#include "engine.h"
#include <string>
#include <iostream>

int main(){
    Engine engine = Engine();
    for (int i = 1; i < 4; ++i){
        Order* order1 = new Order(OrderType::LIMIT_BUY, 10*i, 3 + (0.11 * (4-i)));  // Cast integer to enum
        engine.match(order1);
        // Order* order = new Order(OrderType::LIMIT_SELL, 10*i, 3 + (0.11 * (4-i)));  // Cast integer to enum
        // engine.match(order);
    }
    //std::cout << engine.printBook() << std::endl;

    //std::cout << "Sending the limit sell orders!" << std::endl;

    Order* marketBuy6 = new Order(OrderType::LIMIT_SELL, 20, 4);
    Summary* testing = engine.match(marketBuy6);
    //std::cout << testing -> printSummary();
    std::cout << engine.printBook() << std::endl;

    Order* marketBuy7 = new Order(OrderType::STOP_SELL, 10, 3.22, "public", 3.44);
    testing = engine.match(marketBuy7);
    std::cout << testing -> printSummary();
    std::cout << engine.printBook() << std::endl;

    Order* marketBuy9 = new Order(OrderType::STOP_SELL, 15, 3.22, "public", 3.44);
    testing = engine.match(marketBuy9);
    std::cout << testing -> printSummary();
    std::cout << engine.printBook() << std::endl;

    Order* marketBuy8 = new Order(OrderType::MARKET_BUY, 20);
    testing = engine.match(marketBuy8);
    std::cout << testing -> printSummary();
    std::cout << engine.printBook() << std::endl;

    std::cout << "Done!";
    return 0;
}