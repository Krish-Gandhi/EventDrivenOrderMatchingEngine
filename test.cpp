#include "engine.h"
#include <string>
#include <iostream>

int main(){
    for (int i = 0; i < 13; ++i){
        Order order = Order(static_cast<OrderType>(i), 0);  // Cast integer to enum
        std::cout << i << ": " << order.getOrderType() << std::endl;
    }
    std::cout << "Done!";
    return 0;
}