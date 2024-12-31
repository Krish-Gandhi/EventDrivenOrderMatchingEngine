#include "engine.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <ctime>
#include <string_view>


Order::Order(OrderType orderType, int orderVolume, float pricePerShare, string orderVisibility) : orderType(orderType), orderVolume(orderVolume), pricePerShare(pricePerShare), orderVisibility(orderVisibility) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_time_t);
    auto duration = now.time_since_epoch();
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() % 1000000000;
    std::stringstream ss;
    ss << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S") // Date and time part
       << "." << std::setw(9) << std::setfill('0') << nanoseconds; // Nanoseconds part

    timestampString = ss.str();
}

std::string Order::enumToString(OrderType o) {
    switch (o) {
        case OrderType::MARKET_BUY: return "MARKET_BUY";
        case OrderType::MARKET_SELL: return "MARKET_SELL";
        case OrderType::LIMIT_BUY: return "LIMIT_BUY";
        case OrderType::LIMIT_SELL: return "LIMIT_SELL";
        case OrderType::STOP_BUY: return "STOP_BUY";
        case OrderType::STOP_SELL: return "STOP_SELL";
        case OrderType::STOP_LIMIT_BUY: return "STOP_LIMIT_BUY";
        case OrderType::STOP_LIMIT_SELL: return "STOP_LIMIT_SELL";
        case OrderType::FOK_BUY: return "FOK_BUY";
        case OrderType::FOK_SELL: return "FOK_SELL";
        case OrderType::TRAILING_STOP_BUY: return "TRAILING_STOP_BUY";
        case OrderType::TRAILING_STOP_SELL: return "TRAILING_STOP_SELL";
        default: return "Unknown";
    }
}

std::string Order::getOrderType(){
    return enumToString(orderType);
}

float Order::getPrice(){
    return pricePerShare;
}

std::string Order::getOrderVisibility(){
    return orderVisibility;
}
std::string Order::getTimestamp(){
    return timestampString;
}

std::string Order::printOrder() const{
    std::stringstream out;
    out << std::setprecision(2) << std::fixed;
    out << enumToString(orderType) << " | " << orderVolume << " | $" << pricePerShare << " | " << orderVisibility << " | " << timestampString;
    return out.str();
}

std::string Engine::printBook(){
    auto copyPQ = sellOrders;
    std::stringstream ret;
    ret << "===================" << std::endl << "Sell Orders:" << std::endl;
    while(!copyPQ.empty()){
        ret << copyPQ.top()->printOrder() << std::endl;
        copyPQ.pop();
    }

    auto copy2PQ = buyOrders;
    ret << "===================" << std::endl << "Buy Orders:" << std::endl;
    while(!copy2PQ.empty()){
        ret << copy2PQ.top()->printOrder() << std::endl;
        copy2PQ.pop();
    }

    return ret.str();
}

Engine::~Engine(){
    while(!sellOrders.empty())
        sellOrders.pop();
    while(!buyOrders.empty())
        buyOrders.pop();
}

Order* Engine::match(Order* o){
    std::string type = o -> getOrderType();
    if (type[type.length() - 4] == 'S')
        sellOrders.push(o);
    else
        buyOrders.push(o);
    
    return NULL;
}

