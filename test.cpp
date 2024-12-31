#include "engine.h"
#include <string>
#include <iostream>

int main(){
    Engine engine = Engine();
    for (int i = 1; i < 6; ++i){
        Order* order = new Order(OrderType::LIMIT_BUY, 10*i, 3);  // Cast integer to enum
        engine.match(order);
    }
    for (int i = 1; i < 6; ++i){
        Order* order = new Order(OrderType::LIMIT_BUY, 10*i, i);  // Cast integer to enum
        engine.match(order);
    }
    Order* order = new Order(OrderType::LIMIT_BUY, 40, 3, "hidden");  // Cast integer to enum
    engine.match(order);
    for (int i = 1; i < 6; ++i){
        Order* order = new Order(OrderType::LIMIT_SELL, 10*i, i);  // Cast integer to enum
        engine.match(order);
    }
    std::cout << engine.printBook() << std::endl;
    std::cout << "Done!";
    return 0;
}