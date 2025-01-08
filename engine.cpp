#include "engine.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <ctime>
#include <string_view>
#include <functional>


Order::Order(OrderType orderType, int orderVolume, float pricePerShare, string orderVisibility, float stopPrice) : orderType(orderType), orderVolume(orderVolume), pricePerShare(pricePerShare), orderVisibility(orderVisibility), stopPrice(stopPrice) {
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
    orderID = timestampString.substr(3, 1) + timestampString.substr(5, 2) + timestampString.substr(8, 2) + timestampString.substr(20, 1) +   
              timestampString.substr(14, 2) + timestampString.substr(17, 2) + timestampString.substr(23, 3) + timestampString.substr(27, 2) + 
              timestampString.substr(22, 1) + timestampString.substr(26, 1) + timestampString.substr(21, 1);
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
    stopPrice = other.stopPrice;
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
        default: return "Unknown";
    }
}

std::string Order::getOrderType(){
    return enumToString(orderType);
}

float Order::getPrice(){
    return pricePerShare;
}

float Order::getStopPrice(){
    return stopPrice;
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
    out << " | Order ID: "<< orderID;
    return out.str();
}

void Order::removeVolume(int volume){
    orderVolume -= volume;
}

Summary::Summary(Order* o){
    averagePrice = 0;
    mostFavorablePrice = -1;
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
        mostFavorablePrice = matchedOrders[0] -> getPrice();
    }
    volume = totVolume;
    averagePrice = cost / totVolume;
}

void Summary::addTriggered(Summary* s){
    triggeredOrders.push_back(s);
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
    out << "TRIGGERED STOP ORDER(s):      " << endl;
    if (triggeredOrders.empty())
        out << "NONE" << endl;
    else{
        for(int i = 0; i < triggeredOrders.size(); ++i){
            out << "TRIGGERED ORDER " << i+1 << ": "<< endl;
            out << triggeredOrders[i] -> printSummary() << endl;
        }
    }
    out << "~~~~~~END OF ORDER SUMMARY~~~~~~" << endl;
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
        [this](Order* order) { return this->matchFOKSell(order); }
    };
    updateFunctions = {
        [this](Order* order) { return this->updateStopBuy(order); },
        [this](Order* order) { return this->updateStopBuy(order); },
        [this](Order* order) { return this->updateStopBuy(order); },
        [this](Order* order) { return this->updateStopBuy(order); },
        [this](Order* order) { return this->updateStopBuy(order); },
        [this](Order* order) { return this->updateStopSell(order); },
        [this](Order* order) { return this->updateStopLimitBuy(order); },
        [this](Order* order) { return this->updateStopLimitSell(order); },
        [this](Order* order) { return this->updateStopBuy(order); },
        [this](Order* order) { return this->updateStopBuy(order); }
    };
}

