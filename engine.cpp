#include "engine.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <ctime>
#include <string_view>
#include <functional>


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
    orderTypeInt = static_cast<int>(orderType);
    orderID = 0;
}

// Copy constructor
Order::Order(const Order& other){
    orderType = other.orderType;
    orderTypeInt = other.orderTypeInt;
    orderVolume = other.orderVolume;
    pricePerShare = other.pricePerShare;
    orderVisibility = other.orderVisibility;
    orderID = other.orderID;
    timestampString = other.timestampString;
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

int Order::getOrderVolume(){
    return orderVolume;
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
    out << enumToString(orderType) << " | " << orderVolume << " | ";
    if (orderTypeInt > 1) 
        out << "$" << pricePerShare << " | ";
    out << orderVisibility << " | " << timestampString;
    return out.str();
}

void Order::removeVolume(int volume){
    orderVolume -= volume;
}

Summary::Summary(Order* o){
    averagePrice = 0;
    mostFavorablePrice = 0;
    volume = 0;
    status = "PENDING";
    message = "NO MESSAGE AVIALABLE";
    incomingOrder = o;
    matchedOrders = {};
}

Summary::~Summary(){
    while(!matchedOrders.empty()){
        matchedOrders.pop_back();
    }
}

void Summary::add(Order* o){
    matchedOrders.push_back(o);
    float cost = 0;
    int totVolume = 0;
    for(int i = 0; i < matchedOrders.size(); ++i){
        cost += matchedOrders[i] -> getPrice() * matchedOrders[i] -> getOrderVolume();
        totVolume += matchedOrders[i] -> getOrderVolume();
    }
    mostFavorablePrice = matchedOrders[0] -> getPrice();
    volume = totVolume;
    averagePrice = cost / totVolume;
}

void Summary::setStatus(std::string s){
    status = s;
}

void Summary::setMessage(std::string s){
    message = s;
}

float Summary::getBestPrice(){
    return mostFavorablePrice;
}

std::string Summary::printSummary(){
    std::stringstream out;
    out << std::setprecision(2) << std::fixed;
    out << "========================SUMMARY OF TRANSCATION========================" << endl;
    out << "STATUS:              " << status << endl;
    out << "MESSAGE:             " << message << endl;
    out << "TOTAL TRADE VOLUME:  " << volume << endl;
    out << "AVERAGE PRICE:       $" << averagePrice << endl;
    out << "TOTAL PRICE:         $" << averagePrice * volume << endl;
    out << "INCOMING ORDER:      " << incomingOrder -> printOrder() << endl;
    out << "MATCHED ORDER(s):      " << endl;
    if (matchedOrders.empty())
        out << "NONE" << endl;
    else{
        for(int i = 0; i < matchedOrders.size(); ++i){
            out << matchedOrders[i] -> printOrder() << endl;
        }
    }
    out << "======================================================================" << endl;
    return out.str();
}

Engine::Engine(){
    matchingFunctions = {
        [this](Order* order) { return this->matchMarketBuy(order); },
        [this](Order* order) { return this->matchMarketSell(order); },
        [this](Order* order) { return this->matchLimitBuy(order); },
        [this](Order* order) { return this->matchLimitSell(order); },
        [this](Order* order) { return this->matchStopBuy(order); },
        [this](Order* order) { return this->matchStopSell(order); },
        [this](Order* order) { return this->matchStopLimitBuy(order); },
        [this](Order* order) { return this->matchStopLimitSell(order); },
        [this](Order* order) { return this->matchFOKBuy(order); },
        [this](Order* order) { return this->matchFOKSell(order); },
        [this](Order* order) { return this->matchTrailingStopBuy(order); },
        [this](Order* order) { return this->matchTrailingStopSell(order); }
    };
}

std::string Engine::printBook(){
    auto copyPQ = sellOrders;
    std::stringstream ret;
    ret << "===================" << std::endl << "Sell Orders:" << std::endl;
    if (sellOrders.empty())
        ret << "None" << std::endl;
    while(!copyPQ.empty()){
        ret << copyPQ.top()->printOrder() << std::endl;
        copyPQ.pop();
    }

    auto copy2PQ = buyOrders;
    ret << "===================" << std::endl << "Buy Orders:" << std::endl;
    if (buyOrders.empty())
        ret << "None" << std::endl;
    while(!copy2PQ.empty()){
        ret << copy2PQ.top()->printOrder() << std::endl;
        copy2PQ.pop();
    }

    return ret.str();
}

Engine::~Engine(){
    while(!sellOrders.empty()){
        delete sellOrders.top();
        sellOrders.pop();
    }
    while(!buyOrders.empty()){
        delete buyOrders.top();
        buyOrders.pop();
    }
}

Summary* Engine::matchMarketBuy(Order* order){
    int remVolume = order -> getOrderVolume();
    Summary* ret = new Summary(order);
    if (sellOrders.empty()){ 
        ret -> setStatus("CANCELED"); 
        ret -> setMessage("No sell orders available."); 
        return ret;
    }
    while(remVolume > 0){
        Order* workingOrder = sellOrders.top();
        if (workingOrder -> getOrderVolume() > remVolume){
            Order* completed = new Order(*workingOrder); 
            completed -> removeVolume(workingOrder ->getOrderVolume() - remVolume);
            workingOrder -> removeVolume(remVolume);
            ret -> add(completed);
            remVolume = 0;
            break;
        }else{ // workingOrder -> getOrderVolume() <= remVolume
            ret -> add(workingOrder);
            remVolume -= workingOrder -> getOrderVolume();
            sellOrders.pop();
        }

        if (sellOrders.empty()){ 
            ret -> setStatus("PARTIALLY FILLED"); 
            ret -> setMessage("Not enough sell orders available to fully complete this transaction."); 
            return ret;
        }
    }
    ret -> setStatus("FILLED"); 
    ret -> setMessage("Complete transaction."); 
    return ret;
}

Summary* Engine::matchMarketSell(Order* order){
    int remVolume = order -> getOrderVolume();
    Summary* ret = new Summary(order);
    if (buyOrders.empty()){ 
        ret -> setStatus("CANCELED"); 
        ret -> setMessage("No buy orders available."); 
        return ret;
    }
    while(remVolume > 0){
        Order* workingOrder = buyOrders.top();
        if (workingOrder -> getOrderVolume() > remVolume){
            Order* completed = new Order(*workingOrder); 
            completed -> removeVolume(workingOrder ->getOrderVolume() - remVolume);
            workingOrder -> removeVolume(remVolume);
            ret -> add(completed);
            remVolume = 0;
            break;
        }else{ // workingOrder -> getOrderVolume() <= remVolume
            ret -> add(workingOrder);
            remVolume -= workingOrder -> getOrderVolume();
            buyOrders.pop();
        }

        if (buyOrders.empty()){ 
            ret -> setStatus("PARTIALLY FILLED"); 
            ret -> setMessage("Not enough buys orders available to fully complete this transaction."); 
            return ret;
        }
    }
    ret -> setStatus("FILLED"); 
    ret -> setMessage("Complete transaction."); 
    return ret;
}

Summary* Engine::matchLimitBuy(Order* order){
    int remVolume = order -> getOrderVolume();
    Summary* ret = new Summary(order);
    if (sellOrders.empty()){ 
        ret -> setStatus("PENDING"); 
        ret -> setMessage("No sell orders available, so limit order has been added to order book.");
        buyOrders.push(order);
        return ret;
    }
    while(remVolume > 0 && sellOrders.top() -> getPrice() <= order -> getPrice() && !sellOrders.empty()){
        Order* workingOrder = sellOrders.top();
        if (workingOrder -> getOrderVolume() > remVolume){
            Order* completed = new Order(*workingOrder); 
            completed -> removeVolume(workingOrder ->getOrderVolume() - remVolume);
            workingOrder -> removeVolume(remVolume);
            ret -> add(completed);
            remVolume = 0;
            break;
        }else{ // workingOrder -> getOrderVolume() <= remVolume
            ret -> add(workingOrder);
            remVolume -= workingOrder -> getOrderVolume();
            sellOrders.pop();
        }
    }
    if (remVolume > 0){ 
        if (remVolume != order -> getOrderVolume())
            ret -> setStatus("PARTIALLY FILLED"); 
        else     
            ret -> setStatus("PENDING"); 
        ret -> setMessage("Not enough sell orders under limit price available to fully complete this transaction, so limit order has been added to order book.");
        Order* unmatched = new Order(*order);
        unmatched -> removeVolume(order -> getOrderVolume() - remVolume);
        buyOrders.push(unmatched);
        return ret;
    }
    ret -> setStatus("FILLED"); 
    ret -> setMessage("Complete transaction."); 
    return ret;
}

Summary* Engine::matchLimitSell(Order* order){
    int remVolume = order -> getOrderVolume();
    Summary* ret = new Summary(order);
    if (buyOrders.empty()){ 
        ret -> setStatus("PENDING"); 
        ret -> setMessage("No buy orders available, so limit order has been added to order book.");
        sellOrders.push(order);
        return ret;
    }
    while(remVolume > 0 && buyOrders.top() -> getPrice() >= order -> getPrice() && !buyOrders.empty()){
        Order* workingOrder = buyOrders.top();
        if (workingOrder -> getOrderVolume() > remVolume){
            Order* completed = new Order(*workingOrder); 
            completed -> removeVolume(workingOrder ->getOrderVolume() - remVolume);
            workingOrder -> removeVolume(remVolume);
            ret -> add(completed);
            remVolume = 0;
            break;
        }else{ // workingOrder -> getOrderVolume() <= remVolume
            ret -> add(workingOrder);
            remVolume -= workingOrder -> getOrderVolume();
            buyOrders.pop();
        }
    }
    if (remVolume > 0){ 
        if (remVolume != order -> getOrderVolume())
            ret -> setStatus("PARTIALLY FILLED"); 
        else     
            ret -> setStatus("PENDING"); 
        ret -> setMessage("Not enough buy orders above limit price available to fully complete this transaction, so limit order has been added to order book.");
        Order* unmatched = new Order(*order);
        unmatched -> removeVolume(order -> getOrderVolume() - remVolume);
        sellOrders.push(unmatched);
        return ret;
    }
    ret -> setStatus("FILLED"); 
    ret -> setMessage("Complete transaction."); 
    return ret;
}

Summary* Engine::matchStopBuy(Order* order){
    Summary* ret = new Summary(order);
    pendingOrders.push_back(order);
    ret -> setStatus("PENDING");
    ret -> setMessage("Order has been added to pending queue.");
    return ret;
}

Summary* Engine::matchStopSell(Order* order){
    Summary* ret = new Summary(order);
    pendingOrders.push_back(order);
    ret -> setStatus("PENDING");
    ret -> setMessage("Order has been added to pending queue.");
    return ret;
}

Summary* Engine::matchStopLimitBuy(Order* order){
    Summary* ret = new Summary(order);
    pendingOrders.push_back(order);
    ret -> setStatus("PENDING");
    ret -> setMessage("Order has been added to pending queue.");
    return ret;
}

Summary* Engine::matchStopLimitSell(Order* order){
    Summary* ret = new Summary(order);
    pendingOrders.push_back(order);
    ret -> setStatus("PENDING");
    ret -> setMessage("Order has been added to pending queue.");
    return ret;
}

Summary* Engine::matchFOKBuy(Order* order){
    int remVolume = order -> getOrderVolume();
    Summary* ret = new Summary(order);
    auto copyPQ = sellOrders;
    if (copyPQ.empty()){ 
        ret -> setStatus("CANCELED"); 
        ret -> setMessage("No sell orders available."); 
        return ret;
    }
    // Check if FOK possible
    while(!copyPQ.empty() && copyPQ.top() -> getPrice() <= order->getPrice()){
        remVolume -= copyPQ.top() -> getOrderVolume();
        if (remVolume <= 0)
            break;
        copyPQ.pop();
    }
    if (remVolume > 0){
        ret -> setStatus("CANCELED"); 
        ret -> setMessage("Not enough sell orders under limit price available to fully complete this transaction."); 
        return ret;
    }
    // Execute FOK
    remVolume = order -> getOrderVolume();
    while(remVolume > 0){
        Order* workingOrder = sellOrders.top();
        if (workingOrder -> getOrderVolume() > remVolume){
            Order* completed = new Order(*workingOrder); 
            completed -> removeVolume(workingOrder ->getOrderVolume() - remVolume);
            workingOrder -> removeVolume(remVolume);
            ret -> add(completed);
            remVolume = 0;
            break;
        }else{ // workingOrder -> getOrderVolume() <= remVolume
            ret -> add(workingOrder);
            remVolume -= workingOrder -> getOrderVolume();
            sellOrders.pop();
        }
    }
    ret -> setStatus("FILLED"); 
    ret -> setMessage("Complete transaction."); 
    return ret;
}

Summary* Engine::matchFOKSell(Order* order){
    int remVolume = order -> getOrderVolume();
    Summary* ret = new Summary(order);
    auto copyPQ = buyOrders;
    if (copyPQ.empty()){ 
        ret -> setStatus("CANCELED"); 
        ret -> setMessage("No buy orders available."); 
        return ret;
    }
    // Check if FOK possible
    while(!copyPQ.empty() && copyPQ.top() -> getPrice() >= order->getPrice()){
        remVolume -= copyPQ.top() -> getOrderVolume();
        if (remVolume <= 0)
            break;
        copyPQ.pop();
    }
    if (remVolume > 0){
        ret -> setStatus("CANCELED"); 
        ret -> setMessage("Not enough buy orders above limit price available to fully complete this transaction."); 
        return ret;
    }
    // Execute FOK
    remVolume = order -> getOrderVolume();
    while(remVolume > 0){
        Order* workingOrder = buyOrders.top();
        if (workingOrder -> getOrderVolume() > remVolume){
            Order* completed = new Order(*workingOrder); 
            completed -> removeVolume(workingOrder ->getOrderVolume() - remVolume);
            workingOrder -> removeVolume(remVolume);
            ret -> add(completed);
            remVolume = 0;
            break;
        }else{ // workingOrder -> getOrderVolume() <= remVolume
            ret -> add(workingOrder);
            remVolume -= workingOrder -> getOrderVolume();
            buyOrders.pop();
        }
    }
    ret -> setStatus("FILLED"); 
    ret -> setMessage("Complete transaction."); 
    return ret;
}

Summary* Engine::matchTrailingStopBuy(Order* order){
    Summary* ret = new Summary(order);
    pendingOrders.push_back(order);
    ret -> setStatus("PENDING");
    ret -> setMessage("Order has been added to pending queue.");
    return ret;
}

Summary* Engine::matchTrailingStopSell(Order* order){
    Summary* ret = new Summary(order);
    pendingOrders.push_back(order);
    ret -> setStatus("PENDING");
    ret -> setMessage("Order has been added to pending queue.");
    return ret;
}

Summary* Engine::match(Order* o){
    Summary* ret = matchingFunctions[o -> orderTypeInt](o);
    lastSale = ret -> getBestPrice();
    updateBook(lastSale);
    return ret;
}

void Engine::updateBook(float lastSale){
    for(Order* order : pendingOrders){
        // write updateFunctions logic
    }
}

