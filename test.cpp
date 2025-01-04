#include "engine.h"
#include <string>
#include <iostream>

int main(){
    Engine engine = Engine();
    for (int i = 1; i < 4; ++i){
        Order* order1 = new Order(OrderType::LIMIT_BUY, 10*i, 3 + (0.11 * (4-i)));  // Cast integer to enum
        engine.match(order1);
        //std::cout << engine.printBook() << std::endl;
        std::cout << "Sent buy, sending sell..." << std::endl; 
        Order* order = new Order(OrderType::LIMIT_SELL, 10*i, 3 + (0.11 * (4-i)));  // Cast integer to enum
        engine.match(order);
        //std::cout << engine.printBook() << std::endl;
    }
    std::cout << engine.printBook() << std::endl;

    std::cout << "Sending the limit buy orders!" << std::endl;

    Order* marketBuy6 = new Order(OrderType::LIMIT_SELL, 20, 4);
    Summary* testing = engine.match(marketBuy6);
    //std::cout << testing -> printSummary();
    std::cout << engine.printBook() << std::endl;

    Order* marketBuy7 = new Order(OrderType::LIMIT_SELL, 40, 3.20);
    testing = engine.match(marketBuy7);
    //std::cout << testing -> printSummary();
    std::cout << engine.printBook() << std::endl;

    Order* marketBuy8 = new Order(OrderType::LIMIT_SELL, 40, 3.10);
    testing = engine.match(marketBuy8);
    std::cout << testing -> printSummary();
    std::cout << engine.printBook() << std::endl;

    std::cout << "Done!";
    return 0;
}