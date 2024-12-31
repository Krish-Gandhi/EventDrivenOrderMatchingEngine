#pragma once
#include <string>
#include <queue>
#include <vector>
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
        Order(OrderType orderType, int orderVolume, float pricePerShare = 0, string orderVisiblity = "public");
        static std::string enumToString(OrderType o);
        std::string getOrderType();
        float getPrice();
        std::string getOrderVisibility();
        std::string printOrder() const;
        std::string getTimestamp();
    private:
        OrderType orderType; // "market" "limit" 
        int orderVolume;
        float pricePerShare;
        string orderVisibility;
        int orderID;
        time_t timestamp;
        std::string timestampString;        
};


class Engine{
    public:
        Engine(){};
        Order* match(Order* o);
        ~Engine();
        std::string printBook();
    private:
        struct cmpBuying{
            bool operator()(Order* l, Order* r) const { 
                if (l->getPrice() != r->getPrice()){
                    return l->getPrice() < r->getPrice();
                }else if (l->getOrderVisibility() != r->getOrderVisibility()){
                    if (l -> getOrderVisibility() == "hidden")
                        return true;
                    return false;
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
                }
                return false;
            }
        };

        std::priority_queue<Order*, std::vector<Order*>, cmpSelling> sellOrders;
        std::priority_queue<Order*, std::vector<Order*>, cmpBuying> buyOrders;
};