std::string Engine::printBook(){
    auto copyPQ = sellOrders;
    std::stringstream ret;
    ret << "===================PRINTING ORDER BOOK===================" << std::endl;
    ret << "Last Sale Price: $" << lastSale << endl;
    ret << "Sell Orders:" << std::endl;
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

    auto copy3PQ = pendingSellOrders;
    ret << "===================" << std::endl << "Pending Sell Orders:" << std::endl;
    if (pendingSellOrders.empty())
        ret << "None" << std::endl;
    while(!copy3PQ.empty()){
        ret << copy3PQ.top() -> printOrder() << std::endl;
        copy3PQ.pop();
    }

    auto copy4PQ = pendingBuyOrders;
    ret << "===================" << std::endl << "Pending Buy Orders:" << std::endl;
    if (pendingBuyOrders.empty())
        ret << "None" << std::endl;
    while(!copy4PQ.empty()){
        ret << copy4PQ.top()->printOrder() << std::endl;
        copy4PQ.pop();
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
        if (sellOrders.empty()){ 
            ret -> setStatus("PARTIALLY FILLED"); 
            ret -> setMessage("Not enough sell orders available to fully complete this transaction."); 
            return ret;
        }

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

Summary* Engine::matchMarketSell(Order* order){
    int remVolume = order -> getOrderVolume();
    Summary* ret = new Summary(order);
    if (buyOrders.empty()){ 
        ret -> setStatus("CANCELED"); 
        ret -> setMessage("No buy orders available."); 
        return ret;
    }
    while(remVolume > 0){
        if (buyOrders.empty()){ 
            ret -> setStatus("PARTIALLY FILLED"); 
            ret -> setMessage("Not enough buys orders available to fully complete this transaction."); 
            return ret;
        }

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
    pendingBuyOrders.push(order);
    ret -> setStatus("PENDING");
    ret -> setMessage("Order has been added to pending queue.");
    return ret;
}

Summary* Engine::matchStopSell(Order* order){
    Summary* ret = new Summary(order);
    pendingSellOrders.push(order);
    ret -> setStatus("PENDING");
    ret -> setMessage("Order has been added to pending queue.");
    return ret;
}

Summary* Engine::matchStopLimitBuy(Order* order){
    Summary* ret = new Summary(order);
    pendingBuyOrders.push(order);
    ret -> setStatus("PENDING");
    ret -> setMessage("Order has been added to pending queue.");
    return ret;
}

Summary* Engine::matchStopLimitSell(Order* order){
    Summary* ret = new Summary(order);
    pendingSellOrders.push(order);
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

Summary* Engine::match(Order* o){
    Summary* ret = matchingFunctions[o -> orderTypeInt](o);
    if (ret -> getBestPrice() > 0)
        lastSale = ret -> getBestPrice();
    updateBook(o, lastSale, ret);
    return ret;
}
Summary* Engine::updateStopBuy(Order* order){
    Order* marketBuy = new Order(OrderType::MARKET_BUY, order -> getOrderVolume(), 0, order -> getOrderVisibility());
    return matchMarketBuy(marketBuy);
}

Summary* Engine::updateStopSell(Order* order){
    Order* marketSell = new Order(OrderType::MARKET_SELL, order -> getOrderVolume(), 0, order -> getOrderVisibility());
    return matchMarketSell(marketSell);
}

Summary* Engine::updateStopLimitBuy(Order* order){
    Order* limitBuy = new Order(OrderType::LIMIT_BUY, order -> getOrderVolume(), order -> getPrice(), order -> getOrderVisibility());
    return matchMarketBuy(limitBuy);
}

Summary* Engine::updateStopLimitSell(Order* order){
    Order* limitSell = new Order(OrderType::LIMIT_SELL, order -> getOrderVolume(), order -> getPrice(), order -> getOrderVisibility());
    return matchMarketSell(limitSell);
}

void Engine::updateBook(Order* order, float lastSale, Summary* summary){
    while(!pendingBuyOrders.empty() || !pendingSellOrders.empty()){
        if(!pendingBuyOrders.empty() && !pendingSellOrders.empty()){
            Order* topBuy = pendingBuyOrders.top();
            Order* topSell = pendingSellOrders.top();
            bool activateBuy = topBuy -> getStopPrice() >= lastSale;
            bool activateSell = topSell -> getStopPrice() <= lastSale;
            if (activateBuy && activateSell){
                Order* first = topBuy->getTimestamp() < topSell -> getTimestamp() ? topBuy : topSell;
                Order* second = first == topBuy ? topSell : topBuy;
                Summary* updateSummary1 = updateFunctions[first -> orderTypeInt](first);
                summary -> addTriggered(updateSummary1);

                Summary* updateSummary2 = updateFunctions[second -> orderTypeInt](second);
                summary -> addTriggered(updateSummary2);

                pendingSellOrders.pop();
                pendingBuyOrders.pop();
            }else if (activateBuy && !activateSell){
                Summary* updateSummary = updateFunctions[topBuy -> orderTypeInt](topBuy);
                pendingBuyOrders.pop();
                summary -> addTriggered(updateSummary);
            }else if (activateSell && !activateBuy){
                Summary* updateSummary = updateFunctions[topSell -> orderTypeInt](topSell);
                pendingSellOrders.pop();
                summary -> addTriggered(updateSummary);
            }else{
                break;
            }
        }else if (!pendingBuyOrders.empty()){
            Order* topBuy = pendingBuyOrders.top();
            if (topBuy -> getStopPrice() >= lastSale){  
                Summary* updateSummary = updateFunctions[topBuy -> orderTypeInt](topBuy);
                pendingBuyOrders.pop();
                summary -> addTriggered(updateSummary);
            }else
                break;
        }else{
            Order* topSell = pendingSellOrders.top();
            if (topSell -> getStopPrice() <= lastSale){  
                Summary* updateSummary = updateFunctions[topSell -> orderTypeInt](topSell);
                pendingSellOrders.pop();
                summary -> addTriggered(updateSummary);
            }else
                break;   
        }
    }
}

