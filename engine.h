#pragma once
#include <string>

using namespace std;

enum class OrderType {
    MARKET_BUY,
    MARKET_SELL,
    LIMIT_BUY,
    LIMIT_SELL,
    STOP_BUY,
    STOP_SELL,
    STOP_LIMIT_BUY,
    STOP_LIMIT_SELL,
    FOK_BUY,
    FOK_SELL,
    TRAILING_STOP_BUY,
    TRAILING_STOP_SELL
};

class Order{
    public:
        Order(OrderType orderType, int orderVolume, float pricePerShare = 0, string orderVisiblity = "public");
        static std::string enumToString(OrderType o);
        std::string getOrderType();
    private:
        OrderType orderType; // "market" "limit" 
        int orderVolume;
        float pricePerShare;
        string orderVisibility;
        int orderID;
        int timestamp;        
};


class engine{
    public:
        engine();
        void match(Order o);
        ~engine();
    private:

};

