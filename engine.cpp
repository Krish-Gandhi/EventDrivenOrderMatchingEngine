#include "engine.h"
#include <string>
#include <iostream>

Order::Order(OrderType orderType, int orderVolume, float pricePerShare, string orderVisibility) : orderType(orderType), orderVolume(orderVolume), pricePerShare(pricePerShare), orderVisibility(orderVisibility) {}

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
