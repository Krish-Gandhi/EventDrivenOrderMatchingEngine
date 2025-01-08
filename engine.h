#pragma once
#include <string>
#include <queue>
#include <vector>
#include <functional>
#include <ctime>

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
        Order(OrderType orderType, int orderVolume, float pricePerShare = 0, string orderVisiblity = "public", float stopPrice = 0);
        Order(const Order& other); // Copy constructor
        static std::string enumToString(OrderType o);
        std::string getOrderType();
        float getPrice();
        int getOrderVolume();
        std::string getOrderVisibility();
        std::string printOrder() const;
        std::string getTimestamp();
        OrderType orderType; // "market" "limit"
        int orderTypeInt;
        void removeVolume(int volume);
        float getStopPrice();
    private:
        int orderVolume;
        float stopPrice;
        float pricePerShare;
        string orderVisibility;
        std::string orderID;
        std::string timestampString;        
};


class Summary{
    public:
        Summary(Order* o);
        ~Summary();
        void add(Order* o);
        void addTriggered(Summary* s);
        std::string printSummary();
        void setStatus(std::string s);
        void setMessage(std::string s);
        float getBestPrice();
    private:
        float mostFavorablePrice;
        float averagePrice;
        int volume;
        std::string status;
        std::string message;
        Order* incomingOrder;
        std::vector<Order*> matchedOrders;
        std::vector<Summary*> triggeredOrders;
};

class Engine{
    public:
        Engine();
        Summary* match(Order* o);
        ~Engine();
        std::string printBook();
    private:
        Summary* matchMarketBuy(Order* order);
        Summary* matchMarketSell(Order* order);
        Summary* matchLimitBuy(Order* order);
        Summary* matchLimitSell(Order* order);
        Summary* matchStopBuy(Order* order);
        Summary* matchStopSell(Order* order);
        Summary* matchStopLimitBuy(Order* order);
        Summary* matchStopLimitSell(Order* order);
        Summary* matchFOKBuy(Order* order);
        Summary* matchFOKSell(Order* order);
        Summary* matchTrailingStopBuy(Order* order);
        Summary* matchTrailingStopSell(Order* order);
        std::vector<std::function<Summary*(Order* order)>> matchingFunctions;
        Summary* updateStopBuy(Order* order);
        Summary* updateStopSell(Order* order);
        Summary* updateStopLimitBuy(Order* order);
        Summary* updateStopLimitSell(Order* order);
        Summary* updateTrailingStopBuy(Order* order, float lastSale);
        Summary* updateTrailingStopSell(Order* order, float lastSale);
        std::vector<std::function<Summary*(Order* order, float lastSale)>> updateFunctions;
        float lastSale;
        struct cmpBuying{
            bool operator()(Order* l, Order* r) const { 
                if (l->getPrice() != r->getPrice()){
                    return l->getPrice() < r->getPrice();
                }else if (l->getOrderVisibility() != r->getOrderVisibility()){
                    if (l -> getOrderVisibility() == "hidden")
                        return true;
                    return false;
                }else{
                    return l->getTimestamp() > r->getTimestamp();
                }
                return false;
            }
        };

        struct cmpSelling{
            bool operator()(Order* l, Order* r) const { 
                if (l->getPrice() != r->getPrice()){
                    return l->getPrice() > r->getPrice();
                }else if (l->getOrderVisibility() != r->getOrderVisibility()){
                    if (l -> getOrderVisibility() == "hidden")
                        return true;
                    return false;
                }else{
                    return l->getTimestamp() > r->getTimestamp();
                }
                return false;
            }
        };
        void updateBook(Order* order, float lastSale, Summary* summary);
        std::priority_queue<Order*, std::vector<Order*>, cmpSelling> sellOrders;
        std::priority_queue<Order*, std::vector<Order*>, cmpBuying> buyOrders;

        std::priority_queue<Order*, std::vector<Order*>, cmpBuying> pendingSellOrders;
        std::priority_queue<Order*, std::vector<Order*>, cmpSelling> pendingBuyOrders;
};

